#g++ -Wall -lncurses -pthread main.cpp world.cpp blocks.cpp screen.cpp -o FREG.bin;
clang++ -Wall -pedantic -lncurses -pthread src/*.cpp -o FREG.bin;
