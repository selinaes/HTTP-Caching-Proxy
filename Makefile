clean:
	rm -rf *.o main test
test:
	g++ -g -o test test_malformed.cpp
main:
	g++ -g -o main main.cpp Proxy.h Proxy.cpp Logging.h Response.h Logging.cpp struct_helper.hpp parse.hpp Response.cpp -lpthread