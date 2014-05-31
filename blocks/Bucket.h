#ifndef BUCKET_H
#define BUCKET_H

#include "Block.h"
#include "Inventory.h"

class Bucket : public Block, public Inventory {
public:
    Bucket(int sub, quint16 id);
    Bucket(QDataStream & str, int sub, quint16 id);

    quint8  Kind() const override;
    int     Sub() const override;
    int     Weight() const override;
    QString FullName() const override;
    bool    Get(Block * block, int start) override;
    void    Damage(int dmg, int dmg_kind) override;
    void    ReceiveSignal(QString str) override;
    usage_types Use(Block *) override;
    Inventory * HasInventory() override;

protected:
    void SaveAttributes(QDataStream & out) const override;
};

#endif // BUCKET_H
