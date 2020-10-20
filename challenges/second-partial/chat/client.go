// Copyright © 2016 Alan A. A. Donovan & Brian W. Kernighan.
// License: https://creativecommons.org/licenses/by-nc-sa/4.0/

// See page 227.

// Netcat is a simple read/write client for TCP servers.
package main

import (
	"fmt"
	"io"
	"log"
	"net"
	"os"
)

//!+
func main() {

	if len(os.Args) != 5 || os.Args[1] != "-user" || os.Args[3] != "-server" {
		fmt.Println("Please use the correct format for the client program:")
		fmt.Println("go run client.fo -user <username> -server localhost:<port>")
		return;
	}

	user := os.Args[2]
	port := os.Args[4]

	conn, err := net.Dial("tcp", port)
	if err != nil {
		log.Fatal(err)
	}

	_, err = io.WriteString(conn, user)
	if err != nil {
		log.Fatal(err)
	}

	done := make(chan struct{})
	go func() {
		io.Copy(os.Stdout, conn) // NOTE: ignoring errors
		fmt.Println("Exited from server.")
		done <- struct{}{} // signal the main goroutine
	}()
	mustCopy(conn, os.Stdin)
	conn.Close()
	<-done // wait for background goroutine to finish
}

//!-

func mustCopy(dst io.Writer, src io.Reader) {
	if _, err := io.Copy(dst, src); err != nil {
		log.Fatal(err)
	}
}
