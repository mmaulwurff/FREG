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
#include <QMutexLocker>
#include "blocks/Dwarf.h"
#include "Player.h"
#include "world.h"
#include "Shred.h"
#include "BlockManager.h"
#include "DeferredAction.h"

const bool COMMANDS_ALWAYS_ON = true;

long Player::GlobalX() const { return GetShred()->GlobalX(X()); }
long Player::GlobalY() const { return GetShred()->GlobalY(Y()); }

bool Player::IsRightActiveHand() const {
    return Dwarf::IN_RIGHT == GetActiveHand();
}
int Player::GetActiveHand() const {
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

Shred * Player::GetShred() const { return world->GetShred(X(), Y()); }
World * Player::GetWorld() const { return world; }

bool Player::GetCreativeMode() const { return creativeMode; }
void Player::SetCreativeMode(const bool creative_on) {
    creativeMode = creative_on;
    disconnect(player, 0, 0, 0);
    Active * const prev_player = player;
    SetPlayer(X(), Y(), Z());
    player->SetDir(prev_player->GetDir());
    Inventory * const inv = PlayerInventory();
    if ( inv != nullptr ) {
        inv->GetAll(prev_player->HasInventory());
    }
    if ( not creative_on ) {
        delete prev_player;
    }
    emit Updated();
}

int Player::UsingSelfType() const { return usingSelfType; }
int Player::UsingType() const { return usingType; }
void Player::SetUsingTypeNo() { usingType = USAGE_TYPE_NO; }
bool Player::IfPlayerExists() const { return player != nullptr; }

int Player::HP() const {
    return ( not IfPlayerExists() || GetCreativeMode() ) ?
        -1 : player ? player->GetDurability() : 0;
}

int Player::Breath() const {
    if ( not IfPlayerExists() || GetCreativeMode() ) return -1;
    Animal const * const animal = player->IsAnimal();
    return ( animal ? animal->Breath() : -1 );
}

int Player::BreathPercent() const {
    const int breath = Breath();
    return ( -1==breath ) ?
        100 : breath*100/MAX_BREATH;
}

int Player::Satiation() const {
    if ( not IfPlayerExists() || GetCreativeMode() ) return -1;
    Animal const * const animal = player->IsAnimal();
    return ( animal ? animal->Satiation() : -1 );
}

int Player::SatiationPercent() const {
    const int satiation = Satiation();
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

void Player::UpdateXYZ(int) {
    if ( player ) {
        SetXyz(player->X(), player->Y(), player->Z());
        emit Updated();
    }
}

void Player::Examine() const {
    const QMutexLocker locker(world->GetLock());

    int i, j, k;
    emit GetFocus(&i, &j, &k);
    const Block * const block = world->GetBlock(i, j, k);
    emit Notify( QString("*----- %1 -----*").arg(block->FullName()) );
    const int sub = block->Sub();
    if ( GetCreativeMode() || COMMANDS_ALWAYS_ON ) { // know more
        emit Notify(tr(
            "Light:%1, fire:%2, sun:%3. Transp:%4. Norm:%5. Dir:%6.").
            arg(world->Enlightened(i, j, k)).
            arg(world->FireLight(i, j, k)/16).
            arg(world->SunLight(i, j, k)).
            arg(block->Transparent()).
            arg(block==block_manager.NormalBlock(sub)).
            arg(block->GetDir()));
    }
    if ( IsLikeAir(sub) ) return;
    const QString str = block->GetNote();
    if ( not str.isEmpty() ) {
        emit Notify(tr("Inscription: ")+str);
    }
    emit Notify(tr("Durability: %2. Weight: %3. Id: %4.").
        arg(block->GetDurability()).
        arg(block->Weight()).
        arg(block->GetId()));
}

void Player::Jump() {
    if ( not IfPlayerExists() ) return;
    usingType = USAGE_TYPE_NO;
    if ( GetCreativeMode() ) {
        if ( (UP==dir && z_self<HEIGHT-2) || (DOWN==dir && z_self>1) ) {
            player->GetDeferredAction()->SetGhostMove();
        }
    } else {
        player->GetDeferredAction()->SetJump();
    }
}

void Player::Move(const dirs direction) {
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
    emit Updated();
}

void Player::Use() {
    const QMutexLocker locker(world->GetLock());
    int x, y, z;
    emit GetFocus(&x, &y, &z);
    const int us_type = world->GetBlock(x, y, z)->Use(player);
    usingType = ( us_type==usingType ) ? USAGE_TYPE_NO : us_type;
    emit Updated();
}

void Player::Inscribe() const {
    const QMutexLocker locker(world->GetLock());
    int x, y, z;
    emit GetFocus(&x, &y, &z);
    emit Notify(player ?
        (world->Inscribe(x, y, z) ?
            tr("Inscribed.") : tr("Cannot inscribe this.")) :
        tr("No player."));
}

Block * Player::ValidBlock(const int num) const {
    if ( not IfPlayerExists() ) {
        emit Notify("Player does not exist.");
        return 0;
    } // else:
    Inventory * const inv = player->HasInventory();
    if ( inv == nullptr ) {
        emit Notify("Player has no inventory.");
        return 0;
    } // else:
    if ( num >= inv->Size() ) {
        emit Notify("No such place.");
        return 0;
    } // else:
    Block * const block = inv->ShowBlock(num);
    if ( block == nullptr ) {
        emit Notify("Nothing here.");
        return 0;
    } else {
        return block;
    }
}

usage_types Player::Use(const int num) {
    const QMutexLocker locker(world->GetLock());
    return UseNoLock(num);
}

usage_types Player::UseNoLock(const int num) {
    Block * const block = ValidBlock(num);
    if ( block == nullptr ) return USAGE_TYPE_NO;
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
    switch ( result ) {
    case USAGE_TYPE_READ:
        usingInInventory = num;
        usingType = USAGE_TYPE_READ_IN_INVENTORY;
        emit Updated();
    break;
    case USAGE_TYPE_POUR: {
        int x_targ, y_targ, z_targ;
        emit GetFocus(&x_targ, &y_targ, &z_targ);
        player->GetDeferredAction()->SetPour(x_targ, y_targ, z_targ, num);
    } break;
    case USAGE_TYPE_SET_FIRE: {
        int x_targ, y_targ, z_targ;
        emit GetFocus(&x_targ, &y_targ, &z_targ);
        player->GetDeferredAction()->SetSetFire(x_targ, y_targ, z_targ);
    } break;
    default: break;
    }
    return result;
}

int Player::GetUsingInInventory() const { return usingInInventory; }

void Player::Throw(const int src, const int dest, const int num) {
    int x, y, z;
    emit GetFocus(&x, &y, &z);
    player->GetDeferredAction()->SetThrow(x, y, z, src, dest, num);
}

void Player::Obtain(const int src, const int dest, const int num) {
    const QMutexLocker locker(world->GetLock());
    int x, y, z;
    emit GetFocus(&x, &y, &z);
    world->Get(player, x, y, z, src, dest, num);
    emit Updated();
}

void Player::Wield(const int num) {
    const QMutexLocker locker(world->GetLock());
    if ( ValidBlock(num) ) {
        for (int i=0; i<=Dwarf::ON_LEGS; ++i) {
            InnerMove(num, i);
        }
        emit Updated();
    }
}

void Player::MoveInsideInventory(const int num_from, const int num_to,
        const int num)
{
    const QMutexLocker locker(world->GetLock());
    if ( ValidBlock(num_from) ) {
        InnerMove(num_from, num_to, num);
    }
}

void Player::InnerMove(const int num_from, const int num_to, const int num) {
    PlayerInventory()->MoveInside(num_from, num_to, num);
    emit Updated();
}

void Player::Inscribe(const int num) {
    const QMutexLocker locker(world->GetLock());
    if ( ValidBlock(num) ) {
        QString str;
        emit GetString(str);
        PlayerInventory()->InscribeInv(num, str);
    }
}

void Player::Eat(const int num) {
    const QMutexLocker locker(world->GetLock());
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

void Player::Build(const int slot) {
    const QMutexLocker locker(world->GetLock());
    int x_targ, y_targ, z_targ;
    emit GetFocus(&x_targ, &y_targ, &z_targ);
    Block * const block = ValidBlock(slot);
    if ( block && (AIR != world->GetBlock(X(), Y(), Z()-1)->Sub()
            || 0 == player->Weight()) )
    {
        player->GetDeferredAction()->
            SetBuild(x_targ, y_targ, z_targ, block, slot);
    }
}

void Player::Craft(const int num) {
    const QMutexLocker locker(world->GetLock());
    Inventory * const inv = PlayerInventory();
    if ( inv && inv->MiniCraft(num) ) {
        emit Updated();
    }
}

void Player::TakeOff(const int num) {
    const QMutexLocker locker(world->GetLock());
    if ( ValidBlock(num) ) {
        for (int i=PlayerInventory()->Start();
                i<PlayerInventory()->Size(); ++i)
        {
            InnerMove(num, i, PlayerInventory()->Number(num));
        }
    }
}

void Player::ProcessCommand(QString command) {
    if ( command.isEmpty() ) return;
    const QMutexLocker locker(world->GetLock());
    QTextStream comm_stream(&command);
    QString request;
    comm_stream >> request;
    if ( "give"==request || "get"==request ) {
        if ( not (GetCreativeMode() || COMMANDS_ALWAYS_ON) ) {
            emit Notify(tr("You are not in Creative Mode."));
            return;
        } // else:
        Inventory * const inv = PlayerInventory();
        if ( inv == nullptr ) return;
        QString kind, sub;
        comm_stream >> kind >> sub;
        const int kind_code = BlockManager::StringToKind(kind);
        if ( kind_code == LAST_KIND ) {
            emit Notify(tr("%1 command: invalid kind!").arg(request));
            return;
        } // else:
        const int sub_code = sub.isEmpty() ?
            static_cast<int>(STONE) : BlockManager::StringToSub(sub);
        if ( sub_code == LAST_SUB ) {
            emit Notify(tr("%1 command: invalid substance!").arg(request));
            return;
        } // else:
        emit Notify(QString("sub: %1").arg(sub_code));
        Block * const block = block_manager.NewBlock(kind_code, sub_code);
        if ( not inv->Get(block) ) {
            emit Notify(tr("No place for requested thing."));
            block_manager.DeleteBlock(block);
        } else {
            emit Updated();
        }
    } else if ( "move" == request ) {
        int direction;
        comm_stream >> direction;
        Move(static_cast<dirs>(direction));
    } else if ( "time" == request ) {
        emit Notify( (GetCreativeMode() || COMMANDS_ALWAYS_ON) ?
            GetWorld()->TimeOfDayStr() : tr("Not in Creative Mode.") );
    } else if ( "version" == request ) {
        emit Notify(tr("freg version: %1. Compiled on %2 at %3 with Qt %4.\n\
Current Qt version: %5. Build type: %6. Compiler: %7.").
            arg(VER).arg(__DATE__).arg(__TIME__).arg(QT_VERSION_STR).
            arg(qVersion()).arg(DEBUG ? tr("debug") : tr("release")).
            arg(COMPILER));
    } else if ( "help" == request ) {
        comm_stream >> request;
        emit ShowFile( QString("help_%1/%2.txt")
            .arg(locale.left(2)).arg(request) );
    } else {
        emit Notify(tr("Don't know such command: \"%1\".").arg(command));
    }
} // void Player::ProcessCommand(QString command)

bool Player::Visible(const int x_to, const int y_to, const int z_to)
const {
    return (X()==x_to && Y()==y_to && Z()==z_to) ?
        true :
        world->Visible(X(), Y(), Z(), x_to, y_to, z_to);
}

dirs Player::GetDir() const { return dir; }

void Player::SetDir(const dirs direction) {
    usingType = USAGE_TYPE_NO;
    if ( player ) {
        player->SetDir(direction);
    }
    dir = direction;
    emit Updated();
}

bool Player::Damage() const {
    int x, y, z;
    emit GetFocus(&x, &y, &z);
    if ( player && GetWorld()->InBounds(x, y, z) ) {
        player->GetDeferredAction()->SetDamage(x, y, z);
        return true;
    } else {
        return false;
    }
}

void Player::CheckOverstep(const int direction) {
    UpdateXYZ();
    static const int half_num_shreds = GetWorld()->NumShreds()/2;
    if ( DOWN!=direction && UP!=direction && ( // leaving central zone
            X() <  (half_num_shreds-1)*SHRED_WIDTH ||
            Y() <  (half_num_shreds-1)*SHRED_WIDTH ||
            X() >= (half_num_shreds+2)*SHRED_WIDTH ||
            Y() >= (half_num_shreds+2)*SHRED_WIDTH ) )
    {
        emit OverstepBorder(direction);
    }
    emit Moved(GlobalX(), GlobalY(), Z());
}

void Player::BlockDestroy() {
    if ( not cleaned ) {
        usingType = usingSelfType = USAGE_TYPE_NO;
        emit Destroyed();
        player = 0;
        world->ReloadAllShreds(homeLati, homeLongi, homeX,homeY,homeZ);
    }
}

void Player::SetPlayer(const int _x, const int _y, const int _z) {
    SetXyz(_x, _y, _z);
    if ( GetCreativeMode() ) {
        ( player = block_manager.NewBlock(CREATOR, DIFFERENT)->
            ActiveBlock() )->SetXyz(X(), Y(), Z());
        GetShred()->Register(player);
    } else {
        World * const world = GetWorld();
        for ( ; z_self < HEIGHT-1; ++z_self) {
            const Block * const target_block = world->GetBlock(X(), Y(), Z());
            if ( AIR==target_block->Sub() || DWARF==target_block->Kind() ) {
                break;
            }
        }
        Block * const target_block = world->GetBlock(X(), Y(), Z());
        if ( DWARF == target_block->Kind() ) {
            player = target_block->ActiveBlock();
        } else {
            world->Build( (player = block_manager.
                    NewBlock(DWARF, H_MEAT)->ActiveBlock()),
                X(), Y(), Z(), GetDir(), 0, true /*force build*/ );
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
    connect(player, SIGNAL(ReceivedText(const QString)),
        SIGNAL(Notify(const QString)),
        Qt::DirectConnection);
} // void Player::SetPlayer(int _x, int _y, int _z)

Player::Player() {
    QSettings sett(QDir::currentPath() + '/' + world->WorldName()
        + "/settings.ini", QSettings::IniFormat);
    sett.beginGroup("player");
    homeLongi = sett.value("home_longitude",
        qlonglong(world->GetSpawnLongi())).toLongLong();
    homeLati  = sett.value("home_latitude",
        qlonglong(world->GetSpawnLati())).toLongLong();
    homeX  = sett.value("home_x", 0).toInt();
    homeY  = sett.value("home_y", 0).toInt();
    homeZ  = sett.value("home_z", HEIGHT/2).toInt();
    SetXyz(sett.value("current_x", 0).toInt(),
           sett.value("current_y", 0).toInt(),
           sett.value("current_z", HEIGHT/2+1).toInt());
    creativeMode = sett.value("creative_mode", false).toBool();

    const int plus = world->NumShreds()/2*SHRED_WIDTH;
    homeX += plus;
    homeY += plus;
    SetPlayer(x_self+=plus, y_self+=plus, z_self);

    connect(world, SIGNAL(NeedPlayer(int, int, int)),
        SLOT(SetPlayer(int, int, int)),
        Qt::DirectConnection);
    connect(this, SIGNAL(OverstepBorder(int)),
        world, SLOT(SetReloadShreds(int)),
        Qt::DirectConnection);
    connect(world, SIGNAL(Moved(int)), SLOT(UpdateXYZ(int)),
        Qt::DirectConnection);
} // Player::Player()

void Player::CleanAll() {
    if ( cleaned ) return;
    cleaned = true;

    if ( GetCreativeMode() ) {
        block_manager.DeleteBlock(player);
    }

    QSettings sett(QDir::currentPath()+'/'+world->WorldName()+"/settings.ini",
        QSettings::IniFormat);
    sett.beginGroup("player");
    sett.setValue("home_longitude", qlonglong(homeLongi));
    sett.setValue("home_latitude", qlonglong(homeLati));
    const int min = world->NumShreds()/2*SHRED_WIDTH;
    sett.setValue("home_x", homeX-min);
    sett.setValue("home_y", homeY-min);
    sett.setValue("home_z", homeZ);
    sett.setValue("current_x", X()-min);
    sett.setValue("current_y", Y()-min);
    sett.setValue("current_z", Z());
    sett.setValue("creative_mode", GetCreativeMode());
}

Player::~Player() { CleanAll(); }
