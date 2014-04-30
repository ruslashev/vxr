CXX = clang++
EXECNAME = vxr
OBJS = objs/main.o objs/pixeldrawer.o

$(EXECNAME): $(OBJS)
	$(CXX) -o $@ $^ -lSDL2
	./$(EXECNAME)

objs/%.o: %.cpp
	$(CXX) -c -o $@ $< -Wall -g -std=c++0x

clean:
	-rm -f $(OBJS) $(EXECNAME)

