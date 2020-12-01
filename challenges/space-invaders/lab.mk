
deps:
    go get github.com/faiface/pixel
    go get github.com/faiface/pixel/imdraw
    go get github.com/faiface/pixel/pixelgl
    go get golang.org/x/image/colornames

clean:
	rm -f space-invaders
	
build:
	go build space-invaders.go

run: build
	./space-invaders