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

#include "World.h"
#include "Player.h"
#include "worldmap.h"
#include "CraftManager.h"
#include "BlockFactory.h"
#include "TranslationsManager.h"
#include "screens/CursedScreen.h"
#include <QDir>
#include <QTime>
#include <QSettings>
#include <QLockFile>
#include <QCoreApplication>
#include <QCommandLineParser>

#ifdef Q_OS_WIN32
const QString home_path = "";
#else
//const QString home_path = QDir::homePath() + "/.freg/";
const QString home_path = "";
#endif

int main(int argc, char ** argv) {
    setlocale(LC_CTYPE, "C-UTF-8");

    QCoreApplication freg(argc, argv);
    QCoreApplication::setOrganizationName("freg-team");
    QCoreApplication::setApplicationName("freg");
    QCoreApplication::setApplicationVersion(VER);

    tr_manager = new TranslationsManager;

    if ( not QDir::home().mkpath(".freg") ) {
        fputs("Error creating game home directory", stdout);
        return EXIT_FAILURE;
    }
    if (freopen(qPrintable(home_path + "err.txt"), "wt", stderr) == nullptr) {
        fputs("Error opening errors.txt, writing errors to standard out.",
            stdout);
    }

    // parse arguments
    QCommandLineParser parser;
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

    BlockFactory blockManager;
    blockFactory = &blockManager;
    CraftManager craftManager;
    craft_manager = &craftManager;

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

    QObject::connect(&screen, SIGNAL(ExitReceived()), &freg, SLOT(quit()),
        Qt::DirectConnection);
    QObject::connect(&world,  SIGNAL(ExitReceived()), &freg, SLOT(quit()),
        Qt::DirectConnection);

    world.start();
    return freg.exec();
}
