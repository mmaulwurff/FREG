#ifndef TEXT_H
#define TEXT_H

#include "blocks/Block.h"
#include <QtGlobal>

class Text : public Block {
public:
    using Block::Block;

    bool Inscribe(QString) override;
    QString FullName() const override;
    usage_types Use(Active* who) override;
};

class Map : public Text {
public:
    BLOCK_CONSTRUCTORS(Map)

    wearable Wearable() const override;
    usage_types Use(Active* user) override;
    usage_types UseOnShredMove(Active* user) override;

protected:
    void SaveAttributes(QDataStream& out) const override;

private:
    /// coordinates map titled in. also ~center.
    qint64 longiStart, latiStart;
    quint16 savedShift;
    qint8 savedChar;
};

#endif // TEXT_H

