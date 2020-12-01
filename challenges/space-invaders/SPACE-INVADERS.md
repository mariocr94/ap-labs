# How to run
The machine needs to have the following dependencies:
```
pkg-config
libglvnd
mesa
libxi
xorg-dev
xorg-server-dev
```
In the case of my pc, since I run ssh from VSCode, I had to use the following command to be able to see the program running in the VM.
```
export DISPLAY:=0.0
```

Use the following commands as well:
```
go get github.com/faiface/pixel
go get golang.org/x/image/
```
Once that is installed, use the following command in the project folder.
```
go mod init github.com/faiface/pixel-examples/platformer 
```

In order to correctly run the program, you must execute startx to initialize the graphic OS then build the go file first and then execute the binary.

## The execution of the binary must be done in the VM, not in an SSH.