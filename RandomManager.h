#ifndef RANDOMMANAGER_H
#define RANDOMMANAGER_H

#include <random>

class RandomManager {
public:

    RandomManager(int seed);

    static int rand();

private:

    std::ranlux24 randomEngine;

    static RandomManager* randomManager;

};

#endif // RANDOMMANAGER_H
