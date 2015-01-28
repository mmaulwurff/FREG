    /* freg, Free-Roaming Elementary Game with open and interactive world
    *  Copyright (C) 2012-2015 Alexander 'mmaulwurff' Kromm
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

#include "World.h"
#include "Player.h"
#include "worldmap.h"
#include "CraftManager.h"
#include "BlockFactory.h"
#include "TrManager.h"
#include "screens/CursedScreen.h"

GCC_IGNORE_WEFFCPP_BEGIN
#include <QDir>
#include <QTime>
#include <QSettings>
#include <QLockFile>
#include <QGuiApplication>
#include <QCommandLineParser>
GCC_IGNORE_WEFFCPP_END

#ifdef Q_OS_WIN32
const QString home_path = QDir::currentPath() + "/";
#else
//const QString home_path = QDir::homePath() + "/.freg/";
const QString home_path = QDir::currentPath() + "/";
#endif

int main(int argc, char ** argv) {
    setlocale(LC_CTYPE, "C-UTF-8");

    QGuiApplication freg(argc, argv);
    QCoreApplication::setOrganizationName("freg-team");
    QCoreApplication::setApplicationName("freg");
    QCoreApplication::setApplicationVersion(QString::number(VER));

    // unused as local variable, but important things are in constructor.
    TrManager tr_manager;

    if ( not QDir::home().mkpath(".freg") ) {
        fputs("Error creating game home directory", stdout);
        return EXIT_FAILURE;
    }

    QCommandLineParser parser; // to parse command line arguments
    parser.setApplicationDescription(QObject::tr("freg - 3d open world game"));
    parser.addHelpOption();
    parser.addVersionOption();
    const QCommandLineOption world_argument(QStringList() << "w" << "world",
        QObject::tr("Specify world."),
        QObject::tr("world_name"));
    parser.addOption(world_argument);
    const QCommandLineOption generate(QStringList() << "g" << "generate",
        QObject::tr("Generate new map."));
    parser.addOption(generate);
    const QCommandLineOption map_size(QStringList() << "s" << "size",
        QObject::tr("Generated map size. Works only with -g."),
        QObject::tr("map_size"), QString::number(WorldMap::DEFAULT_MAP_SIZE));
    parser.addOption(map_size);
    const QCommandLineOption map_outer(QStringList() << "o" << "outer",
        QObject::tr("Generated map outer shred. Works only with -g."),
        QObject::tr("map_outer"), QString(QChar(SHRED_OUT_BORDER)));
    parser.addOption(map_outer);
    const QCommandLineOption map_seed(QStringList() << "d" << "seed",
        QObject::tr("Seed to generate map. Works only with -g."),
        QObject::tr("map_seed"), QString::number(0));
    parser.addOption(map_seed);
    parser.process(freg);

    const QString worldName = parser.isSet(world_argument) ?
        parser.value(world_argument) :
        QSettings(home_path + "freg.ini", QSettings::IniFormat).
            value("current_world", "mu").toString();
    if ( not QDir(home_path).mkpath(worldName) ) {
        fputs("Error generating world.", stdout);
        return EXIT_FAILURE;
    }

    qsrand(QTime::currentTime().msec());
    if ( parser.isSet(generate) ) {
        WorldMap::GenerateMap(
            worldName,
            parser.value(map_size).toUShort(),
            parser.value(map_outer).at(0).toLatin1(),
            parser.value(map_seed).toInt());
        fputs("Map generated successfully.", stdout);
        return EXIT_SUCCESS;
    }
    qsrand(QTime::currentTime().msec());

    // unused as local variables, but important things are in constructors.
    BlockFactory blockManager;
    CraftManager craftManager;

    bool world_error = false;
    World world(worldName, &world_error);
    if ( world_error ) {
        puts(qPrintable(QObject::tr("Error loading world.")));
        return EXIT_FAILURE;
    }
    QLockFile lock_file(home_path + worldName + "/lock");
    if ( not lock_file.tryLock() ) {
        puts(qPrintable(
            QObject::tr("World \"%1\" is used by another instance of freg.")
                .arg(worldName)));
        return EXIT_FAILURE;
    }
    Player player;
    int error = SCREEN_NO_ERROR;
    const Screen screen(&player, error);

    if ( error ) return EXIT_FAILURE;

    QObject::connect(&screen, &Screen::ExitReceived,
        &freg, &QCoreApplication::quit, Qt::DirectConnection);
    QObject::connect(&world, &World::ExitReceived,
        &freg, &QCoreApplication::quit, Qt::DirectConnection);

    world.start();
    return freg.exec();
}
