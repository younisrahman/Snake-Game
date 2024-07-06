all:
	g++ -std=c++11 snake_game.cpp -o snake_game -I/opt/homebrew/include/SDL2 -D_THREAD_SAFE -L/opt/homebrew/lib -lSDL2 -lSDL2_image -lSDL2_ttf


