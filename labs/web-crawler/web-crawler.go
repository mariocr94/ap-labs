// Copyright © 2016 Alan A. A. Donovan & Brian W. Kernighan.
// License: https://creativecommons.org/licenses/by-nc-sa/4.0/

// See page 241.

// Crawl2 crawls web links starting with the command-line arguments.
//
// This version uses a buffered channel as a counting semaphore
// to limit the number of concurrent calls to links.Extract.
//
// Crawl3 adds support for depth limiting.
//
package main

import (
	"fmt"
	"bufio"
	"log"
	"flag"

	"os"
	"gopl.io/ch5/links"
)

//!+sema
// tokens is a counting semaphore used to
// enforce a limit of 20 concurrent requests.
var tokens = make(chan struct{}, 20)

func crawl(page webPage) []webPage {
	tokens <- struct{}{} // acquire a token
	list, err := links.Extract(page.link)
	<-tokens // release the token

	if err != nil {
		log.Print(err)
	}

	newList := make([]webPage, 0)
	for _, nextPage := range (list){
		wPage := webPage{link: nextPage, depth: page.depth+1}
		newList = append(newList, wPage)
	}
	return newList
}

//!-sema

type webPage struct{
	link string
	depth int
}

//!+
func main() {

	if len(os.Args) != 4 {
		fmt.Println("Please use the correct format:")
		fmt.Println("go run web-crawler.go -depth=<depth> -results=<file.txt> <link>")
		return
	}

	depth := flag.Int("depth", 1, "Depth")
	result := flag.String("results", "results.txt", "Output file")
	flag.Parse()

	f, err := os.Create(*result)
	if err != nil {
		panic(err)
		return
	}

	writer := bufio.NewWriter(f)
	defer writer.Flush()
	
	worklist := make(chan []webPage)
	var n int // number of pending sends to worklist

	// Start with the command-line arguments.
	n++
	go func() { 
		page := webPage{link: (os.Args[3]), depth: 0}
		links := make([]webPage, 0)
		links = append(links, page)
		worklist <- links 
		}()

	// Crawl the web concurrently.
	seen := make(map[string]bool)
	for ; n > 0; n-- {
		list := <-worklist
		for _, link := range list {
			if !seen[link.link] {
				seen[link.link] = true
				writer.WriteString(link.link + "\n")
				if link.depth == *depth{
					continue
				}
				n++
				go func(link webPage) {
					worklist <- crawl(link)
				}(link)
			}
		}
	}
	writer.Flush()
	err = f.Close()
	if err!= nil {
		panic(err)
	}
	
}

//!-
