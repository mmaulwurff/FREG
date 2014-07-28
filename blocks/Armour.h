#ifndef ARMOURS_H
#define ARMOURS_H

#include "blocks/Block.h"

class Armour : public Block {
public:
    using Block::Block;
    int  Kind() const override;
    void Damage(int dmg, int dmg_kind) override;
    int  Wearable() const override;
    int  DamageLevel() const override;
    QString FullName() const override;
private:
};

class Helmet : public Armour {
public:
    using Armour::Armour;
    int  Kind() const override;
    int  Wearable() const override;
    QString FullName() const override;
private:
};

class Boots : public Armour {
public:
    using Armour::Armour;
    int  Kind() const override;
    int  Wearable() const override;
    QString FullName() const override;
private:
};

#endif // ARMOURS_H
