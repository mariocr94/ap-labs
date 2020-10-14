// Clock2 is a concurrent TCP server that periodically writes the time.
package main

import (
	"io"
	"log"
	"net"
	"os"
	"fmt"
	"time"
)

func handleConn(c net.Conn, tz string) {
	defer c.Close()
	_, timeErr := time.LoadLocation(tz);
	if timeErr != nil {
		fmt.Printf("Could not load time zone: %s", tz)
		log.Print(timeErr)
		return;
	}
	for {
		_, err := io.WriteString(c, time.Now().Format("15:04:05\n"))
		if err != nil {
			return // e.g., client disconnected
		}
		time.Sleep(1 * time.Second)
	}
}

func main() {

	if len(os.Args) != 3{
		fmt.Println("Please use the correct format:")
		fmt.Println("TZ=TimeZone go run clock2.go -port <port>\n")
		os.Exit(1)
	}
	puerto := "localhost:" + os.Args[2]
	tz := os.Getenv("TZ")
	listener, err := net.Listen("tcp", puerto)
	if err != nil {
		log.Fatal(err)
	}

	for {
		conn, err := listener.Accept()
		if err != nil {
			log.Print(err) // e.g., connection aborted
			continue
		}
		go handleConn(conn, tz) // handle connections concurrently
	}
}
