	/* freg, Free-Roaming Elementary Game with open and interactive world
	*  Copyright (C) 2012-2013 Alexander 'mmaulwurff' Kromm
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
	* along with FREG. If not, see <http://www.gnu.org/licenses/>.
	*/

#include <QDataStream>
#include <memory>
#include "header.h"
#include "blocks.h"
#include "BlockManager.h"

const QString BlockManager::kinds[LAST_KIND]={
	"block",
	"bell",
	"chest",
	"pile",
	"intellectual",
	"animal",
	"pick",
	"telegraph",
	"liquid",
	"grass",
	"bush",
	"rabbit",
	"active",
	"clock",
	"plate",
	"workbench",
	"weapon",
	"ladder",
	"door",
	"locked door",
	"creator",
	"text",
	"map"
};

const QString BlockManager::subs[LAST_SUB]={
	"stone",
	"moss stone",
	"nullstone",
	"sky",
	"star",
	"sun or moon",
	"soil",
	"meat of intellectual",
	"animal meat",
	"glass",
	"wood",
	"different",
	"iron",
	"water",
	"greenery",
	"sand",
	"hazelnut",
	"rose",
	"clay",
	"air",
	"paper"
};

BlockManager block_manager;

BlockManager::BlockManager() {
	for (ushort sub=0; sub<LAST_SUB; ++sub) {
		normals[sub]=new Block(sub);
	}
}
BlockManager::~BlockManager() {
	for(ushort sub=0; sub<LAST_SUB; ++sub) {
		delete normals[sub];
	}
}

Block * BlockManager::NormalBlock(const int sub) {
	return normals[sub];
}

Block * BlockManager::NewBlock(const int kind, int sub) {
	if ( sub >= LAST_SUB  ) {
		fprintf(stderr,
			"BlockManager::NewBlock: unknown substance: %d.\n",
			sub);
		sub=STONE;
	}
	switch ( kind ) {
		case BLOCK:  return New<Block >(sub);
		case BELL:   return New<Bell  >(sub);
		case GRASS:  return New<Grass >(sub);
		case PICK:   return New<Pick  >(sub);
		case PLATE:  return New<Plate >(sub);
		case ACTIVE: return New<Active>(sub);
		case LADDER: return New<Ladder>(sub);
		case WEAPON: return New<Weapon>(sub);
		case BUSH:   return New<Bush  >(sub);
		case CHEST:  return New<Chest >(sub);
		case PILE:   return New<Pile  >(sub);
		case DWARF:  return New<Dwarf >(sub);
		case RABBIT: return New<Rabbit>(sub);
		case DOOR:   return New<Door  >(sub);
		case LIQUID: return New<Liquid>(sub);
		case CLOCK:  return New<Clock >(sub);
		case TEXT:   return New<Text  >(sub);
		case MAP:    return New<Map   >(sub);
		case CREATOR: return New<Creator>(sub);
		case WORKBENCH: return New<Workbench>(sub);
		default:
			fprintf(stderr,
				"BlockManager::NewBlock: unlisted kind: %d\n",
				kind);
			return New<Block>(sub);
	}
}

Block * BlockManager::BlockFromFile(QDataStream & str,
		const quint8 kind, const quint8 sub)
{
	switch ( kind ) {
		case BLOCK:  return New<Block >(str, sub);
		case BELL:   return New<Bell  >(str, sub);
		case PICK:   return New<Pick  >(str, sub);
		case PLATE:  return New<Plate >(str, sub);
		case LADDER: return New<Ladder>(str, sub);
		case WEAPON: return New<Weapon>(str, sub);
		case BUSH:   return New<Bush  >(str, sub);
		case CHEST:  return New<Chest >(str, sub);
		case RABBIT: return New<Rabbit>(str, sub);
		case DWARF:  return New<Dwarf >(str, sub);
		case PILE:   return New<Pile  >(str, sub);
		case GRASS:  return New<Grass >(str, sub);
		case ACTIVE: return New<Active>(str, sub);
		case LIQUID: return New<Liquid>(str, sub);
		case TEXT:   return New<Text  >(str, sub);
		case MAP:    return New<Map   >(str, sub);
		case LOCKED_DOOR:
		case DOOR:   return New<Door  >(str, sub);
		case CLOCK:  return New<Clock >(str, sub);
		case CREATOR: return New<Creator>(str, sub);
		case WORKBENCH: return New<Workbench>(str, sub);
		default:
			fprintf(stderr,
				"BlockManager::BlockFromFile: kind (?): %d\n.",
				kind);
			return New<Block>(str, sub);
	}
}

Block * BlockManager::BlockFromFile(QDataStream & str) {
	quint8 kind, sub;
	return KindSubFromFile(str, kind, sub) ?
		NormalBlock(sub) : BlockFromFile(str, kind, sub);
}

bool BlockManager::KindSubFromFile(QDataStream & str,
		quint8 & kind, quint8 & sub)
{
	quint8 data;
	str >> data;
	sub=(data & 0x7F);
	if ( data & 0x80 ) { // normal bit
		return true;
	}
	str >> kind;
	return false;
}

void BlockManager::DeleteBlock(Block * const block) {
	if ( block!=NormalBlock(block->Sub()) ) {
		delete block;
	}
}

QString BlockManager::KindToString(const quint8 kind) {
	return ( kind<LAST_KIND ) ?
		kinds[kind] : "unknown kind";
}

QString BlockManager::SubToString(const quint8 sub) {
	return ( sub<LAST_SUB ) ?
		subs[sub] : "unknown sub";
}

quint8 BlockManager::StringToKind(const QString & str) {
	int i=0;
	for ( ; i<LAST_KIND && kinds[i]!=str; ++i);
	return i;
}

quint8 BlockManager::StringToSub(const QString & str) {
	int i=0;
	for ( ; i<LAST_SUB && subs[i]!=str; ++i);
	return i;
}

template <typename Thing>
Thing * BlockManager::New(const int sub) {
	return new Thing(sub);
}

template <typename Thing>
Thing * BlockManager::New(QDataStream & str, const int sub) {
	return new Thing(str, sub);
}
