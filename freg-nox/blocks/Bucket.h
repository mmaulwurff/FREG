#ifndef BUCKET_H
#define BUCKET_H

#include "Block.h"
#include "Inventory.h"

class Bucket : public Block, public Inventory {
public:
    Bucket(int sub, quint16 id);
    Bucket(QDataStream & str, int sub, quint16 id);

    quint8  Kind() const;
    int     Sub() const;
    ushort  Weight() const;
    QString FullName() const;
    bool    Get(Block * block, ushort start);
    void    Damage(ushort dmg, int dmg_kind);
    void    ReceiveSignal(const QString & str);
    void    SaveAttributes(QDataStream & out) const;
    usage_types Use(Block *);
    Inventory * HasInventory();
};

#endif // BUCKET_H
