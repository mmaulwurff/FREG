	/*
	*This file is part of FREG.
	*
	*FREG is free software: you can redistribute it and/or modify
	*it under the terms of the GNU General Public License as published by
	*the Free Software Foundation, either version 3 of the License, or
	*(at your option) any later version.
	*
	*FREG is distributed in the hope that it will be useful,
	*but WITHOUT ANY WARRANTY; without even the implied warranty of
	*MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	*GNU General Public License for more details.
	*
	*You should have received a copy of the GNU General Public License
	*along with FREG. If not, see <http://www.gnu.org/licenses/>.
	*/

#include <blocks.h>
#include <QDataStream>
#include <QString>
#include "BlockManager.h"

QString & Block::FullName(QString & str) const {
	switch (sub) {
		case STAR: case SUN_MOON: case SKY:
		case AIR:        return str="Air";
		case WATER:      return str="Ice";
		case STONE:      return str="Stone";
		case MOSS_STONE: return str="Moss stone";
		case NULLSTONE:  return str="Nullstone";
		case GLASS:      return str="Glass";
		case SOIL:       return str="Soil";
		case HAZELNUT:   return str="Hazelnut";
		case WOOD:       return str="Wood";
		case GREENERY:   return str="Leaves";
		case ROSE:       return str="Rose";
		case A_MEAT:     return str="Animal meat";
		case H_MEAT:     return str="Not animal meat";
		case IRON:       return str="Iron block";
		default:
			fprintf(stderr,
				"Block::FullName: unlisted sub: %d.\n",
				sub);
			return str="Unknown block";
	}
}

void Block::SetTransparency(const quint8 transp) {
	if ( UNDEF==transp )
		switch ( sub ) {
			case AIR: transparent=INVISIBLE; break;
			case WATER: case GREENERY:
			case GLASS: transparent=BLOCK_TRANSPARENT; break;
			default: transparent=BLOCK_OPAQUE;
		}
	else
		transparent=transp;
}

int Block::Damage(
		const ushort dmg,
		const int dmg_kind=CRUSH)
{
	switch ( sub ) {
		case DIFFERENT:
			if ( TIME==dmg_kind )
				return 0;
			//no break, only time damages DIFFERENT
		case NULLSTONE:
		case STAR:
		case AIR:
		case SKY:
		case SUN_MOON:
		case WATER:
			return durability;
		case MOSS_STONE:
		case STONE:
			return ( MINE==dmg_kind ) ?
				durability-=2*dmg :
				durability-=dmg;
		case GLASS:
			return durability=0;
		case GREENERY:
		case ROSE:
		case HAZELNUT:
		case WOOD:
			return (CUT==dmg_kind) ?
				durability-=2*dmg :
				durability-=dmg;
		case SAND:
		case SOIL:
			return (DIG==dmg_kind) ?
				durability-=2*dmg :
				durability-=dmg;
		case A_MEAT:
		case H_MEAT:
			return (THRUST==dmg_kind) ?
				durability-=2*dmg :
				durability-=dmg;
		default:
			return durability-=dmg;
	}
}

Block * Block::DropAfterDamage() const {
	return ( BLOCK==Kind() && GLASS!=sub ) ?
		block_manager.NormalBlock(sub) : 0;
}

void Block::SetInMemoryChunk(bool in) { inMemoryChunk=in; }

bool Block::InMemoryChunk() const { return inMemoryChunk; }

int  Block::Kind() const { return BLOCK; }

bool Block::Catchable() const { return true; }

int  Block::Movable() const { return ( AIR==Sub() ) ? ENVIRONMENT : NOT_MOVABLE; }

int  Block::BeforePush(const int) { return NO_ACTION; }

int  Block::Move(const int) { return 0; }

bool Block::Armour() const { return false; }

bool Block::IsWeapon() const { return false; }

bool Block::Carving() const { return false; }

int  Block::DamageKind() const { return CRUSH; }

void Block::Restore() { durability=MAX_DURABILITY; }

void Block::NullWeight(const bool null) { nullWeight=null; }

void Block::SetDir(const int dir) { direction=dir; }

int  Block::GetDir() const { return direction; }

int  Block::Sub() const { return sub; }

int  Block::Transparent() const { return transparent; }

short Block::Durability() const { return durability; }

float Block::Weight() const { return nullWeight ? 0 : TrueWeight(); }

uchar Block::LightRadius() const { return 0; }

QString & Block::GetNote(QString & str) const { return str=note; }

ushort Block::DamageLevel() const { return 1; }

before_move_return Block::BeforeMove(const int) { return NOTHING; }

usage_types Block::Use() { return NO; }

Inventory * Block::HasInventory() { return 0; }

Animal * Block::IsAnimal() { return 0; }

Active * Block::ActiveBlock() { return 0; }

int Block::Temperature() const {
	switch (sub) {
		case WATER: return -100;
		default: return 0;
	}
}

bool Block::Inscribable() const {
	//TODO: prevent inscribing living creatures
	//IDEA: add names to living creatures (maybe taiming)
	return !(
		sub==AIR       ||
		sub==NULLSTONE ||
		sub==A_MEAT    ||
		sub==GREENERY  );
}

bool Block::CanBeOut() const {
	switch (sub) {
		case HAZELNUT: return false;
		default: return true;
	}
}

bool Block::Inscribe(const QString & str) {
	if ( Inscribable() ) {
		note=str;
		return true;
	}
	return false;
}

void Block::SaveAttributes(QDataStream &) const {}

bool Block::operator==(const Block & block) const {
	return ( block.Kind()==Kind() &&
			block.Sub()==Sub() &&
			block.Durability()==Durability() &&
			block.note==note );
}

float Block::TrueWeight() const {
	switch ( Sub() ) {
		case NULLSTONE: return 4444;
		case SOIL:      return 1500;
		case GLASS:     return 2500;
		case WOOD:      return 999;
		case IRON:      return 7874;
		case GREENERY:  return 2;
		case SAND:      return 1250;
		case ROSE:
		case HAZELNUT:  return 0.1f;
		case MOSS_STONE:
		case STONE:     return 2600;
		case A_MEAT:    return 1;
		case H_MEAT:    return 1;
		case AIR:       return 0;
		default: return 1000;
	}
}

void Block::SaveToFile(QDataStream & out) const {
	const bool normal=(this==block_manager.NormalBlock(Sub()));
	out << (quint16)Kind() << sub << normal;

	if ( normal ) {
		return;
	}

	out << nullWeight
		<< direction
		<< durability
		<< note;
	SaveAttributes(out);
}

Block::Block(
		const int sb, 
		const quint8 transp) //see defaults in class declaration
		:
		inMemoryChunk(true),
		sub(sb),
		nullWeight(false),
		direction(UP),
		note(""),
		durability(MAX_DURABILITY)
{
	SetTransparency(transp);
}

Block::Block(
		QDataStream & str,
		const int sub_,
		const quint8 transp)
		:
		inMemoryChunk(true),
		sub(sub_)
{
	SetTransparency(transp);
	str >> nullWeight >>
		direction >>
		durability >> note;
}

Block::~Block() {}

