all:demo smtp

smtp:smtp.h smtp.cpp
	g++ -c -o smtp.a smtp.cpp

demo:demo.cpp
	g++ -o demo demo.cpp smtp.a
	
clean:
	rm -rf demo
