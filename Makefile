CXX = clang++
EXECNAME = vxr
OBJS = objs/main.o objs/pixeldrawer.o objs/textures.o objs/game.o

default: $(EXECNAME)
	./$(EXECNAME)

$(EXECNAME): $(OBJS)
	$(CXX) -o $@ $^ -lSDL2

objs/%.o: %.cpp
	$(CXX) -c -o $@ $< -Wall -std=c++0x -O3 -g -pthread

clean:
	-rm -f $(OBJS) $(EXECNAME)

