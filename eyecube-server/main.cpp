#include <QtCore/QCoreApplication>

class block {
    unsigned id : 10;
    unsigned owner : 15;
    unsigned allow_other : 1;
public:
};

class world {
    unsigned long width; //size of the world
    unsigned short shred_width,
                   height,
                   time;
public:
};

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    return a.exec();
}
