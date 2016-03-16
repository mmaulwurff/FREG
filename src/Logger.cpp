#include "Logger.h"

Logger::Logger(const QString& name)
    : Singleton<Logger>(this)
    , filename(name)
{
    start();
}

Logger::~Logger()
{
    quit();
    wait();
}

void Logger::run()
{
    QFile file(filename);
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        connect(this, &Logger::Log, &file, [&file](const QString& message) {
            const QByteArray utf8 = message.toUtf8();
            file.write(utf8);
            file.write("\n");
        });
        emit ready();
        exec();
    }
    disconnect(this, &Logger::Log, nullptr, nullptr);
}
