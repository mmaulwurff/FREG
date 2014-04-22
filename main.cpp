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
#include "world.h"
#include "Player.h"
#include "worldmap.h"

#ifdef CURSED_SCREEN
    #include "screens/CursedScreen.h"
    #include <QCoreApplication>
    typedef QCoreApplication Application;
#else
    #ifdef STUB_SCREEN
        #include "screens/StubScreen.h"
        #include <QCoreApplication>
        typedef QCoreApplication Application;
    #else
        #include <QApplication>
        typedef QApplication Application;
    #endif
#endif

bool IsValidWorldName(const QString world_name) {
    const QStringList invalid_world_names(QStringList()
            << "blocks"
            << "moc"
            << "obj"
            << "recipes"
            << "texts");
    if ( world_name.left(5)=="help_" ) {
        printf("Invalid world name: %s.\n",
            qPrintable(world_name));
        return false;
    }
    for (auto i = invalid_world_names.constBegin();
            i < invalid_world_names.constEnd(); ++i)
    {
        if ( *i == world_name ) {
            printf("Invalid world name: %s.\n",
                qPrintable(world_name));
            return false;
        }
    }
    return true;
}

int main(int argc, char ** argv) {
    setlocale(LC_CTYPE, "C-UTF-8");
    QDir::current().mkdir("texts");
    freopen("texts/errors.txt", "wt", stderr);

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
    const QCommandLineOption ascii(QStringList() << "a" << "ascii",
        QObject::tr("Use ASCII-characters only."));
    parser.addOption(ascii);
    const QCommandLineOption world_argument(QStringList() << "w" << "world",
        QObject::tr("Specify world."),
        QObject::tr("world_name"));
    parser.addOption(world_argument);
    const QCommandLineOption generate(QStringList() << "g" << "generate",
        QObject::tr("Generate new map."));
    parser.addOption(generate);
    const QCommandLineOption map_size(QStringList() << "s" << "size",
        QObject::tr("Generated map size. Works only with -g."),
        QObject::tr("map_size"), QString::number(DEFAULT_MAP_SIZE));
    parser.addOption(map_size);
    const QCommandLineOption map_outer(QStringList() << "o" << "outer",
        QObject::tr("Generated map outer shred. Works only with -g."),
        QObject::tr("map_outer"), QString(QChar(OUT_BORDER_SHRED)));
    parser.addOption(map_outer);
    const QCommandLineOption map_seed(QStringList() << "d" << "seed",
        QObject::tr("Seed to generate map. Works only with -g."),
        QObject::tr("map_seed"), QString::number(0));
    parser.addOption(map_seed);
    parser.process(freg);

    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings sett(QDir::currentPath()+"/freg.ini", QSettings::IniFormat);
    const QString worldName =
        ( parser.isSet(world_argument)
            && IsValidWorldName(parser.value(world_argument)) ) ?
        parser.value(world_argument) :
        sett.value("current_world", "mu").toString();
    sett.setValue("current_world", worldName);

    if ( parser.isSet(generate) ) {
        QDir::current().mkdir(worldName);
        WorldMap::GenerateMap(
            qPrintable(worldName + "/map.txt"),
            parser.value(map_size).toUShort(),
            parser.value(map_outer).at(0).toLatin1(),
            parser.value(map_seed).toInt());
        puts(qPrintable(QObject::tr("Map generated successfully.")));
        return EXIT_SUCCESS;
    }

    qsrand(QTime::currentTime().msec());
    World world(worldName);
    Player player;
    int error = NO_ERROR;
    const Screen screen(&world, &player, error, parser.isSet(ascii));
    if ( error ) return EXIT_FAILURE;

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

    world.start();
    return freg.exec();
}
