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

/**\file Block.h
 * \brief Provides definition for class Block. */

#ifndef BLOCK_H
#define BLOCK_H

#include "header.h"

const QString SOUND_STRINGS[] = {
    "Ding!",
    "Ouch!"
};
const QString DING = SOUND_STRINGS[0];
const QString OUCH = SOUND_STRINGS[1];

enum WEARABLE {
    WEARABLE_NOWHERE,
    WEARABLE_HEAD,
    WEARABLE_ARM,
    WEARABLE_BODY,
    WEARABLE_LEGS
}; // enum WEARABLE

class Inventory;
class Active;
class Animal;

class Block {
    /**\class Block Block.h
     * \brief Block without special physics and attributes. */
public:
    Block(int sub, quint16 id, quint8 transp = UNDEF);
    Block(QDataStream &, int sub, quint16 id, quint8 transp = UNDEF);
    virtual ~Block();

    virtual QString FullName() const;
    virtual quint8 Kind() const;
    virtual bool Catchable() const;
    /// Returns true on success.
    virtual bool Inscribe(QString str);
    virtual int PushResult(int dir) const;
    virtual void Push(int dir, Block * who);
    virtual bool Move(int direction);
    virtual void Damage(ushort dmg, int dmg_kind);
    virtual usage_types Use(Block * who = 0);
    /// Usually returns new block of the same kind and sub (except glass).
    /** When reimplemented in derivatives, inside it you can create a pile,
     *  put several blocks in it, and return pile. */
    virtual Block * DropAfterDamage() const;

    virtual Inventory * HasInventory();
    virtual Animal * IsAnimal();
    virtual Active * ActiveBlock();

    virtual int Wearable() const;
    virtual int DamageKind() const;
    virtual ushort DamageLevel() const;

    virtual uchar LightRadius() const;
    virtual int Temperature() const;
    virtual ushort Weight() const;
    /// Receive text signal.
    virtual void ReceiveSignal(QString);

    /// Determines kind and sub, unique for every kind-sub pair.
    quint16 GetId() const;
    /// Set maximum durability.
    void Restore();
    void SetDir(int dir);

    int GetDir() const;
    int Sub() const;
    short GetDurability() const;
    QString GetNote() const;
    int Transparent() const;

    bool operator==(const Block &) const;
    bool operator!=(const Block &) const;

    void SaveToFile(QDataStream & out) const;
protected:
    virtual void SaveAttributes(QDataStream &) const;
    static quint16 IdFromKindSub(quint16 id, quint8 sub);

    QString * note;
    qint16 durability;
private:
    Block(Block const &);

    static const ushort MAX_NOTE_LENGTH = 144;
    quint8 Transparency(quint8 transp, int sub) const;

    const quint8 transparent;
    const quint8 sub;
    const quint16 id;
    quint8 direction;
};

#endif // BLOCK_H