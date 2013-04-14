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

#include "blocks.h"
#include "BlockManager.h"
#include "CraftManager.h"
#include <QDataStream>

bool Inventory::Access() const { return true; }

ushort Inventory::Start() const { return 0; }

Inventory * Inventory::HasInventory() { return this; }

ushort Inventory::Size() const { return size; }

int Inventory::Drop(const ushort num, Inventory * const inv_to) {
	if ( !inv_to )
		return 1;
	if ( num>=Size() )
		return 6;
	if ( inventory[num].isEmpty() )
		return 6;
	if ( !inv_to->Get(inventory[num].top()) )
		return 2;
	Pull(num);
	return 0;
}

int Inventory::GetAll(Inventory * const from) {
	if ( !from )
		return 1;
	if ( !from->Access() )
		return 2;

	for (ushort i=0; i<from->Size(); ++i)
		while ( from->Number(i) )
			if ( from->Drop(i, this) )
				return 3;
	return 0;
}

usage_types Inventory::Use() { return OPEN; }

void Inventory::Pull(const ushort num) {
	if ( !inventory[num].isEmpty() )
		inventory[num].pop();
}

void Inventory::SaveAttributes(QDataStream & out) const {
	for (ushort i=0; i<Size(); ++i) {
		out << Number(i);
		for (ushort j=0; j<Number(i); ++j)
			inventory[i].top()->SaveToFile(out);
	}
}

bool Inventory::Get(Block * const block) {
	if ( !block )
		return true;

	for (ushort i=Start(); i<Size(); ++i)
		if ( GetExact(block, i) )
			return true;
	return false;
}

bool Inventory::GetExact(Block * const block, const ushort num) {
	if ( inventory[num].isEmpty() ||
			( *block==*inventory[num].top() &&
			Number(num)<max_stack_size ) )
	{
		inventory[num].push(block);
		return true;
	}
	return false;
}

int Inventory::InscribeInv(const ushort num, const QString & str) {
	const int number=Number(num);
	if ( !number )
		return 0;
	if ( !inventory[num].top()->Inscribable() )
		return 1;
	const int sub=inventory[num].top()->Sub();
	if ( inventory[num].top()==block_manager.NormalBlock(sub) ) {
		for (ushort i=0; i<number; ++i)
			inventory[num].replace(i, block_manager.NormalBlock(sub));
	}
	for (ushort i=0; i<number; ++i)
		inventory[num].at(i)->Inscribe(str);
	return 0;
}

QString & Inventory::InvFullName(QString & str, const ushort i) const {
	return str=( inventory[i].isEmpty() ) ? "" :
		inventory[i].top()->FullName(str);
}

QString & Inventory::NumStr(QString & str, const ushort i) const {
	return str=( 1<Number(i) ) ?
		QString(" (%1x)").arg(Number(i)) :
		"";
}

float Inventory::GetInvWeight(const ushort i) const {
	return ( inventory[i].isEmpty() ) ? 0 :
		inventory[i].top()->Weight()*Number(i);
}

int Inventory::GetInvSub(const ushort i) const {
	return ( inventory[i].isEmpty() ) ? AIR :
		inventory[i].top()->Sub();
}

int Inventory::GetInvKind(const ushort i) const {
	return ( inventory[i].isEmpty() ) ? BLOCK :
		inventory[i].top()->Kind();
}

QString & Inventory::GetInvNote(QString & str, const ushort num) const {
	return str=inventory[num].top()->GetNote(str);
}

float Inventory::InvWeightAll() const {
	float sum=0;
	for (ushort i=0; i<Size(); ++i)
		sum+=GetInvWeight(i);
	return sum;
}

Block * Inventory::ShowBlock(const ushort num) const {
	return ( num>Size() || inventory[num].isEmpty() ) ?
		0 : inventory[num].top();
}

bool Inventory::HasRoom() {
	for (ushort i=Start(); i<Size(); ++i) {
		if ( inventory[i].isEmpty() ) {
			return true;
		}
	}
	return false;
}

bool Inventory::IsEmpty() {
	for (ushort i=0; i<Size(); ++i) {
		if ( !inventory[i].isEmpty() ) {
			return false;
		}
	}
	return true;
}

quint8 Inventory::Number(const ushort i) const { return inventory[i].size(); }

int Inventory::MiniCraft(const ushort num) {
	const ushort size=inventory[num].size();
	if ( !size )
		return 1; //empty
	craft_item item={
		size,
		GetInvKind(num),
		GetInvSub(num)
	};
	craft_item result;

	if ( craft_manager.MiniCraft(item, result) ) {
		while ( !inventory[num].isEmpty() ) {
			Block * const to_drop=ShowBlock(num);
			Pull(num);
			block_manager.DeleteBlock(to_drop);
		}
		for (ushort i=0; i<result.num; ++i)
			Get(block_manager.NewBlock(result.kind, result.sub));
		return 0; //success
	}
	return 2; //no such recipe
}

Inventory::Inventory(const ushort sz) :
		size(sz)
{
	inventory=new QStack<Block *>[Size()];
}

Inventory::Inventory(QDataStream & str, const ushort sz) :
		size(sz)
{
	inventory=new QStack<Block *>[Size()];
	for (ushort i=0; i<Size(); ++i) {
		quint8 num;
		str >> num;
		while ( num-- )
			inventory[i].push(block_manager.BlockFromFile(str));
	}
}

Inventory::~Inventory() {
	for (ushort i=0; i<Size(); ++i)
		while ( !inventory[i].isEmpty() ) {
			Block * const block=inventory[i].pop();
			block_manager.DeleteBlock(block);
		}
	delete [] inventory;
}
