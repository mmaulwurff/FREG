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

#include "blocks/blocks.h"
#include "World.h"
#include "BlockFactory.h"
#include "TrManager.h"
#include "SortedArray.h"
#include "Shred.h"
#include "RandomManager.h"

#include <QDataStream>
#include <QTextStream>
#include <QTime>

// Plate::
    QString Plate::FullName() const {
        TrString woodName  = QObject::tr("Wooden board");
        TrString stoneName = QObject::tr("Stone slab");

        switch ( Sub() ) {
        case WOOD:  return woodName;
        case MOSS_STONE:
        case STONE: return stoneName;
        default:    return Block::FullName();
        }
    }

    int Plate::Weight() const { return Block::Weight()/4; }
    push_reaction Plate::PushResult(dirs) const { return JUMP; }

// Ladder::
    QString Ladder::FullName() const {
        TrString stoneName = QObject::tr("Rock with ledges");
        TrString greenName = QObject::tr("Liana");

        switch ( Sub() ) {
        case MOSS_STONE:
        case STONE:    return stoneName;
        case GREENERY: return greenName;
        default:       return Block::FullName();
        }
    }

    bool Ladder::Catchable() const { return true; }
    int  Ladder::Weight() const { return Block::Weight()*3; }
    push_reaction Ladder::PushResult(dirs) const { return MOVE_UP; }

    Block* Ladder::DropAfterDamage(bool* const delete_block) {
        Block* const pile = BlockFactory::NewBlock(BOX, DIFFERENT);
        if ( STONE==Sub() || MOSS_STONE==Sub() ) {
            pile->HasInventory()->Get(BlockFactory::Normal(Sub()));
        } else {
            pile->HasInventory()->Get(this);
            *delete_block = false;
        }
        return pile;
    }

// Liquid::
    void Liquid::DoRareAction() {
        if ( Q_UNLIKELY(not IsSubAround(Sub()) || Sub()==SUB_CLOUD) ) {
            Damage(MAX_DURABILITY*2/World::SECONDS_IN_NIGHT, DAMAGE_TIME);
            if ( Q_UNLIKELY(GetDurability() <= 0) ) {
                World::GetWorld()->DestroyAndReplace(X(), Y(), Z());
                return;
            }
            if ( Q_UNLIKELY(Sub() == SUB_CLOUD) ) return;
        }
        if ( Q_UNLIKELY(Sub() == ACID || Sub() == STONE) ) {
            DamageAround();
        }
        const int random = RandomManager::getRandBit4();
        if ( Q_UNLIKELY(random < 4) ) {
            World::GetWorld()->Move(X(), Y(), Z(),
                static_cast<dirs>(random+2));
        }
    }

    int Liquid::DamageKind() const {
        switch ( Sub() ) {
        default:    return DAMAGE_NO;
        case ACID:  return DAMAGE_ACID;
        case STONE: return DAMAGE_HEAT;
        }
    }

    int  Liquid::ShouldAct()   const { return FREQUENT_RARE; }
    int  Liquid::DamageLevel() const { return MAX_DURABILITY / 20; }
    int  Liquid::LightRadius() const { return ( STONE==Sub() ) ? 3 : 0; }
    bool Liquid::Inscribe(const QString&) { return false; }
    wearable Liquid::Wearable() const { return WEARABLE_VESSEL; }
    inner_actions Liquid::ActInner() { return INNER_ACTION_NONE; }
    push_reaction Liquid::PushResult(dirs) const { return ENVIRONMENT; }

    void Liquid::Damage(const int dmg, const int dmg_kind) {
        if ( dmg_kind < DAMAGE_PUSH_UP ) {
            Falling::Damage(dmg, dmg_kind);
        }
    }

    Block* Liquid::DropAfterDamage(bool*) {
        return BlockFactory::Normal( ( Sub() == STONE ) ?
            STONE : AIR);
    }

    QString Liquid::FullName() const {
        TrString stoneName = tr("Lava");
        TrString meatName  = tr("Blood");

        switch ( Sub() ) {
        case STONE:  return stoneName;
        case H_MEAT:
        case A_MEAT: return meatName;
        default:     return TrManager::SubNameUpper(Sub());
        }
    }

// Grass::
    void Grass::DoRareAction() {
        int i=X(), j=Y();
        // increase this if grass grows too fast
        switch ( RandomManager::rand() & (FIRE==Sub() ? 3 : 255) ) {
        case 0: ++i; break;
        case 1: --i; break;
        case 2: ++j; break;
        case 3: --j; break;
        default: return;
        }
        World* const world = World::GetWorld();
        if ( Q_UNLIKELY(not world->InBounds(i, j)) ) return;
        const subs sub_near = world->GetBlock(i, j, Z())->Sub();
        int k = Z();
        if ( (AIR == sub_near
                    && IsBase(Sub(), world->GetBlock(i, j, k-1)->Sub() ) )
                || ( IsBase(Sub(), sub_near)
                    && AIR == world->GetBlock(i, j, ++k)->Sub() ) )
        {
            world->Build(BlockFactory::NewBlock(GRASS, Sub()), i, j, k);
        }
        if ( not IsBase(Sub(), world->GetBlock(X(), Y(), Z()-1)->Sub()) ) {
            world->DestroyAndReplace(X(), Y(), Z());
        } else if ( Q_UNLIKELY(FIRE == Sub()) ) {
            DamageAround();
            if ( RandomManager::getRandBit4() || IsSubAround(WATER) ) {
                Damage(2, DAMAGE_FREEZE);
            }
        }
    }

    bool Grass::IsBase(const subs own_sub, const subs ground) {
        static const SortedArray<subs,
            WOOD, GREENERY, H_MEAT, A_MEAT, SUB_NUT, ROSE, PAPER> FIRE_BASES;
        return ( GREENERY==own_sub && SOIL==ground ) ||
               ( FIRE    ==own_sub &&
                   std::binary_search(ALL(FIRE_BASES), ground) );
    }

    QString Grass::FullName() const {
        TrString fireName = tr("Fire");

        switch ( Sub() ) {
        case GREENERY: return TrManager::KindName(GRASS);
        case FIRE:     return fireName;
        default:       return Block::FullName();
        }
    }

    QString Grass::Description() const {
        TrString growDescription = tr("It grows on soil.");
        TrString fireDescription = tr("It burns");

        return (( Sub() != FIRE ) ?
            growDescription : fireDescription);
    }

    int  Grass::ShouldAct() const  { return FREQUENT_RARE; }
    int  Grass::LightRadius() const { return (FIRE == Sub()) ? 5 : 0; }
    inner_actions Grass::ActInner() { return INNER_ACTION_NONE; }
    Block* Grass::DropAfterDamage(bool*) {return BlockFactory::Normal(AIR);}

    int Grass::DamageKind() const {
        return (Sub() == FIRE) ? DAMAGE_HEAT : DAMAGE_NO;
    }

// Bush::
    int  Bush::ShouldAct() const  { return FREQUENT_RARE; }
    void Bush::ReceiveSignal(const QString& str){ Active::ReceiveSignal(str); }
    int  Bush::Weight() const { return Inventory::Weight() + Block::Weight(); }
    QString Bush::FullName() const { return TrManager::KindName(BUSH); }
    usage_types Bush::Use(Active*) { return USAGE_TYPE_OPEN; }
    inner_actions Bush::ActInner() { return INNER_ACTION_NONE; }

    QString Bush::Description() const {
        TrString description = tr("Food and wood source.");
        return description;
    }

    void Bush::DoRareAction() {
        if ( Q_UNLIKELY(0 == RandomManager::getRandBit8()) ) {
            Get(BlockFactory::NewBlock(WEAPON, SUB_NUT));
        }
    }

    void Bush::Damage(const int dmg, const int dmg_kind) {
        if ( dmg_kind >= DAMAGE_PUSH_UP ) {
            Push(X(), Y(), Z(), dmg_kind);
        } else {
            Block::Damage(dmg, dmg_kind);
        }
    }

    Block* Bush::DropAfterDamage(bool*) {
        Block* const pile = BlockFactory::NewBlock(BOX, DIFFERENT);
        Inventory* const pile_inv = pile->HasInventory();
        pile_inv->Get(BlockFactory::NewBlock(WEAPON, WOOD));
        pile_inv->Get(BlockFactory::NewBlock(WEAPON, SUB_NUT));
        return pile;
    }

    void Bush::SaveAttributes(QDataStream& out) const {
        Active   ::SaveAttributes(out);
        Inventory::SaveAttributes(out);
    }

    Bush::Bush(const kinds kind, const subs sub)
        : Active(kind, sub)
        , Inventory(BUSH_SIZE)
    {}

    Bush::Bush(QDataStream& str, const kinds kind, const subs sub)
        : Active(str, kind, sub)
        , Inventory(str, BUSH_SIZE)
    {}

// Door::
    void Door::Damage(const int dmg, const int dmg_kind) {
        if ( dmg_kind >= DAMAGE_PUSH_UP
                && not shifted
                && not locked
                && World::Anti(GetDir()) != MakeDirFromDamage(dmg_kind) )
        {
            movable = MOVABLE;
            shifted = World::GetWorld()->Move(X(), Y(), Z(), GetDir());
            movable = NOT_MOVABLE;
        }
        Block::Damage(dmg, dmg_kind);
    }

    void Door::ActFrequent() {
        if ( Q_UNLIKELY(shifted) ) {
            World* const world = World::GetWorld();
            movable = MOVABLE;
            shifted = !world->Move(X(), Y(), Z(), World::Anti(GetDir()));
            movable = NOT_MOVABLE;
        }
    }

    void Door::ReceiveSignal(const QString& signal) {
        if (Block::GetSubGroup(Sub()) == GROUP_METAL &&
                signal.contains(QChar::fromLatin1('-')))
        {
            Use(nullptr);
        }
    }

    int  Door::ShouldAct() const { return FREQUENT_SECOND; }
    push_reaction Door::PushResult(dirs) const { return movable; }

    QString Door::FullName() const {
        TrString lockedName = tr("Locked door");
        return Str("%1 (%2)").
            arg(locked ? lockedName : TrManager::KindName(DOOR)).
            arg(TrManager::SubName(Sub()));
    }

    usage_types Door::Use(Active*) {
        if (not (locked = !locked) && GetShred() != nullptr) {
            World* const world = World::GetWorld();
            const int my_x = X();
            const int my_y = Y();
            world->GetShred(my_x, my_y)->
                AddFalling(world->GetBlock(my_x, my_y, Z() + 1));
        }
        return USAGE_TYPE_INNER;
    }

    void Door::SaveAttributes(QDataStream& out) const {
        out << shifted << locked;
    }

    Door::Door(const kinds kind, const subs sub)
        : Active(kind, sub)
        , shifted(false)
        , locked(false)
    {}

    Door::Door(QDataStream& stream, const kinds kind, const subs sub)
        : Active(stream, kind, sub)
        , shifted()
        , locked()
    {
        stream >> shifted >> locked;
    }

// Clock::
    usage_types Clock::Use(Active* const who) {
        if ( who ) {
            TrString outerTimeString = tr("Outer time is %1.");
            who->ReceiveSignal( (GetNote().left(4) == Str("real")) ?
                outerTimeString.arg(QTime::currentTime().toString()) :
                World::GetCWorld()->TimeOfDayStr() );
        } else {
            SendSignalAround(GetNote());
        }
        return USAGE_TYPE_INNER;
    }

    QString Clock::FullName() const {
        TrString explosiveName = tr("Bomb");
        return (( Sub() == EXPLOSIVE ) ? explosiveName : Block::FullName());
    }

    void Clock::Damage(int, const int dmg_kind) {
        if ( dmg_kind >= DAMAGE_PUSH_UP ) {
            Use(nullptr);
        } else {
            alarmTime = timerTime = -1;
            Break();
        }
    }

    int Clock::ShouldAct() const  { return FREQUENT_RARE; }
    int Clock::Weight() const { return Block::Weight()/10; }
    wearable Clock::Wearable() const { return WEARABLE_OTHER; }

    inner_actions Clock::ActInner() {
        const World* const world = World::GetCWorld();
        const int current_time = world->TimeOfDay();
        int notify_flag = 1;
        if ( alarmTime == current_time ) {
            TrString alarmString = tr("Alarm. %1");
            Block::Inscribe(alarmString.arg(world->TimeOfDayStr()));
            ++notify_flag;
        } else if ( timerTime > 0 )  {
            --timerTime;
            Block::Inscribe(QString::number(timerTime));
        } else if ( timerTime == 0 ) {
            timerTime = -1;
            TrString timerString = tr("Timer fired. %1");
            Block::Inscribe(timerString.arg(world->TimeOfDayStr()));
            ++notify_flag;
        }
        if ( Q_UNLIKELY(Sub()==EXPLOSIVE) ) {
            return ( notify_flag>1 ) ?
                INNER_ACTION_EXPLODE : INNER_ACTION_ONLY;
        }

        TrString morningString = tr("Morning has come.");
        TrString dayString     = tr("Day has come.");
        TrString eveningString = tr("Evening has come.");
        TrString nightString   = tr("Night has come.");
        switch ( current_time ) {
        default: --notify_flag; break;
        case World::END_OF_NIGHT:   Inscribe(morningString); break;
        case World::END_OF_MORNING: Inscribe(dayString    ); break;
        case World::END_OF_NOON:    Inscribe(eveningString); break;
        case World::END_OF_EVENING: Inscribe(nightString  ); break;
        }
        if ( Q_UNLIKELY(notify_flag > 0) ) {
            Use(nullptr);
            return INNER_ACTION_MESSAGE;
        } else {
            return INNER_ACTION_ONLY;
        }
    }

    bool Clock::Inscribe(const QString& note) {
        Block::Inscribe(note);
        QString string(note);
        QTextStream txt_stream(&string);
        char c;
        txt_stream >> c;
        switch ( c ) {
        case 'a': {
            int alarm_hour;
            txt_stream >> alarm_hour >> alarmTime;
            alarmTime += alarm_hour*60;
            timerTime = -1;
            } break;
        case 't':
            txt_stream >> timerTime;
            alarmTime = -1;
            break;
        default:
            alarmTime = timerTime = -1;
            break;
        }
        return true;
    }

    void Clock::SaveAttributes(QDataStream& str) const {
        str << alarmTime << timerTime;
    }

    Clock::Clock(const kinds kind, const subs sub)
        : Active(kind, sub)
    {}

    Clock::Clock(QDataStream& str, const kinds kind, const subs sub)
        : Active(str, kind, sub)
    {
        str >> alarmTime >> timerTime;
    }

// Signaller:: section
    wearable Signaller::Wearable() const { return WEARABLE_OTHER; }

    void Signaller::Damage(const int damage_level, const int damage_kind) {
        Signal(damage_level);
        if ( not AbsorbDamage(static_cast<damage_kinds>(damage_kind)) ) {
            Break();
        }
    }

    usage_types Signaller::Use(Active*) {
        Signal(1);
        return USAGE_TYPE_INNER;
    }

    bool Signaller::AbsorbDamage(const damage_kinds damage_kind) const {
        switch ( Sub() ) {
        case IRON:
        case WOOD:
        case STONE: return damage_kind >= DAMAGE_PUSH_UP;
        default: return false;
        }
    }

    void Signaller::Signal(const int level) const {
        TrString dingString = tr("^ Ding! ^");

        switch ( Sub() ) {
        case WOOD:
        case STONE:
            SendSignalAround(QString(level/4 + 1, QChar::fromLatin1('-')));
            break;
        default: SendSignalAround(dingString); break;
        }
    }

    QString Signaller::FullName() const {
        TrString ironName  = tr("Bell");
        TrString stoneName = tr("Button");

        switch ( Sub() ) {
        case IRON:  return ironName;
        case WOOD:
        case STONE: return stoneName;
        default:    return Block::FullName();
        }
    }

    QString Signaller::Description() const {
        TrString description = tr("Reacts on touch.");
        return description;
    }

// Telegraph:: section
    QString Telegraph::sharedMessage;

    Telegraph::Telegraph(const kinds kind, const subs sub)
        : Active(kind, sub)
        , isReceiver(true)
    {}

    Telegraph::Telegraph(QDataStream& str, const kinds kind, const subs sub)
        : Active(str, kind, sub)
        , isReceiver()
    {
        str >> isReceiver;
    }

    void Telegraph::SaveAttributes(QDataStream& str) const {
        str << isReceiver;
    }

    int  Telegraph::ShouldAct() const { return FREQUENT_RARE; }
    void Telegraph::ReceiveSignal(const QString& str) { Inscribe(str); }
    wearable Telegraph::Wearable() const { return WEARABLE_OTHER; }

    bool Telegraph::Inscribe(const QString& str) {
        isReceiver = false;
        return Block::Inscribe(str);
    }

    usage_types Telegraph::Use(Active*) {
        isReceiver = not isReceiver;
        return USAGE_TYPE_INNER;
    }

    inner_actions Telegraph::ActInner() {
        if ( isReceiver ) {
            const QString note = GetNote();
            if ( note != sharedMessage && not sharedMessage.isEmpty() ) {
                Inscribe(sharedMessage);
                SendSignalAround(sharedMessage);
                return INNER_ACTION_MESSAGE;
            }
        } else {
            sharedMessage = GetNote();
            isReceiver    = true;
        }
        return INNER_ACTION_ONLY;
    }

// MedKit:: section
    wearable MedKit::Wearable() const { return WEARABLE_OTHER; }

    usage_types MedKit::Use(Active* const user) {
        if ( user
                && GROUP_MEAT == GetSubGroup(user->Sub())
                && GetDurability() > MAX_DURABILITY/10 )
        {
            user->Mend(MAX_DURABILITY/10);
            Damage(MAX_DURABILITY/10, DAMAGE_TIME);
        }
        return USAGE_TYPE_INNER;
    }

// Informer:: section
    wearable Informer::Wearable() const { return WEARABLE_OTHER; }

    usage_types Informer::Use(Active* const user) {
        TrString compassMessage = QObject::tr("Your direction: %1.");

        switch ( Sub() ) {
        case IRON: user->ReceiveSignal(compassMessage
            .arg(TrManager::DirName(user->GetDir()).toLower())); break;
        default: break;
        }
        return USAGE_TYPE_INNER;
    }

    QString Informer::FullName() const {
        TrString compassName = QObject::tr("Compass");

        switch ( Sub() ) {
        case IRON: return compassName;
        default:   return Block::FullName();
        }
    }
