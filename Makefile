clean:
	rm -rf *.o main
main:
	g++ -g -o main main.cpp Proxy.cpp Logging.cpp struct_helper.hpp parse.hpp Response.cpp -lpthread