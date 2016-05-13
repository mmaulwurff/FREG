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
 *  Provides definition for class Block. */

#ifndef BLOCK_H
#define BLOCK_H

#include <QtGlobal>

#define BLOCK_CONSTRUCTORS(BlockClass) \
BlockClass(subs);                       \
BlockClass(kinds, subs);                 \
BlockClass(QDataStream&, kinds, subs);    \
BlockClass& operator=(const BlockClass&) = delete;

#define CONSTRUCT_AS_PARENT(BlockClass, ParentClass) \
BlockClass::BlockClass(const kinds kind, const subs sub) \
    : ParentClass(kind, sub) {} \
BlockClass::BlockClass(QDataStream& stream, const kinds kind, const subs sub) \
    : ParentClass(stream, kind, sub) {}

enum wearable {
    WEARABLE_NOWHERE,
    WEARABLE_HEAD,
    WEARABLE_BODY,
    WEARABLE_LEGS,
    WEARABLE_OTHER,
    WEARABLE_VESSEL
};

/// weights in measures - mz (mezuro)
enum weights {
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

enum kinds : quint8;
enum subs  : quint8;
enum dirs  : int;
enum usage_types : int;
enum push_reaction : int;

/** Block without special physics and attributes. */
class Block {
public:
    BLOCK_CONSTRUCTORS(Block)
    virtual ~Block();

    virtual QString FullName() const;
    virtual bool Catchable() const;
    /// Returns true on success.
    virtual bool Inscribe(const QString& str);
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

    virtual class Falling*   ShouldFall();
    virtual class Animal*    IsAnimal();
    virtual class Active*    ActiveBlock();
    virtual const class Active* ActiveBlockConst() const;

    class Inventory* HasInventory();
    const class Inventory* HasConstInventory() const;

    virtual wearable Wearable() const;
    virtual int DamageKind() const;
    virtual int DamageLevel() const;

    /// Get light radius (the same as level). Non-Actives should return 0.
    virtual int LightRadius() const;
    virtual int Weight() const;
    /// Receive text signal.
    virtual void ReceiveSignal(const QString&);
    virtual QString Description() const;

    /// Set maximum durability.
    void Restore();
    /// Set durability to null.
    void Break();
    /// Increase durability, no more than MAX_DURABILITY.
    void Mend(int plus);
    bool SetDir(int dir);

    int GetDurability() const { return durability; }
    QString GetNote() const;

    int Transparent() const { return transparent; }
    subs  Sub()  const;
    kinds Kind() const;
    dirs GetDir() const;

    bool operator==(const Block&) const;

    void SaveToFile(QDataStream& out) const;
    void SaveNormalToFile(QDataStream& out) const;

    static sub_groups GetSubGroup(int sub);
    static dirs MakeDirFromDamage(int damage_kind);

    /// 10 bits to store durability in file, signed.
    static const int MAX_DURABILITY;

    static void TestDamage();

    static void operator delete(void* ptr, std::size_t size);

protected:

    virtual void SaveAttributes(QDataStream&) const;
    Block* DropInto(bool* delete_block);

    quint16 noteId;

private:

    Q_DECL_RELAXED_CONSTEXPR static int Transparency(int sub);

    bool IsNormal() const;

    friend class BlockFactory;

    qint16 durability;
    const quint8 transparent;
    const quint8 blockKind;
    const quint8 substance;
    quint8 direction;
};

#endif // BLOCK_H
