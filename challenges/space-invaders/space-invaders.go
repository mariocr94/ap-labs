package main

import (
	"flag"
	"fmt"
	"math/rand"
	"sync/atomic"
	"time"

	"github.com/faiface/pixel"
	"github.com/faiface/pixel/imdraw"
	"github.com/faiface/pixel/pixelgl"
	"github.com/faiface/pixel/text"
	"golang.org/x/image/colornames"
	"golang.org/x/image/font/basicfont"
)

type ship struct {
	sType  string
	name   string
	x      float64
	y      float64
	alive  bool
	vx     float64
	vy     float64
	in     chan msg
}

type controlTower struct {
	shipList        map[string]ship
	shipChan	    chan msg
	displayChan     chan msg
	commandChan     map[string](*chan msg)
}

type msg struct {
	cmd string
	val string
	s   ship
}

var gos uint64

var window *pixelgl.Window

var ct = controlTower{shipList: make(map[string]ship),
					 commandChan: make(map[string](*chan msg)),
					 displayChan: make(chan msg, 1000000),
					 shipChan	 : make(chan msg, 1000000)}   

var channelSpeed int
var shipSpeed int
var numEnemies int
var score int
var live int
var enemyKC int
var finish bool

func start() {
	window, _ = pixelgl.NewWindow(pixelgl.WindowConfig{Title: "Space Invaders", 
													   Bounds: pixel.R(0, 0, float64(512), float64(512)), VSync: true})
	window.SetPos(window.GetPos().Add(pixel.V(0, 1)))
	//initialize control Tower
	atomicGo(ct.driver)
	<-make(chan bool)
}

func atomicGo(f func()) {
	atomic.AddUint64(&gos, 1)
	go f()
}

func (m *controlTower) driver() {
	create()
	atomicGo(ct.actions)
	var a = 0
	for range time.NewTicker(time.Duration(shipSpeed) * time.Millisecond).C {
		a = a + 1
		if a%2 == 0 {
			ct.displayChan <- msg{cmd: "Display"}
		}
		if a%2 == 0 {
			ct.displayChan <- msg{cmd: "CheckCollisions"}
		}
		if a%2 == 0 {
			ct.displayChan <- msg{cmd: "KeyPressed"}
		}
	}
}

func create() {
	var spacingPx = 50
	var con = 0
	for r := 0; r < 300/spacingPx; r++ {
		for i := 0; i < 400/spacingPx; i++ {
			if con < numEnemies {
				ct.displayChan <- msg{cmd: "Add",
									   val: "Enemy", 
									   s: ship{x: float64(100 + spacingPx*i),
													 y: float64(400 - spacingPx*r),
													 vx: float64(-100), 
													 vy: 0}}
			}
			con++
		}
	}
	for f := 0; f < 4; f++ {
		for c := 0; c < 4; c++ {
			for r := 0; r < 4; r++ {
				if(!(r==0&&(c==1||c==2))){
					ct.displayChan <- msg{cmd: "Add", val: "Wall", 
										s: ship{x: float64(100 + f*100 + c*10), 
												y: float64(60 + r*10), 
												vx: 0, vy: 0}}
				}
			}
		}
	}
	ct.displayChan <- msg{cmd: "Add", val: "Ship", 
						  s: ship{x: 100, y: 10, vx: 0, vy: 0}}
}

func (m *controlTower) actions() {
	for {
		select {   
		case message := <-m.shipChan:
			if message.cmd == "Update" {
				m.shipList[message.s.name] = ship{name: message.s.name,
													sType: message.s.sType,
													x: message.s.x, 
													y: message.s.y, 
													alive: message.s.alive}
			} else if message.cmd == "Remove" {

				delete(m.shipList, message.s.name)
				if(message.s.sType == "Enemy"){
					enemyKC++
					if enemyKC > numEnemies{
						finish = true
					}
				}
			}
			// else if message.cmd == "Change" {
			// 	for _, s1 := range m.shipList {
			// 		if s1.sType == "Enemy"{
			// 			m.shipList[s1.name] = ship{name: s1.name,
			// 										sType: s1.sType,
			// 										x: s1.x, 
			// 										y: s1.y-10, 
			// 										alive: s1.alive,
			// 										vx: -s1.vx}
			// 		}
			// 	}
			// }
		case message := <-m.displayChan:
			if message.cmd == "Add" {
				name := message.val
				if name != "Ship" {
					// add timestamp to differentiate from others
					name = fmt.Sprintf(message.val, time.Now())
				}
				p := ship{name: name, 
						  sType: message.val, 
						  x: message.s.x, 
						  y: message.s.y, 
						  alive: true, 
						  vx: message.s.vx, 
						  vy: message.s.vy, 
						  in: make(chan msg)}
				m.commandChan[p.name] = &p.in
				atomicGo(p.driver)
			}
			if message.cmd == "CheckCollisions" {
				for _, s1 := range m.shipList {
					if s1.alive && s1.sType == "Laser" {
						for _, s2 := range m.shipList {
							if s2.alive && (s2.sType == "Enemy" || s2.sType == "Wall") {
								if (s2.x-s1.x)*(s2.x-s1.x)+(s2.y-s1.y)*(s2.y-s1.y) < 40 {
									*m.commandChan[s2.name] <- msg{cmd: "Kill"}
									score += 10
									*m.commandChan[s1.name] <- msg{cmd: "Kill"}
								}
							}
						}
					}
					if s1.alive && s1.sType == "Bomb" {
						for _, s2 := range m.shipList {
							if s2.alive && s2.sType == "Ship" {
								if (s2.x-s1.x)*(s2.x-s1.x)+(s2.y-s1.y)*(s2.y-s1.y) < 40 {
									live--
									if live == 0 {
										*m.commandChan[s2.name] <- msg{cmd: "Kill"}
									}
									*m.commandChan[s1.name] <- msg{cmd: "Kill"}
								}
							}
							if s2.alive && s2.sType == "Wall" {
								if (s2.x-s1.x)*(s2.x-s1.x)+(s2.y-s1.y)*(s2.y-s1.y) < 40 {
									*m.commandChan[s2.name] <- msg{cmd: "Kill"}
									*m.commandChan[s1.name] <- msg{cmd: "Kill"}
								}
							}
						}
					}
					if s1.alive && s1.sType == "Enemy" {
						for _, s2 := range m.shipList {
							if s2.alive && s2.sType == "Ship" {
								if (s2.x-s1.x)*(s2.x-s1.x)+(s2.y-s1.y)*(s2.y-s1.y) < 40 {
									live--
									if live == 0 {
										*m.commandChan[s2.name] <- msg{cmd: "Kill"}
									}
									*m.commandChan[s1.name] <- msg{cmd: "Kill"}
								}
							}
							if s2.alive && s2.sType == "Wall" {
								if (s2.x-s1.x)*(s2.x-s1.x)+(s2.y-s1.y)*(s2.y-s1.y) < 40 {
									*m.commandChan[s2.name] <- msg{cmd: "Kill"}
									*m.commandChan[s1.name] <- msg{cmd: "Kill"}
								}
							}
						}
					}
				}
			}
			if message.cmd == "KeyPressed" {
				if window != nil {
					if m.commandChan["Ship"] != nil {
						if window.Pressed(pixelgl.KeyLeft) {
							*m.commandChan["Ship"] <- msg{cmd: "MoveLeft"}
						} else if window.Pressed(pixelgl.KeyRight) {
							*m.commandChan["Ship"] <- msg{cmd: "MoveRight"}
						} else if window.Pressed(pixelgl.KeySpace) {
							*m.commandChan["Ship"] <- msg{cmd: "Shoot"}
						} else {
							*m.commandChan["Ship"] <- msg{cmd: "Stop"}
						}
					}
				}
			}
			if message.cmd == "Display" {
				if window != nil {
					var imd = imdraw.New(nil)
					imd.Clear()
					for _, s := range m.shipList {
						if s.alive && s.sType == "Ship" {
							imd.Color = colornames.Purple
							imd.Push(pixel.V(float64(s.x), float64(s.y)))
							imd.Circle(4, 2)
						} else if s.alive && s.sType == "Enemy" {
							imd.Color = colornames.White
							imd.Push(pixel.V(float64(s.x), float64(s.y)))
							imd.Circle(4, 2)
						} else if s.alive && s.sType == "Wall" {
							imd.Color = colornames.Green
							imd.Push(pixel.V(float64(s.x), float64(s.y)))
							imd.Circle(5, 4)
						} else if s.alive && s.sType == "Laser" {
							imd.Color = colornames.Orange
							imd.Push(pixel.V(float64(s.x), float64(s.y)))
							imd.Circle(2, 2)
						} else if s.alive && s.sType == "Bomb" {
							imd.Color = colornames.Red
							imd.Push(pixel.V(float64(s.x), float64(s.y)))
							imd.Circle(3, 2)
						}
					}
					window.Clear(colornames.Black)
					imd.Draw(window)
					basicAtlas := text.NewAtlas(basicfont.Face7x13, text.ASCII)
					basicTxt := text.New(pixel.V(100, 500), basicAtlas)
					fmt.Fprintln(basicTxt, "Score: ", score)
					fmt.Fprintln(basicTxt, "Lives: ", live)
					if finish {
						fmt.Fprintln(basicTxt, "You won!")
					}
					basicTxt.Draw(window, pixel.IM)
					window.Update()
					if finish {
						<-make(chan bool)
					}
				}
			}
		}
	}
}

func (p *ship) driver() {
	atomicGo(p.actions)
	for range time.NewTicker(time.Duration(channelSpeed) * time.Millisecond).C {
		p.in <- msg{cmd: "Move"}
	}
}

func (p *ship) actions() {
	for {
		m := <-p.in

		// Player commands
		if m.cmd == "Kill" {
			p.alive = false   
			ct.shipChan	  <- msg{cmd: "Remove", s: *p}
			if p.sType == "Ship" {
				fmt.Println("Game Over")
				<-make(chan bool)
			}
		} else if m.cmd == "MoveLeft" {
			if p.x > 20 {
				p.vx = -80
			}
		} else if m.cmd == "Stop" {
			p.vx = 0
		} else if m.cmd == "MoveRight" {
			if p.x < 480 {
				p.vx = 80
			}
		} else if m.cmd == "Shoot" {
			ct.displayChan <- msg{cmd: "Add", val: "Laser", 
								  s: ship{x: p.x, y: p.y, vx: 0, vy: 100}}
		} 

		//Enemy commands
		if m.cmd == "Move" {
			var xPixels = p.vx / 1000 * float64(shipSpeed)
			var yPixels = p.vy / 1000 * float64(shipSpeed)
			p.x = p.x + xPixels
			if p.alive && p.sType == "Enemy" {
				if rand.Intn(950) < 1 {
					ct.displayChan <- msg{cmd: "Add", val: "Bomb", 
										  s: ship{x: p.x, y: p.y, vx: 0, vy: -100}}
				}
				if p.x > 500 || p.x < 10 {
					//ct.shipChan <- msg{cmd: "Change"}
					p.vx = -p.vx
					p.y = p.y - 10
				}
			} else {
				p.y = p.y + yPixels
			}

			if p.sType == "Laser" && p.y > 600 {
				p.alive = false   
				ct.shipChan	  <- msg{cmd: "Remove", s: *p}
			} else if p.sType == "Bomb" && p.y < 0 {
				p.alive = false   
				ct.shipChan	  <- msg{cmd: "Remove", s: *p}
			}

    		if p.alive {   
				ct.shipChan	  <- msg{cmd: "Update", s: *p}
			}
		}
	}
}

func main() {
	channelSpeed = 10
	shipSpeed = 20
	live = 10
	enemyKC = 0
	finish = false
	flag.IntVar(&numEnemies, "Enemies", 50, "Number of Enemies")

	flag.Parse()
	pixelgl.Run(start)
}