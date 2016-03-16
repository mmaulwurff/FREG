#ifndef LOGGER_H
#define LOGGER_H

#include "Singleton.h"
#include <QThread>
#include <QFile>

/** Simple logger class. Provides thread-safe log writing in parallel thread.
 *  After construction, Logger is not ready immediately. It emits ready() signal
 *  when it is ready. */
class Logger : public QThread, public Singleton<Logger> {
    Q_OBJECT

public:

    explicit Logger(const QString& filename);
    ~Logger();

signals:

    void Log(const QString& message) const;
    void ready();

protected:

    void run() override;

private:

    const QString filename;

};

#endif // LOGGER_H
