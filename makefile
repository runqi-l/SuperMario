NAME = "mario"

# Add $(MAC_OPT) to the compile line for macOS 
# (should be ignored by Linux, set to nothing if causing problems)
MAC_OPT = -I/opt/X11/include

all:
	@echo "Compiling $(NAME)"
	g++ -std=c++0x -o $(NAME) $(NAME).cpp -L/opt/X11/lib -lX11 -lstdc++ $(MAC_OPT)

run: all
	@echo "Running $(NAME)"

clean:
	-rm *o
	-rm $(NAME)