	/* freg, Free-Roaming Elementary Game with open and interactive world
	*  Copyright (C) 2012-2013 Alexander 'mmaulwurff' Kromm
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
	* along with FREG. If not, see <http://www.gnu.org/licenses/>.
	*/

#ifndef DWARF_H
#define DWARF_H

#include "Animal.h"
#include "Inventory.h"

class Dwarf : public Animal, public Inventory {
	Q_OBJECT

	static const uchar MIN_DWARF_LIGHT_RADIUS=2;
	quint8 activeHand;
	uchar lightRadius;
	quint16 NutritionalValue(int sub) const;
	void UpdateLightRadius();

	public:
	static const uchar ON_HEAD=0;
	static const uchar IN_RIGHT=1;
	static const uchar IN_LEFT=2;
	static const uchar ON_BODY=3;
	static const uchar ON_LEGS=4;

	uchar GetActiveHand() const;
	void  SetActiveHand(bool right);

	quint8 Kind() const;
	int Sub() const;
	int ShouldAct() const;
	QString FullName() const;
	ushort Weight() const;
	ushort Start() const;
	int DamageKind() const;
	ushort DamageLevel() const;
	bool Move(int direction);

	Inventory * HasInventory();
	bool Access() const;
	Block * DropAfterDamage() const;
	bool Inscribe(const QString & str);
	void MoveInside(ushort num_from, ushort num_to, ushort num);
	void ReceiveSignal(const QString &);

	protected:
	void SaveAttributes(QDataStream & out) const;

	public:
	uchar LightRadius() const;

	Dwarf(int sub, quint16 id);
	Dwarf(QDataStream & str, int sub, quint16 id);
}; // class Dwarf

#endif
