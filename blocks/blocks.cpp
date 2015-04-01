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
#include <QTextStream>
#include <QTime>

// Plate::
    QString Plate::FullName() const {
        switch ( Sub() ) {
        case WOOD:  return QObject::tr("Wooden board");
        case MOSS_STONE:
        case STONE: return QObject::tr("Stone slab");
        default:    return Block::FullName();
        }
    }

    int Plate::Weight() const { return Block::Weight()/4; }
    push_reaction Plate::PushResult(dirs) const { return JUMP; }

// Ladder::
    QString Ladder::FullName() const {
        switch ( Sub() ) {
        case MOSS_STONE:
        case STONE:    return QObject::tr("Rock with ledges");
        case GREENERY: return QObject::tr("Liana");
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
        if ( not IsSubAround(Sub()) || Sub()==SUB_CLOUD ) {
            Damage(MAX_DURABILITY*2/World::SECONDS_IN_NIGHT, DAMAGE_TIME);
            if ( Q_UNLIKELY(GetDurability() <= 0) ) {
                World::GetWorld()->DestroyAndReplace(X(), Y(), Z());
                return;
            }
        }
        if ( Q_UNLIKELY(Sub() == SUB_CLOUD) ) return;
        if ( Sub() == ACID || Sub() == STONE ) {
            DamageAround();
        }
        const int random = qrand() & 15;
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
    bool Liquid::Inscribe(QString)   { return false; }
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
        switch ( Sub() ) {
        case STONE:  return tr("Lava");
        case H_MEAT:
        case A_MEAT: return tr("Blood");
        default:     return TrManager::SubNameUpper(Sub());
        }
    }

// Grass::
    void Grass::DoRareAction() {
        int i=X(), j=Y();
        // increase this if grass grows too fast
        switch ( qrand() & (FIRE==Sub() ? 3 : 255) ) {
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
            if ( (qrand() & 15) || IsSubAround(WATER) ) {
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
        switch ( Sub() ) {
        case GREENERY: return TrManager::KindName(GRASS);
        case FIRE:     return tr("Fire");
        default:       return Block::FullName();
        }
    }

    QString Grass::Description() const {
        return ( Sub() != FIRE ) ?
            tr("It grows on soil.") :
            tr("It burns");
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
    void Bush::ReceiveSignal(const QString str) { Active::ReceiveSignal(str); }
    int  Bush::Weight() const { return Inventory::Weight() + Block::Weight(); }
    QString Bush::FullName() const { return TrManager::KindName(BUSH); }
    QString Bush::Description() const { return tr("Food and wood source."); }
    usage_types Bush::Use(Active*) { return USAGE_TYPE_OPEN; }
    Inventory* Bush::HasInventory() { return this; }
    inner_actions Bush::ActInner() { return INNER_ACTION_NONE; }

    void Bush::DoRareAction() {
        if ( Q_UNLIKELY(0 == (qrand() & 255)) ) {
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

    Bush::Bush(const kinds kind, const subs sub) :
            Active(kind, sub),
            Inventory(BUSH_SIZE)
    {}

    Bush::Bush(QDataStream& str, const kinds kind, const subs sub) :
            Active(str, kind, sub),
            Inventory(str, BUSH_SIZE)
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

    int  Door::ShouldAct() const { return FREQUENT_SECOND; }
    push_reaction Door::PushResult(dirs) const { return movable; }

    QString Door::FullName() const {
        return Str("%1 (%2)").
            arg(locked ? tr("Locked door") : TrManager::KindName(DOOR)).
            arg(TrManager::SubName(Sub()));
    }

    usage_types Door::Use(Active*) {
        locked = !locked;
        return USAGE_TYPE_INNER;
    }

    void Door::SaveAttributes(QDataStream& out) const {
        out << shifted << locked;
    }

    Door::Door(const kinds kind, const subs sub) :
            Active(kind, sub),
            shifted(false),
            locked(false)
    {}

    Door::Door(QDataStream& str, const kinds kind, const subs sub) :
            Active(str, kind, sub),
            shifted(),
            locked()
    {
        str >> shifted >> locked;
    }

// Clock::
    usage_types Clock::Use(Active* const who) {
        if ( who ) {
            who->ReceiveSignal( (GetNote().left(4) == Str("real")) ?
                tr("Outer time is %1.").arg(QTime::currentTime().toString()) :
                World::GetConstWorld()->TimeOfDayStr() );
        } else {
            SendSignalAround(GetNote());
        }
        return USAGE_TYPE_INNER;
    }

    QString Clock::FullName() const {
        return ( Sub() == EXPLOSIVE ) ? tr("Bomb") : Block::FullName();
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
        const World* const world = World::GetConstWorld();
        const int current_time = world->TimeOfDay();
        int notify_flag = 1;
        if ( alarmTime == current_time ) {
            Block::Inscribe(tr("Alarm. %1").arg(world->TimeOfDayStr()));
            ++notify_flag;
        } else if ( timerTime > 0 )  {
            --timerTime;
            Block::Inscribe(QString().setNum(timerTime));
        } else if ( timerTime == 0 ) {
            timerTime = -1;
            Block::Inscribe(tr("Timer fired. %1").arg(world->TimeOfDayStr()));
            ++notify_flag;
        }
        if ( Q_UNLIKELY(Sub()==EXPLOSIVE) ) {
            return ( notify_flag>1 ) ?
                INNER_ACTION_EXPLODE : INNER_ACTION_ONLY;
        }
        switch ( current_time ) {
        default: --notify_flag; break;
        case World::END_OF_NIGHT:   Inscribe(tr("Morning has come.")); break;
        case World::END_OF_MORNING: Inscribe(tr("Day has come."    )); break;
        case World::END_OF_NOON:    Inscribe(tr("Evening has come.")); break;
        case World::END_OF_EVENING: Inscribe(tr("Night has come."  )); break;
        }
        if ( Q_UNLIKELY(notify_flag > 0) ) {
            Use(nullptr);
            return INNER_ACTION_MESSAGE;
        } else {
            return INNER_ACTION_ONLY;
        }
    }

    bool Clock::Inscribe(QString str) {
        Block::Inscribe(str);
        char c;
        QTextStream txt_stream(&str);
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

    Clock::Clock(const kinds kind, const subs sub) : Active(kind, sub) {}

    Clock::Clock(QDataStream& str, const kinds kind, const subs sub) :
            Active(str, kind, sub)
    {
        str >> alarmTime >> timerTime;
    }

// Bell:: section
    wearable Bell::Wearable() const { return WEARABLE_OTHER; }

    void Bell::Damage(int, int) {
        Bell::Use(nullptr);
        Break();
    }

    usage_types Bell::Use(Active*) {
        SendSignalAround(tr("^ Ding! ^"));
        return USAGE_TYPE_INNER;
    }

// Telegraph:: section
    QString Telegraph::sharedMessage;

    Telegraph::Telegraph(const kinds kind, const subs sub) :
            Active(kind, sub),
            isReceiver(true)
    {}

    Telegraph::Telegraph(QDataStream& str, const kinds kind, const subs sub) :
            Active(str, kind, sub),
            isReceiver()
    {
        str >> isReceiver;
    }

    void Telegraph::SaveAttributes(QDataStream& str) const {
        str << isReceiver;
    }

    int  Telegraph::ShouldAct() const { return FREQUENT_RARE; }
    void Telegraph::ReceiveSignal(const QString str) { Inscribe(str); }
    wearable Telegraph::Wearable() const { return WEARABLE_OTHER; }

    bool Telegraph::Inscribe(const QString str) {
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
        switch ( Sub() ) {
        case IRON: user->ReceiveSignal(QObject::tr("Your direction: %1.").
            arg(TrManager::DirName(user->GetDir()).toLower())); break;
        default: break;
        }
        return USAGE_TYPE_INNER;
    }

    QString Informer::FullName() const {
        switch ( Sub() ) {
        case IRON: return QObject::tr("Compass");
        default:   return Block::FullName();
        }
    }
