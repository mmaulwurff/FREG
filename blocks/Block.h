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

/**\file Block.h
 * \brief Provides definition for class Block. */

#ifndef BLOCK_H
#define BLOCK_H

#include "header.h"
#include <cstdint>

#define BLOCK_CONSTRUCTORS(BlockClass)     \
BlockClass(kinds, subs);                    \
BlockClass(class QDataStream&, kinds, subs); \
BlockClass(const BlockClass &) = delete;      \
BlockClass &operator=(const BlockClass &) = delete;

enum wearable {
    WEARABLE_NOWHERE,
    WEARABLE_HEAD,
    WEARABLE_BODY,
    WEARABLE_LEGS,
    WEARABLE_OTHER,
    WEARABLE_VESSEL
};

enum weights { ///< weights in measures - mz (mezuro)
    WEIGHT_NULLSTONE = 1000,
    WEIGHT_WATER     =  500,
    WEIGHT_IRON      =  300,
    WEIGHT_STONE     =  200,
    WEIGHT_GLASS     =  150,
    WEIGHT_SAND      =  100,
    WEIGHT_MINIMAL   =    4,
    WEIGHT_AIR       =    0,
};

enum sub_groups {
    GROUP_NONE,
    GROUP_AIR,
    GROUP_MEAT,
    GROUP_METAL,
    GROUP_HANDY,
};

class Block {
    /**\class Block Block.h
     * \brief Block without special physics and attributes. */
public:
    BLOCK_CONSTRUCTORS(Block)
    virtual ~Block();

    virtual QString FullName() const;
    virtual bool Catchable() const;
    /// Returns true on success.
    virtual bool Inscribe(QString str);
    virtual void Move(dirs direction);
    virtual void Damage(int dmg, int dmg_kind);
    virtual usage_types Use(class Active* user);
    virtual usage_types UseOnShredMove(class Active* user);
    virtual push_reaction PushResult(dirs) const;
    /// Should return dropped block.
    /** It can be pile(BOX, DIFFERENT) containing all dropped blocks, or
     *  block itself.
     *  Set delete_self false if this block itself should not be deleted.
     *  (by default block is deleted, beware). */
    virtual Block* DropAfterDamage(bool* delete_self);

    virtual class Inventory* HasInventory();
    virtual class Active    * ActiveBlock();
    virtual class Falling   * ShouldFall();
    virtual class Animal    * IsAnimal();

    virtual wearable Wearable() const;
    virtual int DamageKind() const;
    virtual int DamageLevel() const;

    /// Get light radius (the same as level). Non-Actives should return 0.
    virtual int LightRadius() const;
    virtual int Weight() const;
    /// Receive text signal.
    virtual void ReceiveSignal(QString);
    virtual QString Description() const;

    /// Set maximum durability.
    void Restore();
    /// Set durability to null.
    void Break();
    /// Increase durability, no more than MAX_DURABILITY.
    void Mend(int plus);
    void SetDir(int dir);

    int GetDurability() const { return durability; }
    QString GetNote() const;

    int Transparent() const { return transparent; }
    subs  Sub()  const;
    kinds Kind() const;
    dirs GetDir() const;

    bool operator==(const Block&) const;

    /// Important! If block will be used after save,
    /// call RestoreDurabilityAfterSave.
    void SaveToFile(QDataStream& out);
    void SaveNormalToFile(class QDataStream& out) const;

    /// Important! Use it if block won't be deleted after SaveToFile.
    void RestoreDurabilityAfterSave() { durability >>= 4; }

    static sub_groups GetSubGroup(int sub);
    static dirs MakeDirFromDamage(int damage_kind);

    /// 10 bits to store durability in file, signed.
    static const int MAX_DURABILITY;

protected:
    virtual void SaveAttributes(class QDataStream&) const;
    Block* DropInto(bool* delete_block);

    uint16_t noteId;

private:
    static int Transparency(int sub);

    int16_t durability;
    const uint8_t transparent;
    const uint8_t kind;
    const uint8_t sub;
    uint8_t direction;
};

#endif // BLOCK_H
