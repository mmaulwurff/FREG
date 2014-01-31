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
#include <QSettings>
#include <QDir>
#include <QReadLocker>
#include "Dwarf.h"
#include "Player.h"
#include "world.h"
#include "Shred.h"
#include "BlockManager.h"
#include "DeferredAction.h"

const bool COMMANDS_ALWAYS_ON = true;

short Player::X() const { return x; }
short Player::Y() const { return y; }
short Player::Z() const { return z; }
long Player::GlobalX() const { return GetShred()->GlobalX(x); }
long Player::GlobalY() const { return GetShred()->GlobalY(y); }

bool Player::IsRightActiveHand() const {
    return Dwarf::IN_RIGHT == GetActiveHand();
}
ushort Player::GetActiveHand() const {
    Dwarf * const dwarf = dynamic_cast<Dwarf *>(player);
    return ( dwarf ) ?
        dwarf->GetActiveHand() : 0;
}
void Player::SetActiveHand(const bool right) {
    Dwarf * const dwarf = dynamic_cast<Dwarf *>(player);
    if ( dwarf ) {
        dwarf->SetActiveHand(right);
    }
}

Shred * Player::GetShred() const { return world->GetShred(x, y); }
World * Player::GetWorld() const { return world; }

bool Player::GetCreativeMode() const { return creativeMode; }
void Player::SetCreativeMode(const bool turn) {
    creativeMode = turn;
    disconnect(player, 0, 0, 0);
    Active * const prev_player = player;
    SetPlayer(x, y, z);
    player->SetDir(prev_player->GetDir());
    Inventory * const inv = PlayerInventory();
    if ( inv ) {
        inv->GetAll(prev_player->HasInventory());
    }
    if ( !GetCreativeMode() ) {
        prev_player->SetToDelete();
    }
    emit Updated();
}

int Player::UsingSelfType() const { return usingSelfType; }
int Player::UsingType() const { return usingType; }
void Player::SetUsingTypeNo() { usingType = USAGE_TYPE_NO; }

bool Player::IfPlayerExists() const { return !!player; }

short Player::HP() const {
    return ( !player || GetCreativeMode() ) ?
        -1 : player ? player->GetDurability() : 0;
}

short Player::Breath() const {
    if ( !player || GetCreativeMode() ) {
        return -1;
    }
    Animal const * const animal = player->IsAnimal();
    return ( animal ? animal->Breath() : -1 );
}
ushort Player::BreathPercent() const {
    const short breath = Breath();
    return ( -1==breath ) ?
        100 : breath*100/MAX_BREATH;
}

short Player::Satiation() const {
    if ( !player || GetCreativeMode() ) {
        return -1;
    }
    Animal const * const animal = player->IsAnimal();
    return ( animal ? animal->Satiation() : -1 );
}
ushort Player::SatiationPercent() const {
    const short satiation = Satiation();
    return ( -1==satiation ) ?
        50 : satiation*100/SECONDS_IN_DAY;
}

Inventory * Player::PlayerInventory() const {
    Inventory * inv;
    if ( player && (inv=player->HasInventory()) ) {
        return inv;
    } else {
        emit Notify(tr("You have no inventory."));
        return 0;
    }
}

long Player::GetLongitude() const { return GetShred()->Longitude(); }
long Player::GetLatitude()  const { return GetShred()->Latitude();  }

void Player::UpdateXYZ(const int) {
    if ( player ) {
        x = player->X();
        y = player->Y();
        z = player->Z();
        emit Updated();
    }
}

void Player::Focus(ushort & i_target, ushort & j_target, ushort & k_target)
const {
    world->Focus(x, y, z, i_target, j_target, k_target, GetDir());
}

void Player::Examine(const short i, const short j, const short k) const {
    QReadLocker locker(world->GetLock());

    const Block * const block = world->GetBlock(i, j, k);
    emit Notify( QString("*----- %1 -----*").arg(block->FullName()) );
    const int sub = block->Sub();
    if ( GetCreativeMode() || COMMANDS_ALWAYS_ON ) { //know more
        emit Notify(tr(
        "Light:%1, fire:%2, sun:%3. Transp:%4. Norm:%5. Dir:%6.").
            arg(world->Enlightened(i, j, k)).
            arg(world->FireLight(i, j, k)/16).
            arg(world->SunLight(i, j, k)).
            arg(block->Transparent()).
            arg(block==block_manager.NormalBlock(sub)).
            arg(block->GetDir()));
    }
    if ( AIR==sub || SKY==sub || SUN_MOON==sub ) {
        return;
    } // else:
    QString str;
    if ( "" != (str=block->GetNote()) ) {
        emit Notify(tr("Inscription: ")+str);
    }
    emit Notify(tr("Temperature: %1. Durability: %2. Weight: %3. Id: %4.").
        arg(world->Temperature(i, j, k)).
        arg(block->GetDurability()).
        arg(block->Weight()).
        arg(block->GetId()));
}

void Player::Jump() {
    if ( !player ) {
        return;
    }
    usingType = USAGE_TYPE_NO;
    if ( GetCreativeMode() ) {
        if ( (UP==dir && z<HEIGHT-2) || (DOWN==dir && z>1) ) {
            player->GetDeferredAction()->SetGhostMove();
        }
    } else {
        player->GetDeferredAction()->SetJump();
    }
}

void Player::Move(const int direction) {
    if ( player ) {
        if ( GetCreativeMode() ) {
            player->GetDeferredAction()->SetGhostMove(direction);
        } else {
            player->GetDeferredAction()->SetMove(direction);
        }
    }
}

void Player::StopUseAll() { usingType = usingSelfType = USAGE_TYPE_NO; }

void Player::Backpack() {
    if ( PlayerInventory() ) {
        usingSelfType = ( USAGE_TYPE_OPEN==usingSelfType ) ?
            USAGE_TYPE_NO : USAGE_TYPE_OPEN;
    }
}

void Player::Use(const short x, const short y, const short z) {
    QWriteLocker locker(world->GetLock());
    const int us_type = world->GetBlock(x, y, z)->Use(player);
    usingType = ( us_type==usingType ) ? USAGE_TYPE_NO : us_type;
    emit Updated();
}

void Player::Inscribe(const short x, const short y, const short z) const {
    QWriteLocker locker(world->GetLock());
    emit Notify(player ?
        (world->Inscribe(x, y, z) ?
            tr("Inscribed.") : tr("Cannot inscribe this.")) :
        tr("No player."));
}

Block * Player::ValidBlock(const ushort num) const {
    if ( !player ) {
        emit Notify("Player does not exist.");
        return 0;
    } // else:
    Inventory * const inv = player->HasInventory();
    if ( !inv ) {
        emit Notify("Player has no inventory.");
        return 0;
    } // else:
    if ( num>=inv->Size() ) {
        emit Notify("No such place.");
        return 0;
    } // else:
    Block * const block = inv->ShowBlock(num);
    if ( !block ) {
        emit Notify("Nothing here.");
        return 0;
    } else {
        return block;
    }
}

usage_types Player::Use(const ushort num) {
    QWriteLocker locker(world->GetLock());
    return UseNoLock(num);
}

usage_types Player::UseNoLock(const ushort num) {
    Block * const block = ValidBlock(num);
    if ( !block ) {
        return USAGE_TYPE_NO;
    } // else:
    Animal * const animal = player->IsAnimal();
    if ( animal && animal->NutritionalValue(block->Sub()) ) {
        animal->Eat(block->Sub());
        Inventory * const inv = PlayerInventory();
        Block * const eaten = inv->ShowBlock(num);
        inv->Pull(num);
        block_manager.DeleteBlock(eaten);
        return USAGE_TYPE_NO;
    } // else:
    const usage_types result = block->Use(player);
    if ( result == USAGE_TYPE_READ ) {
        usingInInventory = num;
        usingType = USAGE_TYPE_READ_IN_INVENTORY;
        emit Updated();
    }
    return result;
}

ushort Player::GetUsingInInventory() const { return usingInInventory; }

void Player::Throw(const short x, const short y, const short z,
        const ushort src, const ushort dest, const ushort num)
{
    player->GetDeferredAction()->SetThrow(x, y, z, src, dest, num);
}

void Player::Obtain(const short x, const short y, const short z,
        const ushort src, const ushort dest, const ushort num)
{
    QWriteLocker locker(world->GetLock());
    world->Get(player, x, y, z, src, dest, num);
    emit Updated();
}

void Player::Wield(const ushort num) {
    QWriteLocker locker(world->GetLock());
    if ( ValidBlock(num) ) {
        for (ushort i=0; i<=Dwarf::ON_LEGS; ++i) {
            InnerMove(num, i);
        }
        emit Updated();
    }
}

void Player::MoveInsideInventory(const ushort num_from, const ushort num_to,
        const ushort num)
{
    QWriteLocker locker(world->GetLock());
    if ( ValidBlock(num_from) ) {
        InnerMove(num_from, num_to, num);
    }
}

void Player::InnerMove(const ushort num_from, const ushort num_to,
        const ushort num)
{
    PlayerInventory()->MoveInside(num_from, num_to, num);
    emit Updated();
}

void Player::Inscribe(const ushort num) {
    QWriteLocker locker(world->GetLock());
    if ( ValidBlock(num) ) {
        QString str;
        emit GetString(str);
        PlayerInventory()->InscribeInv(num, str);
    }
}

void Player::Eat(const ushort num) {
    QWriteLocker locker(world->GetLock());
    Block * const food = ValidBlock(num);
    if ( food ) {
        Animal * const animal = player->IsAnimal();
        if ( animal ) {
            if ( animal->Eat(food->Sub()) ) {
                PlayerInventory()->Pull(num);
                block_manager.DeleteBlock(food);
                emit Updated();
            }
        } else {
            emit Notify(tr("You cannot eat."));
        }
    }
}

void Player::Pour(const short x_targ, const short y_targ, const short z_targ,
        const ushort slot)
{
    player->GetDeferredAction()->
        SetPour(x_targ, y_targ, z_targ, slot);
}

void Player::Build(const short x_target, const short y_target,
        const short z_target, const ushort slot)
{
    QWriteLocker locker(world->GetLock());
    Block * const block = ValidBlock(slot);
    if ( block && (AIR!=world->GetBlock(x, y, z-1)->Sub() ||
            0==player->Weight()) )
    {
        player->GetDeferredAction()->
            SetBuild(x_target, y_target, z_target, block, slot);
    }
}

void Player::Craft(const ushort num) {
    QWriteLocker locker(world->GetLock());
    Inventory * const inv = PlayerInventory();
    if ( inv && inv->MiniCraft(num) ) {
        emit Updated();
    }
}

void Player::TakeOff(const ushort num) {
    QWriteLocker locker(world->GetLock());
    if ( ValidBlock(num) ) {
        for (ushort i=PlayerInventory()->Start();
                i<PlayerInventory()->Size(); ++i)
        {
            InnerMove(num, i, PlayerInventory()->Number(num));
        }
    }
}

void Player::ProcessCommand(QString & command) {
    if ( 0 == command.length() ) {
        return;
    } // else:
    QWriteLocker locker(world->GetLock());
    QTextStream comm_stream(&command);
    QString request;
    comm_stream >> request;
    if ( "give"==request || "get"==request ) {
        if ( !(GetCreativeMode() || COMMANDS_ALWAYS_ON) ) {
            emit Notify(tr("You are not in Creative Mode."));
            return;
        } // else:
        Inventory * const inv = PlayerInventory();
        if ( !inv ) {
            return;
        } // else:
        int kind, sub, num;
        comm_stream >> kind >> sub >> num;
        for (num = qBound(1, num, 9); num; --num) {
            Block* const block = block_manager.NewBlock(kind, sub);
            if ( !inv->Get(block) ) {
                emit Notify( (num==1) ?
                        tr("No place for one thing.") :
                        tr("No place for %n things.", "", num) );
                block_manager.DeleteBlock(block);
                break;
            }
        }
        emit Updated();
    } else if ( "move" == request ) {
        int direction;
        comm_stream >> direction;
        Move(direction);
    } else if ( "what" == request ) {
        ushort x_what, y_what, z_what;
        comm_stream >> x_what >> y_what >> z_what;
        if ( !world->InBounds(x_what, y_what, z_what) ) {
            emit Notify(tr("Such block is out of loaded world."));
        } else if ( GetCreativeMode() || COMMANDS_ALWAYS_ON
                || qAbs(x-x_what) > 1
                || qAbs(y-y_what) > 1
                || qAbs(z-z_what) > 1 )
        {
            Examine(x_what, y_what, z_what);
        } else {
            emit Notify(tr("Too far."));
        }
    } else if ( "kindtostring"==request || "k2str"==request ) {
        int kind;
        comm_stream >> kind;
        emit Notify(tr("Kind %1 is %2.").
            arg(kind).arg(block_manager.KindToString(kind)));
    } else if ( "stringtokind"==request || "str2k"==request ) {
        comm_stream >> request;
        const int kind = block_manager.StringToKind(request);
        emit Notify( ( kind == LAST_KIND ) ?
            tr("\"%1\" is unknown kind.").arg(request) :
            tr("Code of kind %1 is %2.").arg(request).arg(kind) );
    } else if ( "subtostring"==request || "s2str"==request ) {
        int sub;
        comm_stream >> sub;
        emit Notify(tr("Sub %1 is %2.").
            arg(sub).arg(BlockManager::SubToString(sub)));
    } else if ( "stringtosub"==request || "str2s"==request ) {
        comm_stream >> request;
        const int sub = BlockManager::StringToSub(request);
        emit Notify( ( sub == LAST_SUB ) ?
            tr("\"%1\" is unknown substance.").arg(request) :
            tr("Code of substance %1 is %2.").
                arg(request).arg(sub) );
    } else if ( "time" == request ) {
        emit Notify( (GetCreativeMode() || COMMANDS_ALWAYS_ON) ?
            GetWorld()->TimeOfDayStr() :
            tr("Not in Creative Mode.") );
    } else if ( "version" == request ) {
        emit Notify(tr("freg version: %1. Compiled on %2 at %3.").
            arg(VER).arg(__DATE__).arg(__TIME__));
    } else if ( "help" == request ) {
        comm_stream >> request;
        emit ShowFile( QString("help_%1/%2.txt")
            .arg(QLocale::system().name().left(2)).arg(request));
    } else {
        emit Notify(tr("Don't know such command: \"%1\".").arg(command));
    }
} // void Player::ProcessCommand(QString & command)

bool Player::Visible(const ushort x_to, const ushort y_to, const ushort z_to)
const {
    return world->Visible(x, y, z, x_to, y_to, z_to);
}

int  Player::GetDir() const { return dir; }
void Player::SetDir(const int direction) {
    usingType = USAGE_TYPE_NO;
    if ( player ) {
        player->SetDir(direction);
    }
    dir = direction;
    emit Updated();
}

bool Player::Damage(const short x, const short y, const short z) const {
    if ( player && GetWorld()->InBounds(x, y, z) ) {
        player->GetDeferredAction()->SetDamage(x, y, z);
        return true;
    } else {
        return false;
    }
}

void Player::CheckOverstep(const int direction) {
    UpdateXYZ();
    static const ushort half_num_shreds = GetWorld()->NumShreds()/2;
    if ( DOWN!=direction && UP!=direction && ( // leaving central zone
            x <  (half_num_shreds-1)*SHRED_WIDTH ||
            y <  (half_num_shreds-1)*SHRED_WIDTH ||
            x >= (half_num_shreds+2)*SHRED_WIDTH ||
            y >= (half_num_shreds+2)*SHRED_WIDTH ) )
    {
        emit OverstepBorder(direction);
    }
    emit Moved(GlobalX(), GlobalY(), z);
}

void Player::BlockDestroy() {
    if ( !cleaned ) {
        usingType = usingSelfType = USAGE_TYPE_NO;
        emit Destroyed();
        player = 0;
        world->ReloadAllShreds(homeLati, homeLongi, homeX,homeY,homeZ);
    }
}

void Player::SetPlayer(const ushort _x, const ushort _y, const ushort _z) {
    x = _x;
    y = _y;
    z = _z;
    if ( GetCreativeMode() ) {
        ( player = block_manager.NewBlock(CREATOR, DIFFERENT)->
            ActiveBlock() )->SetXYZ(x, y, z);
        GetShred()->Register(player);
    } else {
        World * const world = GetWorld();
        for ( ; z < HEIGHT-1; ++z) {
            Block * const target_block = world->GetBlock(x, y, z);
            if ( AIR==target_block->Sub() ||
                    DWARF==target_block->Kind() )
            {
                break;
            }
        }
        Block * const target_block = world->GetBlock(x, y, z);
        if ( DWARF == target_block->Kind() ) {
            player = target_block->ActiveBlock();
        } else {
            world->Build( (player = block_manager.
                    NewBlock(DWARF,H_MEAT)->ActiveBlock()),
                x, y, z, GetDir(), 0, true /*force build*/ );
        }
    }
    player->SetDeferredAction(new DeferredAction(player));
    SetDir(player->GetDir());

    connect(player, SIGNAL(Destroyed()), SLOT(BlockDestroy()),
        Qt::DirectConnection);
    connect(player, SIGNAL(Moved(int)), SLOT(CheckOverstep(int)),
        Qt::DirectConnection);
    connect(player, SIGNAL(Updated()), SIGNAL(Updated()),
        Qt::DirectConnection);
    connect(player, SIGNAL(ReceivedText(const QString &)),
        SIGNAL(Notify(const QString &)),
        Qt::DirectConnection);
} // void Player::SetPlayer(ushort _x, ushort _y, ushort _z)

Player::Player() :
        dir(NORTH),
        usingType(USAGE_TYPE_NO),
        usingSelfType(USAGE_TYPE_NO),
        cleaned(false)
{
    puts(qPrintable(tr("Loading player...")));
    QSettings sett(QDir::currentPath() + '/' + world->WorldName() +
            "/settings.ini",
        QSettings::IniFormat);
    sett.beginGroup("player");
    homeLongi = sett.value("home_longitude",
        qlonglong(world->GetSpawnLongi())).toLongLong();
    homeLati  = sett.value("home_latitude",
        qlonglong(world->GetSpawnLati())).toLongLong();
    homeX = sett.value("home_x", 0).toInt();
    homeY = sett.value("home_y", 0).toInt();
    homeZ = sett.value("home_z", HEIGHT/2).toInt();
    x     = sett.value("current_x", 0).toInt();
    y     = sett.value("current_y", 0).toInt();
    z     = sett.value("current_z", HEIGHT/2+1).toInt();
    creativeMode = sett.value("creative_mode", false).toBool();

    const ushort plus = world->NumShreds()/2*SHRED_WIDTH;
    homeX += plus;
    homeY += plus;
    SetPlayer(x+=plus, y+=plus, z);

    connect(world, SIGNAL(NeedPlayer(ushort, ushort, ushort)),
        SLOT(SetPlayer(ushort, ushort, ushort)),
        Qt::DirectConnection);
    connect(this, SIGNAL(OverstepBorder(int)),
        world, SLOT(SetReloadShreds(int)),
        Qt::DirectConnection);
    connect(world, SIGNAL(Moved(int)), SLOT(UpdateXYZ(int)),
        Qt::DirectConnection);
} // Player::Player()

void Player::CleanAll() {
    if ( cleaned ) {
        return;
    }
    cleaned = true;

    if ( GetCreativeMode() ) {
        block_manager.DeleteBlock(player);
    }

    QSettings sett(QDir::currentPath()+'/'+
            world->WorldName()+"/settings.ini",
        QSettings::IniFormat);
    sett.beginGroup("player");
    sett.setValue("home_longitude", qlonglong(homeLongi));
    sett.setValue("home_latitude", qlonglong(homeLati));
    const ushort min = world->NumShreds()/2*SHRED_WIDTH;
    sett.setValue("home_x", homeX-min);
    sett.setValue("home_y", homeY-min);
    sett.setValue("home_z", homeZ);
    sett.setValue("current_x", x-min);
    sett.setValue("current_y", y-min);
    sett.setValue("current_z", z);
    sett.setValue("creative_mode", GetCreativeMode());
}

Player::~Player() { CleanAll(); }
