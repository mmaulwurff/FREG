#include "world.h"
#include "Bucket.h"

Bucket::Bucket(const int sub, const quint16 id) :
        Block(sub, id),
        Inventory(1)
{}

Bucket::Bucket(QDataStream & str, const int sub, const quint16 id) :
        Block(str, sub, id),
        Inventory(str, 1)
{}

quint8  Bucket::Kind() const { return BUCKET; }
int     Bucket::Sub()  const { return Block::Sub(); }
Inventory * Bucket::HasInventory() { return Inventory::HasInventory(); }
void    Bucket::Damage(ushort /*dmg*/, int /*dmg_kind*/) { durability = 0; }
void    Bucket::ReceiveSignal(const QString & str) { Block::ReceiveSignal(str); }
usage_types Bucket::Use(Block *) { return USAGE_TYPE_POUR; }

QString Bucket::FullName() const {
    QString name;
    switch (GetInvSub(0)) {
        case AIR:   return QObject::tr("Empty bucket"); break;
        case WATER: name = QObject::tr("Bucket with water"); break;
        case STONE: name = QObject::tr("Bucket with lava"); break;
        default:
            fprintf(stderr, "Bucket::FullName: unlisted sub: %d\n.",
                GetInvSub(0));
            name = QObject::tr("Bucket with something");
    }
    name.append(QObject::tr(" (%1/%2 full)").
        arg(Number(0)).arg(MAX_STACK_SIZE));
    return name; 
}

bool Bucket::Get(Block * const block, const ushort start = 0) {
    if ( block->Kind() != LIQUID ) {
        return false;
    }
    for (int i=start; i<Size(); ++i) {
        if ( GetExact(block, i) ) {
            return true;
        }
    }
    return false;
}

void Bucket::SaveAttributes(QDataStream & out) const {
    Inventory::SaveAttributes(out);
}

ushort Bucket::Weight() const {
    return Block::Weight()/6+Inventory::Weight();
}
