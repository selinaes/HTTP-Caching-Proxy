clean:
	rm -rf *.o main
main:
	g++ -g -o main main.cpp Proxy.cpp Logging.cpp struct_helper.h Response.cpp -lpthread