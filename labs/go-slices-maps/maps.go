package main

import (
	"golang.org/x/tour/wc"
	"strings"
)

func WordCount(s string) map[string]int {
	mapa := make(map[string]int)
	for _, e := range(strings.Fields(s)){
		mapa[string(e)]++
	}
	return mapa
}

func main() {
	wc.Test(WordCount)
}
