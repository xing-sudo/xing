.PHONY:all
all:mykill signal
mykill:mykill.cc
	g++ -o $@ $^ -std=c++11
signal:signal.cc
	g++ -o $@ $^ -std=c++11

.PHONY:clean
clean:
	rm -f mykill signal