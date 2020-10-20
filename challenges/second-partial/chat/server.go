// Copyright © 2016 Alan A. A. Donovan & Brian W. Kernighan.
// License: https://creativecommons.org/licenses/by-nc-sa/4.0/

// See page 254.
//!+

// Chat is a server that lets clients chat with each other.
package main

import (
	"bufio"
	"time"
	"fmt"
	"log"
	"os"
	"strings"
	"net"
)

//!+broadcaster
type client chan<- string // an outgoing message channel

var (
	entering = make(chan client)
	leaving  = make(chan client)
	messages = make(chan string) // all incoming client messages
)

type clientStruct struct{
	username string
	address string
	isAdmin bool
	kicked bool
}

var clientes = make(map[string]client)
var infoClientes = make(map[string]clientStruct)
var clientesKickeados = make([]string, 0)

func broadcaster() {
	clients := make(map[client]bool) // all connected clients
	for {
		select {
		case msg := <-messages:
			// Broadcast incoming message to all
			// clients' outgoing message channels.
			for cli := range clients {
				cli <- msg
			}

		case cli := <-entering:
			clients[cli] = true

		case cli := <-leaving:
			delete(clients, cli)
			close(cli)
		}
	}
}

//!-broadcaster

//!+handleConn
func handleConn(conn net.Conn) {
	ch := make(chan string) // outgoing client messages
	go clientWriter(conn, ch)

	input := bufio.NewScanner(conn)
	input.Scan()
	user := input.Text()

	var cli clientStruct
	cli.username = user
	cli.address = conn.RemoteAddr().String()
	cli.kicked = false
	cli.isAdmin = false

	ch <- "irc-server > Welcome to the Simple IRC Server"
	ch <- "irc-server > Your user [" + user + "] is successfully logged"
	messages <- "irc-server > New connected user: " + user
	fmt.Println("irc-server > New connected user:", user)
	clientes[user] = ch
	infoClientes[user] = cli
	entering <- ch

	if len(clientes) == 1 {
		cli.isAdmin = true
		ch <- "irc-server > Congratulations! You were the first user."
		ch <- "irc-server > You have been promoted to ADMIN."
		fmt.Println("irc-server >", user,"has been promoted to ADMIN.", )
	}
	
	for input.Scan() {
		data := strings.Split(input.Text(), " ")
		if !isKicked(cli.username) {
			switch data[0]{
				case "/users":
						list := "irc-server > Connected users: "
						for cli := range clientes {
							if isKicked(cli) {
								list = list + "[" + cli + ", KICKED]" 
							}else{
								list = list + "[" + cli + "]" 
							}
							
						}
						ch <- list
					break

				case "/msg":
						if len(data) < 3 {
							ch <- "irc-server > Please use the correct notation:"
							ch <- "irc-server > /msg <user> <message>"
						}else{
							mUser := data[1]
							if _, exists := clientes[mUser]; exists{
								if !isKicked(mUser){
									msg := user + " > "
									for _, info := range data[2:]{
										msg = msg + info + " "
									}
									clientes[mUser] <- msg
								}else{
									ch <- "irc-server > the user [" + mUser + "] has been kicked and can't receive messages"
								}
							}else{
								ch <- "irc-server > The user [" + mUser + "] does not exist."
							}
						}
					break
				
				case "/time":
					ch <- "irc-server > Local time is: " + time.Now().Format("15:04")
					break
				
				case "/user":
					if len(data) != 2 {
						ch <- "irc-server > Please use the correct notation:"
						ch <- "irc-server > /user <user>"
					}else{
						mUser := data[1]
						if _, exists := clientes[mUser]; exists{
							ch <- "irc-server > Info on user [" + mUser + "]:"
							ch <- "irc-server > Username: " + infoClientes[mUser].username + ", IP Address: " + infoClientes[mUser].address
							if isKicked(mUser){
								ch <- "irc-server > This user has been kicked of the server."
							}
						}else{
							ch <- "irc-server > The user [" + mUser + "] does not exist."
						}
					}
					break

				case "/kick":
					if cli.isAdmin == true{
						if len(data) != 2{
							ch <- "irc-server > Please use the correct notation:"
							ch <- "irc-server > /kick <user>"
						}else{
							mUser := data[1]
							if _, exists := clientes[mUser]; exists{
								messages <- "irc-server > The user [" + mUser + "] has been kicked."
								fmt.Println("irc-server > The user [" + mUser + "] has been kicked.")
								kickUser(mUser)
								leaving <- clientes[mUser]
							}else{
								ch <- "irc-server > The user [" + mUser + "] does not exist."
							}
						}
					}else{
						ch <- "irc-server > You are not the ADMIN so you can't kick someone."
					}
					break

				default:
					messages <- user + " > " + input.Text()
			}
		}
		
	}
	// NOTE: ignoring potential errors from input.Err()

	leaving <- ch
	delete(clientes, user)
	messages <- "irc-server > " + user + " left the server."
	conn.Close()
}

func kickUser(user string){
	
	clientes[user] <- "irc-server > You have been kicked by the ADMIN."
	clientesKickeados = append(clientesKickeados, user)
}

func isKicked(user string) bool{
	for _, cliente := range clientesKickeados{
		if cliente == user{
			return true
		}
	}
	return false
}

func clientWriter(conn net.Conn, ch <-chan string) {
	for msg := range ch {
		fmt.Fprintln(conn, msg) // NOTE: ignoring network errors
	}
}

//!-handleConn

//!+main
func main() {

	if len(os.Args) != 5 {
		fmt.Println("Please use the correct format for the server.go")
		fmt.Println("go run server.go -host localhost -port <port>")
		return;
	}

	server := "localhost:" + os.Args[4]

	listener, err := net.Listen("tcp", server)
	if err != nil {
		log.Fatal(err)
	}

	fmt.Println("irc-server > Simple IRC Server started at %s", server)
	fmt.Println("irc-server > Ready for receiving new clients")

	go broadcaster()
	for {
		conn, err := listener.Accept()
		if err != nil {
			log.Print(err)
			continue
		}
		go handleConn(conn)
	}
}

//!-main
