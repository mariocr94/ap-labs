package main

import (
	//"io"
	"log"
	"net"
	"os"
	"fmt"
	"strings"
)

func connect(city, host string){
	conn, err := net.Dial("tcp", host)
    if err != nil {
        log.Fatal(err)
	}
	// fmt.Printf("%s\t:\t",city)
	// io.Copy(os.Stdout, conn) // NOTE: ignoring errors

	for {
		hour := make([]byte,11)
		_, err := conn.Read(hour)
		if err != nil{
			conn.Close()
			log.Print(err)
		}
		fmt.Printf("%s\t: %s",city, hour)
		
	  }
	
}

func main() {

	if len(os.Args) < 2 {
		fmt.Println("Please use the correct format and at least one time zone with its port")
		fmt.Println("go run clockWall.go TZ1=localhost:<port>")
		os.Exit(1)
	}

    for i := 1; i<len(os.Args); i++{
		puerto := strings.Split(os.Args[i],"=")
		go connect(puerto[0], puerto[1])
	}

	for{

	}
}