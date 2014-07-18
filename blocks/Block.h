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

// weights in measures - mz (mezuro)
    const int WEIGHT_NULLSTONE = 1000;
    const int WEIGHT_SAND = 100;
    const int WEIGHT_WATER = 50;
    const int WEIGHT_GLASS = 150;
    const int WEIGHT_IRON = 300;
    const int WEIGHT_GREENERY = 3;
    const int WEIGHT_MINIMAL = 1;
    const int WEIGHT_STONE = 200;
    const int WEIGHT_AIR = 0;

class Inventory;
class Active;
class Falling;
class Animal;

class Block {
    /**\class Block Block.h
     * \brief Block without special physics and attributes. */
public:
    Block(int sub, int id, int transp = UNDEF);
    Block(QDataStream &, int sub, int id, int transp = UNDEF);
    virtual ~Block();

    Block & operator=(const Block &) = delete;
    Block(const Block &) = delete;

    virtual QString FullName() const;
    virtual int  Kind() const;
    virtual bool Catchable() const;
    /// Returns true on success.
    virtual bool Inscribe(QString str);
    virtual void Push(dirs, Block * who);
    virtual void Move(dirs direction);
    virtual void Damage(int dmg, int dmg_kind);
    virtual usage_types Use(Block * who);
    virtual push_reaction PushResult(dirs) const;
    /// Should return dropped block.
    /** It can be pile(CONTAINER, DIFFERENT) containing all dropped blocks, or
     *  block itself.
     *  Set delete_self false if this block itself should not be deleted.
     *  (by default block is deleted, beware). */
    virtual Block * DropAfterDamage(bool * delete_self);

    virtual Inventory * HasInventory();
    virtual Animal * IsAnimal();
    virtual Active * ActiveBlock();
    virtual Falling * ShouldFall();

    virtual int Wearable() const;
    virtual int DamageKind() const;
    virtual int DamageLevel() const;

    virtual int LightRadius() const;
    virtual int Weight() const;
    /// Receive text signal.
    virtual void ReceiveSignal(QString);

    /// Determines kind and sub, unique for every kind-sub pair.
    int GetId() const;
    /// Set maximum durability.
    void Restore();
    /// Set durability to null.
    void Break();
    /// Increments durability, no more than MAX_DURABILITY.
    void Mend();
    void SetDir(int dir);

    dirs GetDir() const;
    int GetDurability() const;
    QString GetNote() const;
    inline int Transparent() const { return transparent; }
    inline int Sub() const { return sub; }

    bool operator==(const Block &) const;
    bool operator!=(const Block &) const;

    /// Important! If block will be used after save,
    /// call RestoreDurabilityAfterSave.
    void SaveToFile(QDataStream & out);
    void SaveNormalToFile(QDataStream & out) const;
    /// Importart! Use it if block won't be deleted after SaveToFile.
    void RestoreDurabilityAfterSave();

protected:
    virtual void SaveAttributes(QDataStream &) const;

    QString * note;

private:
    int Transparency(int transp, int sub) const;

    qint16 durability;
    const quint8 transparent;
    const quint8 sub;
    const quint16 id;
    quint8 direction;
};

#endif // BLOCK_H
