// Copyright © 2016 Alan A. A. Donovan & Brian W. Kernighan.
// License: https://creativecommons.org/licenses/by-nc-sa/4.0/

// See page 156.

// Package geometry defines simple types for plane geometry.
//!+point
package main

import (
	"strconv"
	"fmt"
	"os"
	"math"
	"math/rand"
	"time"
	"sort"
)
type Point struct{ x, y float64 }

func (p Point) X() float64{
	return p.x
}
func (p Point) Y() float64{
	return p.y
}

// traditional function
func Distance(p, q Point) float64 {
	return math.Hypot(q.X()-p.X(), q.Y()-p.Y())
}

// same thing, but as a method of the Point type
func (p Point) Distance(q Point) float64 {
	return math.Hypot(q.X()-p.X(), q.Y()-p.Y())
}

//!-point

//!+path

// A Path is a journey connecting the points with straight lines.
type Path []Point

// Distance returns the distance traveled along the path.
func (path Path) Distance() float64 {
	sum := 0.0
	for i := range path {
		if i > 0 {
			sum += path[i-1].Distance(path[i])
		}
	}
	return sum
}

func generatePoints(sides int) [] Point{
	points := make([]Point, sides)
	s1 := rand.NewSource(time.Now().UnixNano())
    r1 := rand.New(s1)
	for i := 0; i < sides; i++{
		x := math.Round(r1.Float64()*200) - 100
		y := math.Round(r1.Float64()*200) - 100
		points[i] = Point{x, y} 
	}

	return points

}

func createPath(points []Point) Path{
	left := getLeftMost(points)
	right := getRightMost(points)
	upper, lower := getUL(points, left, right)
	path := make(Path, len(points))
	i := 1
	path[0] = points[left]
	for _, p := range(upper){
		path[i] = p
		i++
	}
	path[i] = points[right]
	i++
	for _, p := range(lower){
		path[i] = p
		i++
	}

	return path
}

func getLeftMost(points []Point) int{
	left := 101.0
	index := -1
	for i, p := range(points){
		if (p.X() < left){
			left = p.X()
			index = i
		}
	}
	return index
}

func getRightMost(points []Point) int{
	right := -101.0
	index := -1
	for i, p := range(points){
		if (p.X() > right){
			right = p.X()
			index = i
		}
	}
	return index
}

func getUL(points []Point, left, right int) ([]Point, []Point){
	var u, l []Point
	i := 0
	j := 0
	leftPoint := points[left]
	rightPoint := points[right]
	for k, p := range(points){
		if(k!=left && k!=right){
			if(aboveOrBelow(p, leftPoint, rightPoint) > 0){
				u = append(u, p)
				i++
			}else{
				l = append(l, p)
				j++
			}
		}
	}
	sort.Sort(ByXAsc(u))
	sort.Sort(ByXDes(l))

	return u, l
}

type ByXAsc []Point
func (a ByXAsc) Len() int           { return len(a) }
func (a ByXAsc) Less(i, j int) bool { return a[i].X() < a[j].X() }
func (a ByXAsc) Swap(i, j int)      { a[i], a[j] = a[j], a[i] }
type ByXDes []Point
func (a ByXDes) Len() int           { return len(a) }
func (a ByXDes) Less(i, j int) bool { return a[i].X() > a[j].X() }
func (a ByXDes) Swap(i, j int)      { a[i], a[j] = a[j], a[i] }

func aboveOrBelow(p , l, r Point) float64{
	v1x := r.X() - l.X()
	v1y := r.Y() - l.Y()
	v2x := p.X() - l.X()
	v2y := p.Y() - l.Y()
	cross := v1x * v2y - v1y * v2x
	return cross
}

func printAll(p Path){
	fmt.Printf("- Generating a [%d] sides figure.\n", len(p))
	fmt.Println("- Figure's vertices:")
	for _, point := range(p){
		fmt.Printf("  - ( %d , %d )", int(point.X()), int(point.Y()))
		fmt.Println()
	}
	fmt.Println("- Figure's Perimeter:")
	printPerimeters(p)
}

func printPerimeters (p Path){
	fmt.Printf("  - ")
	s := 0.0
	i := 0
	for i = 0; i < len(p)-1 ; i++{
		d := p[i].Distance(p[i+1])
		s += d
		fmt.Printf("%.2f + ",d)
	}
	d:= p[i].Distance(p[0])
	s+= d
	fmt.Printf("%.2f = %.2f",d,s )
	fmt.Println()
}

func main() {
	sides, _ := strconv.Atoi(os.Args[1])
	if sides < 3 {
		fmt.Printf("Please make sure that the figure has at least 3 sides.\n")
		return 
	}
	
	points := generatePoints(sides)
	camino := createPath(points)
	printAll(camino)

}

//!-path
