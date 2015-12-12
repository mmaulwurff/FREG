#include "RandomManager.h"

#include <QtGlobal>
#include <QDebug>

RandomManager* RandomManager::randomManager = nullptr;

RandomManager::RandomManager(const int seed)
    : randomEngine(seed)
{
    Q_ASSERT(randomManager == nullptr);
    randomManager = this;
}

int RandomManager::rand() { return randomManager->randomEngine(); }

