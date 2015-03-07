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

/**\file everything.cpp
 * \brief This file includes all cpp files of project.
 *
 * This file is used for one-unit compilation (unity build) for release.
 * One-unit compilation provides faster compilation, a smaller executable,
 * and (probably) higher level of optimization, as modules know more about
 * contents of each other. */

#include "blocks/Accumulator.cpp"
#include "blocks/Active.cpp"
#include "blocks/Animal.cpp"
#include "blocks/Armour.cpp"
#include "blocks/Block.cpp"
#include "blocks/blocks.cpp"
#include "blocks/Bucket.cpp"
#include "blocks/Containers.cpp"
#include "blocks/Dwarf.cpp"
#include "blocks/Filter.cpp"
#include "blocks/Illuminator.cpp"
#include "blocks/Inventory.cpp"
#include "blocks/RainMachine.cpp"
#include "blocks/Teleport.cpp"
#include "blocks/Weapons.cpp"
#include "blocks/Text.cpp"

#include "screens/CursedScreen.cpp"
#include "screens/IThread.cpp"
#include "screens/VirtScreen.cpp"

#include "BlockFactory.cpp"
#include "CraftManager.cpp"
#include "DeferredAction.cpp"
#include "Lighting.cpp"
#include "main.cpp"
#include "Player.cpp"
#include "Shred-gen-flat.cpp"
#include "Shred.cpp"
#include "ShredStorage.cpp"
#include "TrManager.cpp"
#include "Weather.cpp"
#include "World.cpp"
#include "worldmap.cpp"
#include "Xyz.cpp"
#include "AroundCoordinates.cpp"

#include "moc/moc_Active.cpp"
#include "moc/moc_Animal.cpp"
#include "moc/moc_blocks.cpp"
#include "moc/moc_Containers.cpp"
#include "moc/moc_CursedScreen.cpp"
#include "moc/moc_Dwarf.cpp"
#include "moc/moc_Filter.cpp"
#include "moc/moc_Illuminator.cpp"
#include "moc/moc_IThread.cpp"
#include "moc/moc_Player.cpp"
#include "moc/moc_RainMachine.cpp"
#include "moc/moc_Teleport.cpp"
#include "moc/moc_VirtScreen.cpp"
#include "moc/moc_World.cpp"

#include "res/qrc_resources.cpp"
