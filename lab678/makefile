all: control counting

control:
	g++ control.cpp -lzmq -o control -Wall -pedantic

counting:
	g++ counting.cpp -lzmq -o counting -Wall -pedantic

clean:
	rm -rf control counting