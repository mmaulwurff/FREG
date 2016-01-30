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

/** @file
 * This file includes all cpp files of project.
 *
 * This file is used for one-unit compilation (unity build) for release.
 * One-unit compilation provides faster compilation, a smaller executable,
 * and (probably) higher level of optimization, as modules know more about
 * contents of each other. */

#include "src/AroundCoordinates.cpp"
#include "src/BlockFactory.cpp"
#include "src/CraftManager.cpp"
#include "src/DeferredAction.cpp"
#include "src/IniSettings.cpp"
#include "src/Lighting.cpp"
#include "src/LoadingLineThread.cpp"
#include "src/main.cpp"
#include "src/Player.cpp"
#include "src/RandomManager.cpp"
#include "src/Shred.cpp"
#include "src/Shred-gen-flat.cpp"
#include "src/ShredStorage.cpp"
#include "src/TrManager.cpp"
#include "src/VisionRay.cpp"
#include "src/WaysTree.cpp"
#include "src/Weather.cpp"
#include "src/World.cpp"
#include "src/WorldMap.cpp"

#include "src/blocks/Accumulator.cpp"
#include "src/blocks/Active.cpp"
#include "src/blocks/Animal.cpp"
#include "src/blocks/Armour.cpp"
#include "src/blocks/Block.cpp"
#include "src/blocks/blocks.cpp"
#include "src/blocks/Bucket.cpp"
#include "src/blocks/Containers.cpp"
#include "src/blocks/Dwarf.cpp"
#include "src/blocks/Filter.cpp"
#include "src/blocks/Illuminator.cpp"
#include "src/blocks/Inventory.cpp"
#include "src/blocks/Pipe.cpp"
#include "src/blocks/RainMachine.cpp"
#include "src/blocks/Teleport.cpp"
#include "src/blocks/Text.cpp"
#include "src/blocks/Weapons.cpp"

#include "src/screens/CursedScreen.cpp"
#include "src/screens/VirtualScreen.cpp"

#include "moc/moc_Active.cpp"
#include "moc/moc_Animal.cpp"
#include "moc/moc_blocks.cpp"
#include "moc/moc_Containers.cpp"
#include "moc/moc_CursedScreen.cpp"
#include "moc/moc_Dwarf.cpp"
#include "moc/moc_Filter.cpp"
#include "moc/moc_Illuminator.cpp"
#include "moc/moc_Pipe.cpp"
#include "moc/moc_Player.cpp"
#include "moc/moc_RainMachine.cpp"
#include "moc/moc_Teleport.cpp"
#include "moc/moc_VirtualScreen.cpp"
#include "moc/moc_World.cpp"

#include "res/qrc_resources.cpp"
