package main

import (
	"fmt"
	"time"
)
var token = make(chan string)

func main() {
	maxStages := 1000000;
	first := make(chan time.Time)
	fmt.Println("Maximum number of pipeline stages   : ", maxStages)
	go pipeline(first, 0, maxStages)
	first <- time.Now()
	<-token	
	fmt.Println("Maximum number of pipeline stages   : ", maxStages)
}

func pipeline(first chan time.Time, current int, max int) {
	if current < max {
		tiempo := <- first
		fmt.Println("Routine No.",current, " Time to transit trough the pipeline : ", time.Since(tiempo))
		go pipeline(first, current+1, max)
		first<- time.Now()
	}else{
		fmt.Println("done")
		token <- "done"
	}
}