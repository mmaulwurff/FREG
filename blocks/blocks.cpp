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

#include <QTextStream>
#include <QFile>
#include <QTime>
#include "blocks.h"
#include "world.h"
#include "Shred.h"
#include "BlockManager.h"

// Plate::
    QString Plate::FullName() const {
        switch ( Sub() ) {
        case WOOD:  return QObject::tr("Wooden board");
        case IRON:  return QObject::tr("Iron plate");
        case MOSS_STONE:
        case STONE: return QObject::tr("Stone slab");
        default:
            fprintf(stderr, "%s: unlisted sub: %d.\n", Q_FUNC_INFO, Sub());
            return "Strange plate";
        }
    }

    int Plate::Kind() const { return PLATE; }
    int Plate::Weight() const { return Block::Weight()/4; }
    push_reaction Plate::PushResult(dirs) const { return JUMP; }

// Ladder::
    QString Ladder::FullName() const {
        switch ( Sub() ) {
        case WOOD:  return QObject::tr("Ladder");
        case MOSS_STONE:
        case STONE: return QObject::tr("Rock with ledges");
        case GREENERY: return QObject::tr("Liana");
        default:
            fprintf(stderr, "%s: unlisted sub: %d\n", Q_FUNC_INFO, Sub());
            return "Strange ladder";
    }
    }

    bool Ladder::Catchable() const { return true; }
    int  Ladder::Weight() const { return Block::Weight()*3; }
    int  Ladder::Kind() const { return LADDER; }
    push_reaction Ladder::PushResult(dirs) const { return MOVE_UP; }

    Block * Ladder::DropAfterDamage(bool * const delete_block) {
        Block * const pile = block_manager.NewBlock(CONTAINER, DIFFERENT);
        if ( STONE==Sub() || MOSS_STONE==Sub() ) {
            pile->HasInventory()->Get(block_manager.NormalBlock(Sub()));
        } else {
            pile->HasInventory()->Get(this);
            *delete_block = false;
        }
        return pile;
    }

// Animal::
    INNER_ACTIONS Animal::ActInner() {
        if ( satiation <= 0 ) {
            Damage(5, HUNGER);
        } else {
            --satiation;
            Mend();
        }
        emit Updated();
        return INNER_ACTION_NONE;
    }

    void Animal::DoRareAction() {
        if ( not IsSubAround(AIR) ) {
            if ( breath <= 0 ) {
                Damage(10, BREATH);
            } else {
                --breath;
            }
        } else if ( breath < MAX_BREATH ) {
            ++breath;
        }
        if ( GetDurability() <= 0 ) {
            GetWorld()->DestroyAndReplace(X(), Y(), Z());
        } else {
            emit Updated();
        }
    }

    int Animal::ShouldAct() const { return FREQUENT_SECOND | FREQUENT_RARE; }
    int Animal::Breath() const { return breath; }
    int Animal::Satiation() const { return satiation; }
    Animal * Animal::IsAnimal() { return this; }
    int Animal::DamageKind() const { return BITE; }

    bool Animal::Eat(const int sub) {
        const int value = NutritionalValue(sub);
        if ( value ) {
            satiation += value;
            ReceiveSignal(tr("Yum."));
            if ( SECONDS_IN_DAY < satiation ) {
                satiation = 1.1 * SECONDS_IN_DAY;
                ReceiveSignal(tr("You have gorged yourself!"));
            }
            return true;
        } else {
            ReceiveSignal(tr("You cannot eat this."));
            return false;
        }
    }

    void Animal::SaveAttributes(QDataStream & out) const {
        Falling::SaveAttributes(out);
        out << breath << satiation;
    }

    void Animal::EatGrass() {
        for (int x=X()-1; x<=X()+1; ++x)
        for (int y=Y()-1; y<=Y()+1; ++y) {
            if ( world->InBounds(x, y) &&
                    GREENERY == world->GetBlock(x, y, Z())->Sub() )
            {
                TryDestroy(x, y, Z());
                Eat(GREENERY);
                return;
            }
        }
    }

    Block * Animal::DropAfterDamage(bool *) {
        return block_manager.NewBlock(WEAPON, BONE);
    }

    Animal::Animal(const int sub, const int id) :
            Falling(sub, id, NONSTANDARD),
            breath(MAX_BREATH),
            satiation(SECONDS_IN_DAY)
    {}

    Animal::Animal(QDataStream & str, const int sub, const int id) :
            Falling(str, sub, id, NONSTANDARD)
    {
        str >> breath >> satiation;
    }

// Liquid::
    void Liquid::DoRareAction() {
        if ( not IsSubAround(Sub()) || Sub()==SUB_CLOUD ) {
            Damage(MAX_DURABILITY*2/SECONDS_IN_NIGHT, TIME);
            if ( GetDurability() <= 0 ) {
                GetWorld()->DestroyAndReplace(X(), Y(), Z());
                return;
            }
        }
        switch ( Sub() ) {
        case SUB_CLOUD: return;
        case ACID:
        case STONE: DamageAround(); // no break;
        default:
            switch ( qrand()%20 ) {
            case 0: GetWorld()->Move(X(), Y(), Z(), NORTH); break;
            case 1: GetWorld()->Move(X(), Y(), Z(), EAST ); break;
            case 2: GetWorld()->Move(X(), Y(), Z(), SOUTH); break;
            case 3: GetWorld()->Move(X(), Y(), Z(), WEST ); break;
            }
        }
    }

    int Liquid::DamageKind() const {
        switch ( Sub() ) {
        default:    return NO_HARM;
        case ACID:  return DAMAGE_ACID;
        case STONE: return HEAT;
        }
    }

    int Liquid::ShouldAct() const  { return FREQUENT_RARE; }
    int Liquid::Kind() const { return LIQUID; }
    push_reaction Liquid::PushResult(dirs) const { return ENVIRONMENT; }

    Block * Liquid::DropAfterDamage(bool *) {
        return block_manager.NormalBlock( ( Sub() == STONE ) ?
            STONE : AIR);
    }

    int Liquid::LightRadius() const {
        static const int radius = ( STONE==Sub() ) ? 3 : 0;
        return radius;
    }

    QString Liquid::FullName() const {
        switch ( Sub() ) {
        case WATER: return tr("Water");
        case STONE: return tr("Lava");
        case ACID:  return tr("Acid");
        case SUB_CLOUD: return tr("Cloud");
        default:
            fprintf(stderr, "%s: sub (?): %d\n", Q_FUNC_INFO, Sub());
            return "Unknown liquid";
        }
    }

// Grass::
    void Grass::DoRareAction() {
        World * const world = GetWorld();
        if ( FIRE == Sub() ) {
            DamageAround();
            if ( qrand()%10 || IsSubAround(WATER) ) {
                Damage(2, FREEZE);
            }
        }
        if ( not IsBase(Sub(), world->GetBlock(X(), Y(), Z()-1)->Sub()) ) {
            world->DestroyAndReplace(X(), Y(), Z());
            return;
        } // else:
        int i=X(), j=Y(), k=Z();
        // increase this if grass grows too fast
        switch ( qrand() % (FIRE==Sub() ? 4 : SECONDS_IN_HOUR*2) ) {
        case 0: ++i; break;
        case 1: --i; break;
        case 2: ++j; break;
        case 3: --j; break;
        default: return;
        }
        if ( not world->InBounds(i, j) ) return;
        const int sub_near = world->GetBlock(i, j, k)->Sub();
        if ( world->Enlightened(i, j, k) || FIRE == Sub() ) {
            if ( AIR == sub_near
                    && IsBase(Sub(), world->GetBlock(i, j, k-1)->Sub() ) )
            {
                world->Build(block_manager.NewBlock(GRASS, Sub()), i, j, k);
            } else if ( IsBase(Sub(), sub_near)
                    && AIR == world->GetBlock(i, j, ++k)->Sub() )
            {
                world->Build(block_manager.NewBlock(GRASS, Sub()), i, j, k);
            }
        }
    }

    bool Grass::IsBase(const int own_sub, const int ground) {
        return ( GREENERY==own_sub && SOIL==ground )
            || ( FIRE==own_sub && (
                WOOD==ground
                || GREENERY==ground
                || H_MEAT==ground
                || A_MEAT==ground
                || HAZELNUT==ground
                || ROSE==ground
                || PAPER==ground ) );
    }

    QString Grass::FullName() const {
        switch ( Sub() ) {
        case GREENERY: return tr("Grass");
        case FIRE:     return tr("Fire");
        default:
            fprintf(stderr, "%s: sub (?): %d\n", Q_FUNC_INFO, Sub());
            return "Unknown plant";
        }
    }

    int  Grass::ShouldAct() const  { return FREQUENT_RARE; }
    int  Grass::Kind() const { return GRASS; }
    push_reaction Grass::PushResult(dirs) const { return ENVIRONMENT; }

    Block * Grass::DropAfterDamage(bool *) {
        return block_manager.NormalBlock(AIR);
    }

    void Grass::Push(dirs, Block *) {
        GetWorld()->DestroyAndReplace(X(), Y(), Z());
    }

    int  Grass::LightRadius() const {
        static const int radius = (FIRE == Sub()) ? 5 : 0;
        return radius;
    }

// Bush::
    int  Bush::Sub() const { return Block::Sub(); }
    int  Bush::ShouldAct() const  { return FREQUENT_RARE; }
    void Bush::ReceiveSignal(const QString str) { Active::ReceiveSignal(str); }
    int  Bush::Kind() const { return BUSH; }
    int  Bush::Weight() const { return Inventory::Weight()+Block::Weight(); }
    QString Bush::FullName() const { return tr("Bush"); }
    usage_types Bush::Use(Block *) { return USAGE_TYPE_OPEN; }
    Inventory * Bush::HasInventory() { return this; }

    void Bush::DoRareAction() {
        if ( 0 == qrand()%(SECONDS_IN_HOUR*4) ) {
            Get(block_manager.NormalBlock(HAZELNUT));
        }
    }

    void Bush::Push(dirs, Block * const who) { Inventory::Push(who); }

    Block * Bush::DropAfterDamage(bool *) {
        Block * const pile = block_manager.NewBlock(CONTAINER, DIFFERENT);
        Inventory * const pile_inv = pile->HasInventory();
        pile_inv->Get(block_manager.NewBlock(WEAPON, WOOD));
        pile_inv->Get(block_manager.NormalBlock(HAZELNUT));
        return pile;
    }

    void Bush::SaveAttributes(QDataStream & out) const {
        Active   ::SaveAttributes(out);
        Inventory::SaveAttributes(out);
    }

    Bush::Bush(const int sub, const int id) :
            Active(sub, id),
            Inventory(BUSH_SIZE)
    {}

    Bush::Bush(QDataStream & str, const int sub, const int id) :
            Active(str, sub, id),
            Inventory(str, BUSH_SIZE)
    {}

// Rabbit::
    int Rabbit::Attractive(const int sub) const {
        switch ( sub ) {
        case GREENERY: return   1;
        case H_MEAT:   return -16;
        case A_MEAT:   return - 1;
        case SAND:     return - 1;
        default:       return   0;
        }
    }

    void Rabbit::DoRareAction() {
        // eat sometimes
        if ( SECONDS_IN_DAY/2 > Satiation() ) {
            EatGrass();
        }
        if ( not moved_in_this_turn ) {
            switch ( qrand()%60 ) {
            case 0: SetDir(NORTH); break;
            case 1: SetDir(SOUTH); break;
            case 2: SetDir(EAST);  break;
            case 3: SetDir(WEST);  break;
            default: if ( Gravitate(4, 1, 3, 4) ) {
                if ( qrand()%2 ) {
                    GetWorld()->Jump(X(), Y(), Z(), GetDir());
                } else {
                    GetWorld()->Move(X(), Y(), Z(), GetDir());
                }
            } return;
            }
            GetWorld()->Move(X(), Y(), Z(), GetDir());
        }
        moved_in_this_turn = false; // for next turn
        Animal::DoRareAction();
    }

    Block * Rabbit::DropAfterDamage(bool * delete_block) {
        Block * const pile = block_manager.NewBlock(CONTAINER, DIFFERENT);
        Inventory * const pile_inv = pile->HasInventory();
        pile_inv->Get(block_manager.NormalBlock(A_MEAT));
        pile_inv->Get(Animal::DropAfterDamage(delete_block));
        return pile;
    }

    QString Rabbit::FullName() const { return tr("Herbivore"); }
    int  Rabbit::Kind() const { return RABBIT; }
    void Rabbit::DoFrequentAction() {
        if ( Gravitate(2, 1, 2, 4) ) {
            if ( qrand()%2 ) {
                world->Jump(X(), Y(), Z(), GetDir());
            } else {
                world->Move(X(), Y(), Z(), GetDir());
            }
            moved_in_this_turn = true;
        }
    }

    int Rabbit::NutritionalValue(const int sub) const {
        return ( GREENERY == sub ) ? SECONDS_IN_HOUR*4 : 0;
    }

// Door::
    push_reaction Door::PushResult(dirs) const {
        return movable ? MOVABLE : NOT_MOVABLE;
    }

    void Door::Push(dirs, Block * const who) {
        if ( not shifted
                && not locked
                && World::Anti(GetDir())!=who->GetDir() )
        {
            movable = true;
            shifted = GetWorld()->Move(X(), Y(), Z(), GetDir());
            movable = false;
        }
    }

    void Door::DoFrequentAction() {
        if ( shifted ) {
            movable = true;
            World * const world = GetWorld();
            int x, y, z;
            world->Focus(X(), Y(), Z(), &x, &y, &z, World::Anti(GetDir()));
            if (ENVIRONMENT == world->GetBlock(x, y, z)->PushResult(NOWHERE)) {
                shifted = !world->Move(X(), Y(), Z(), World::Anti(GetDir()));
            }
            movable = false;
        }
    }

    int  Door::ShouldAct() const { return FREQUENT_SECOND; }
    int  Door::Kind() const { return locked ? LOCKED_DOOR : DOOR; }

    QString Door::FullName() const {
        QString sub_string;
        switch ( Sub() ) {
        case WOOD:  sub_string = tr(" of wood");  break;
        case MOSS_STONE:
        case STONE: sub_string = tr(" of stone"); break;
        case GLASS: sub_string = tr(" of glass"); break;
        case IRON:  sub_string = tr(" of iron");  break;
        default:
            sub_string = tr(" of something");
            fprintf(stderr, "%s: unlisted sub: %d\n", Q_FUNC_INFO, Sub());
        }
        return locked ? tr("Locked door") : tr("Door") + sub_string;
    }

    usage_types Door::Use(Block *) {
        locked = !locked;
        return USAGE_TYPE_NO;
    }

    void Door::SaveAttributes(QDataStream & out) const {
        Active::SaveAttributes(out);
        out << shifted << locked;
    }

    Door::Door(const int sub, const int id) :
            Active(sub, id, ( STONE==sub || MOSS_STONE==sub ) ?
                BLOCK_OPAQUE : NONSTANDARD),
            shifted(false),
            locked(false)
    {}

    Door::Door(QDataStream & str, const int sub, const int id) :
            Active(str, sub, id, ( STONE==sub || MOSS_STONE==sub ) ?
                BLOCK_OPAQUE : NONSTANDARD)
    {
        str >> shifted >> locked;
    }

// Clock::
    usage_types Clock::Use(Block * const who) {
        const QString time_str = (GetNote() == "real") ?
            tr("Time is %1.").arg(QTime::currentTime().toString()) :
            GetWorld()->TimeOfDayStr();
        if ( who != nullptr ) {
            const Active * const active = who->ActiveBlock();
            if ( active != nullptr ) {
                who->ReceiveSignal(time_str);
            }
        } else {
            SendSignalAround(time_str);
        }
        return USAGE_TYPE_NO;
    }

    QString Clock::FullName() const {
        switch ( Sub() ) {
        case IRON: return QObject::tr("Iron clock");
        case EXPLOSIVE: return QObject::tr("Bomb");
        default:
            fprintf(stderr, "%s: unlisted sub: %d\n", Q_FUNC_INFO, Sub());
            return "Strange clock";
        }
    }

    int  Clock::ShouldAct() const  { return FREQUENT_RARE; }
    void Clock::Damage(int, int) { Break(); }
    void Clock::Push(dirs, Block *) { Use(); }
    int  Clock::Kind() const { return CLOCK; }
    int  Clock::Weight() const { return Block::Weight()/10; }

    void Clock::DoRareAction() {
        if ( alarmTime == GetWorld()->TimeOfDay()
                || ActInner() == INNER_ACTION_MESSAGE )
        {
            Use();
        }
    }

    INNER_ACTIONS Clock::ActInner() {
        if ( timerTime > 0 )  {
            --timerTime;
            note->setNum(timerTime);
        } else if ( timerTime == 0 ) {
            Use();
            *note = QObject::tr("Timer fired. %1").
                arg(GetWorld()->TimeOfDayStr());
            timerTime = -1;
            return INNER_ACTION_MESSAGE;
        }
        return INNER_ACTION_NONE;
    }

    bool Clock::Inscribe(const QString str) {
        Block::Inscribe(str);
        char c;
        QTextStream txt_stream(note);
        txt_stream >> c;
        if ( 'a' == c ) {
            int alarm_hour;
            txt_stream >> alarm_hour;
            txt_stream >> alarmTime;
            alarmTime += alarm_hour*60;
            timerTime = -1;
        } else if ( 't'==c ) {
            txt_stream >> timerTime;
            alarmTime = -1;
        } else {
            alarmTime = timerTime = -1;
        }
        return true;
    }

    Clock::Clock(const int sub, const int id) :
            Active(sub, id, NONSTANDARD)
    {}

    Clock::Clock (QDataStream & str, const int sub, const int id) :
            Active(str, sub, id, NONSTANDARD)
    {
        if ( note ) {
            Inscribe(*note);
        }
    }

// Creator::
    int Creator::Kind() const { return CREATOR; }
    int Creator::Sub() const { return Block::Sub(); }
    QString Creator::FullName() const { return tr("Creative block"); }
    int Creator::DamageKind() const { return TIME; }
    int Creator::DamageLevel() const { return MAX_DURABILITY; }
    Inventory * Creator::HasInventory() { return this; }
    int Creator::ShouldAct() const { return FREQUENT_FIRST; }

    void Creator::ReceiveSignal(const QString str) {
        Active::ReceiveSignal(str);
    }

    void Creator::SaveAttributes(QDataStream & out) const {
        Active::SaveAttributes(out);
        Inventory::SaveAttributes(out);
    }

    Creator::Creator(const int sub, const int id) :
            Active(sub, id, NONSTANDARD),
            Inventory(INV_SIZE)
    {}
    Creator::Creator(QDataStream & str, const int sub, const int id) :
            Active(str, sub, id, NONSTANDARD),
            Inventory(str, INV_SIZE)
    {}

// Text::
    int Text::Kind() const { return TEXT; }
    QString Text::FullName() const {
        switch ( Sub() ) {
        case PAPER: return QObject::tr("Paper page");
        case GLASS: return QObject::tr("Screen");
        default:
            fprintf(stderr, "%s: sub ?: %d\n", Q_FUNC_INFO, Sub());
            return QObject::tr("Strange text");
        }
    }

    usage_types Text::Use(Block * const who) {
        if ( note ) {
            return USAGE_TYPE_READ;
        } else {
            who->ReceiveSignal(QObject::tr(
                "Nothing is written on this page."));
            return USAGE_TYPE_NO;
        }
    }

    bool Text::Inscribe(const QString str) {
        if ( '.' != str.at(0) && (note == nullptr || GLASS == Sub()) ) {
            Block::Inscribe(str);
            return true;
        } else {
            return false;
        }
    }

// Map::
    int Map::Kind() const { return MAP; }
    QString Map::FullName() const { return QObject::tr("Map"); }

    usage_types Map::Use(Block * const who) {
        if ( note == nullptr ) {
            if ( who ) {
                who->ReceiveSignal(QObject::tr(
                    "Set title to this map first."));
            }
            return USAGE_TYPE_NO;
        } else if ( who && who->ActiveBlock() ) {
            const Active * const active = who->ActiveBlock();
            QFile map_file(active->GetWorld()->
                WorldName() + "/texts/" + *note + ".txt");
            if ( not map_file.open(QIODevice::ReadWrite | QIODevice::Text) ) {
                return USAGE_TYPE_READ;
            }
            const Shred * const shred = active->GetShred();
            const long  lati = shred->Latitude();
            const long longi = shred->Longitude();
            static const int FILE_SIZE_CHARS = 31;
            if ( 0 == map_file.size() ) { // new map
                char header[FILE_SIZE_CHARS+1];
                memset(header, '-', FILE_SIZE_CHARS);
                header[0] = header[FILE_SIZE_CHARS/2] =
                    header[FILE_SIZE_CHARS-1] = '+';
                char body[FILE_SIZE_CHARS+1];
                memset(body, ' ', FILE_SIZE_CHARS);
                body[0] = body[FILE_SIZE_CHARS-1] = '|';
                body[FILE_SIZE_CHARS] = header[FILE_SIZE_CHARS] = '\n';
                map_file.write(header, FILE_SIZE_CHARS+1);
                for (int i=0; i<FILE_SIZE_CHARS-2; ++i) {
                    map_file.write(body,FILE_SIZE_CHARS+1);
                }
                map_file.write(header, FILE_SIZE_CHARS+1);

                map_file.seek( FILE_SIZE_CHARS/2 * (FILE_SIZE_CHARS+1) );
                map_file.putChar('+');
                map_file.seek(FILE_SIZE_CHARS/2*
                    (FILE_SIZE_CHARS+1)+FILE_SIZE_CHARS-1);
                map_file.putChar('+');

                longiStart = longi;
                latiStart  = lati;
            }
            if ( ( qAbs(lati-latiStart) > FILE_SIZE_CHARS/2 )
                    || ( qAbs(longi-longiStart) > FILE_SIZE_CHARS/2 ) )
            {
                return USAGE_TYPE_READ;
            }
            if ( savedChar ) {
                map_file.seek(savedShift);
                map_file.putChar(savedChar);
            }
            map_file.seek(savedShift=(FILE_SIZE_CHARS+1)*
                (longi-longiStart+FILE_SIZE_CHARS/2)+
                 lati -latiStart +FILE_SIZE_CHARS/2);
            map_file.putChar('@');
            savedChar=active->GetWorld()->TypeOfShred(longi, lati);
            map_file.seek((FILE_SIZE_CHARS+1)*FILE_SIZE_CHARS-1);
            map_file.write("\n @ = ");
            map_file.putChar(savedChar);
            map_file.putChar('\n');
        }
        return USAGE_TYPE_READ;
    } // usage_types Map::Use(Block * who)

    void Map::SaveAttributes(QDataStream & out) const {
        out << longiStart << latiStart << savedShift << savedChar;
    }

    Map::Map(const int sub, const int id) :
            Text(sub, id),
            longiStart(),
            latiStart(),
            savedShift(),
            savedChar(0)
    {}

    Map::Map(QDataStream & str, const int sub, const int id) :
            Text(str, sub, id)
    {
        str >> longiStart >> latiStart >> savedShift >> savedChar;
    }

// Bell::
    void Bell::Damage(int, int) { Break(); }
    int  Bell::Kind() const { return BELL; }
    QString Bell::FullName() const { return QObject::tr("Bell"); }

    usage_types Bell::Use(Block  * const) {
        SendSignalAround(DING);
        return USAGE_TYPE_NO;
    }

    void Bell::ReceiveSignal(const QString str) {
        if ( DING != str ) {
            SendSignalAround(DING);
        }
    }

// Predator::
    int Predator::DamageLevel() const { return 10; }
    int Predator::Kind() const { return PREDATOR; }
    QString Predator::FullName() const { return "Predator"; }

    int Predator::NutritionalValue(const int sub) const {
        return Attractive(sub) * SECONDS_IN_HOUR;
    }

    void Predator::DoFrequentAction() {
        if ( Gravitate(5, 1, 2, 0) ) {
            world->Move(X(), Y(), Z(), GetDir());
        }
    }

    void Predator::DoRareAction() {
        const Xyz coords[] = {
            Xyz(X()-1, Y(), Z()),
            Xyz(X()+1, Y(), Z()),
            Xyz(X(), Y()-1, Z()),
            Xyz(X(), Y()+1, Z()),
            Xyz(X(), Y(), Z()-1)
        };
        World * const world = GetWorld();
        for (const Xyz xyz : coords) {
            if ( not world->InBounds(xyz.X(), xyz.Y()) ) {
                continue;
            }
            Block * const block = world->GetBlock(xyz.X(), xyz.Y(), xyz.Z());
            if ( Attractive(block->Sub()) ) {
                block->ReceiveSignal(tr("Predator bites you!"));
                world->Damage(xyz.X(), xyz.Y(), xyz.Z(),
                    DamageLevel(), DamageKind());
                Eat(block->Sub());
            }
        }
        if ( SECONDS_IN_DAY/4 > Satiation() ) {
            EatGrass();
        }
        Animal::DoRareAction();
    }

    int Predator::Attractive(const int sub) const {
        switch ( sub ) {
        default:       return  0;
        case GREENERY: return  1;
        case A_MEAT:
        case H_MEAT:   return 10;
        }
    }
