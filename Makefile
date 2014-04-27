CXX = clang++
EXECNAME = vxr
OBJS = objs/main.o

$(EXECNAME): $(OBJS)
	$(CXX) -o $@ $^ -lSDL2

objs/%.o: %.cpp
	$(CXX) -c -o $@ $< -Wall -g -std=c++0x

clean:
	-rm -f $(OBJS) $(EXECNAME)

