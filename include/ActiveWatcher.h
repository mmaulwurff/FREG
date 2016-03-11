#ifndef ACTIVEWATCHER_H
#define ACTIVEWATCHER_H

#include <QObject>
#include "blocks/Active.h"

class QString;

class ActiveWatcher : public QObject {
    Q_OBJECT
public:

    ActiveWatcher(QObject* const parent)
        : QObject(parent)
    {}
    ~ActiveWatcher() { watched->SetWatcher(nullptr); }

    void SetWatched(Active* const a) { watched = a; }

signals:

    void Moved(int direction) const;
    void ReceivedText(const QString& text) const;

    void Updated() const;
    void CauseTeleportation() const;

    void Destroyed() const;

private:

    Active* watched;
};

#endif // ACTIVEWATCHER_H
