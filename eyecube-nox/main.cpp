#include <QtCore/QCoreApplication>
#include <ncurses.h>

void contact_selver() {}

void get_blocks() { //this will be rewritten to use network and get blocks from server
    short i;
}

class player {
    long x, y; //position
    unsigned health : 7;
};

class world { //surrounding world without physics
    unsigned short shred_width,
                   height,
                   time;
    unsigned short blocks[shred_width][shred_width][height];
    void get_blocks();
    world() {
        contact_server();
        get_blocks();
    }
public:

};

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    player mole;
    world earth;
    return a.exec();
}
