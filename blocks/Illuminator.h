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

#ifndef ILLUMINATOR_H
#define ILLUMINATOR_H

#include "blocks/Active.h"

class Illuminator final : public Active {
    Q_OBJECT
public:
    BLOCK_CONSTRUCTORS(Illuminator)

    int  ShouldAct() const override;
    int  DamageKind() const override;
    int  LightRadius() const override;
    Block* DropAfterDamage(bool * delete_block) override;
    QString FullName() const override;
    wearable Wearable() const override;
    usage_types Use(Active *) override;
    inner_actions ActInner() override;

protected:
    void DoRareAction() override;
    void SaveAttributes(QDataStream & out) const override;

private:
    quint16 fuelLevel;
    bool isOn;
};

#endif // ILLUMINATOR_H
