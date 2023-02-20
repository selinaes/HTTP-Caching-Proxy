clean:
	rm -rf *.o main
main:
	g++ -g -o main main.cpp Proxy.cpp -lpthread