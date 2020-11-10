package main

import (
	"fmt"
	"time"
)
var commsPerSecond int
func main() {
	
	ping := make(chan int)
	pong := make(chan int)
	startTime := time.Now()
	sum := 0
	go back(ping, pong)
	go forth(ping, pong)
	for{
		time.Sleep(1 * time.Second)
		fmt.Println("Communications Per Second : ", commsPerSecond)
		sum += commsPerSecond
		commsPerSecond = 0
		if time.Since(startTime)>(10 * time.Second) {
			sum /= 10
			fmt.Println("Average communications per second in 10 seconds: ", sum)
			close(ping)
			close(pong)
			return
		}
	}
}

func back(ping, pong chan int){
	for {
		ping<-1
		commsPerSecond += 1
		<-pong
	}
}

func forth(ping, pong chan int){
	for {
		<-ping
		commsPerSecond += 1
		pong<-1
	}
}