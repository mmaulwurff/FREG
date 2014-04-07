    /* freg, Free-Roaming Elementary Game with open and interactive world
    *  Copyright (C) 2012-2014 Alexander 'mmaulwurff' Kromm
    *  mmaulwurff@gmail.com
    *
    * This file is part of FREG.
    *
    * FREG is free software: you can redistribute it and/or modify
    * it under the terms of the GNU General Public License as published by
    * the Free Software Foundation, either version 3 of the License, or
    * (at your option) any later version.
    *
    * FREG is distributed in the hope that it will be useful,
    * but WITHOUT ANY WARRANTY; without even the implied warranty of
    * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    * GNU General Public License for more details.
    *
    * You should have received a copy of the GNU General Public License
    * along with FREG. If not, see <http://www.gnu.org/licenses/>. */

#include <QTranslator>
#include <QSettings>
#include <QDir>
#include <QTime>
#include <QCommandLineParser>
#include "screen.h" // NOX, if needed, is defined in screen.h
#include "world.h"
#include "Player.h"

#ifdef NOX // no need for X server
    #include <QCoreApplication>
    typedef QCoreApplication Application;
#else
    #include <QApplication>
    typedef QApplication Application;
#endif

int main(int argc, char ** argv) {
    setlocale(LC_CTYPE, "C-UTF-8");
    QDir::current().mkdir("texts");
    freopen("texts/errors.txt", "wt", stderr);
    qsrand(QTime::currentTime().msec());

    Application freg(argc, argv);
    QCoreApplication::setOrganizationName("freg-team");
    QCoreApplication::setApplicationName("freg");
    QCoreApplication::setApplicationVersion(VER);

    QTranslator translator;
    translator.load(QString("freg_") + locale);
    freg.installTranslator(&translator);

    // parse arguments
    QCommandLineParser parser;
    parser.setApplicationDescription(QObject::tr("freg - 3d open world game"));
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption ascii(QStringList() << "a" <<"ascii",
        QObject::tr("Use ASCII-characters only."));
    parser.addOption(ascii);
    parser.process(freg);

    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings sett(QDir::currentPath()+"/freg.ini", QSettings::IniFormat);
    const QString worldName = sett.value("current_world", "mu").toString();
    sett.setValue("current_world", worldName);

    World world(worldName);
    Player player;
    int error = NO_ERROR;
    const Screen screen(&world, &player, error, parser.isSet(ascii));
    if ( error ) {
        return EXIT_FAILURE;
    } // else:
    world.start();

    QObject::connect(&player, SIGNAL(Destroyed()),
        &screen, SLOT(DeathScreen()));

    QObject::connect(&freg, SIGNAL(aboutToQuit()),
        &screen, SLOT(CleanAll()));
    QObject::connect(&freg, SIGNAL(aboutToQuit()),
        &player, SLOT(CleanAll()));
    QObject::connect(&freg, SIGNAL(aboutToQuit()),
        &world, SLOT(CleanAll()));

    QObject::connect(&screen, SIGNAL(ExitReceived()), &freg, SLOT(quit()));
    QObject::connect(&world,  SIGNAL(ExitReceived()), &freg, SLOT(quit()));

    return freg.exec();
}
