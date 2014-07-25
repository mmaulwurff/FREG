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
#include "blocks/Animal.h"
#include "blocks/Inventory.h"
#include "Player.h"
#include "World.h"
#include "Shred.h"
#include "BlockManager.h"
#include "DeferredAction.h"

#ifdef QT_NO_DEBUG
const bool COMMANDS_ALWAYS_ON = false;
#else
const bool COMMANDS_ALWAYS_ON = true;
#endif

const subs PLAYER_SUB = ADAMANTINE;

long Player::GlobalX() const { return GetShred()->GlobalX(X()); }
long Player::GlobalY() const { return GetShred()->GlobalY(Y()); }
Shred * Player::GetShred() const { return world->GetShred(X(), Y()); }
World * Player::GetWorld() const { return world; }

dirs Player::GetDir() const { return dir; }
int  Player::UsingType() const { return usingType; }
void Player::StopUseAll() { usingType = usingSelfType = USAGE_TYPE_NO; }
int  Player::UsingSelfType() const { return usingSelfType; }
void Player::SetUsingTypeNo() { usingType = USAGE_TYPE_NO; }
bool Player::IfPlayerExists() const { return player != nullptr; }
int  Player::GetUsingInInventory() const { return usingInInventory; }
long Player::GetLongitude() const { return GetShred()->Longitude(); }
long Player::GetLatitude()  const { return GetShred()->Latitude();  }
bool Player::GetCreativeMode() const { return creativeMode; }

void Player::SetCreativeMode(const bool creative_on) {
    creativeMode = creative_on;
    player->disconnect();
    Active * const prev_player = player;
    SetPlayer(X(), Y(), Z());
    player->SetDir(prev_player->GetDir());
    Inventory * const inv = PlayerInventory();
    if ( inv != nullptr ) {
        inv->GetAll(prev_player->HasInventory());
    }
    if ( not creative_on ) {
        block_manager.DeleteBlock(prev_player);
    }
    emit Updated();
}

int Player::HP() const {
    return ( not IfPlayerExists() || GetCreativeMode() ) ?
        -1 : player ? player->GetDurability() : 0;
}

int Player::BreathPercent() const {
    if ( not IfPlayerExists() || GetCreativeMode() ) return -100;
    return player->Breath()*100/MAX_BREATH;
}

int Player::SatiationPercent() const {
    if ( not IfPlayerExists() || GetCreativeMode()
            || (player->Sub()!=H_MEAT && player->Sub()!=A_MEAT) )
    {
        return -100;
    }
    return player->Satiation()*100/SECONDS_IN_DAY;
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
        emit Notify(tr("Nothing here."));
        return 0;
    } else {
        return block;
    }
}

usage_types Player::Use(const int num) {
    const QMutexLocker locker(world->GetLock());
    Block * const block = ValidBlock(num);
    if ( block == nullptr ) return USAGE_TYPE_NO;
    if ( player->Eat(static_cast<subs>(block->Sub())) ) {
        PlayerInventory()->Pull(num);
        block_manager.DeleteBlock(block);
        emit Updated();
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
    default: Wield(num); break;
    }
    return result;
}

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

void Player::Wield(const int from) {
    if ( ValidBlock(from) ) {
        Inventory * const inv = PlayerInventory();
        const int start = (from >= inv->Start()) ? 0 : inv->Start();
        for (int i=start; i<inv->Size(); ++i) {
            PlayerInventory()->MoveInside(from, i, 1);
        }
        emit Updated();
    }
}

void Player::MoveInsideInventory(const int from, const int to, const int num) {
    const QMutexLocker locker(world->GetLock());
    if ( ValidBlock(from) ) {
        PlayerInventory()->MoveInside(from, to, num);
        emit Updated();
    }
}

void Player::Inscribe(const int num) {
    const QMutexLocker locker(world->GetLock());
    if ( ValidBlock(num) ) {
        QString str;
        emit GetString(str);
        PlayerInventory()->InscribeInv(num, str);
    }
}

void Player::Build(const int slot) {
    const QMutexLocker locker(world->GetLock());
    int x_targ, y_targ, z_targ;
    emit GetFocus(&x_targ, &y_targ, &z_targ);
    Block * const block = ValidBlock(slot);
    if ( block != nullptr && (AIR != world->GetBlock(X(), Y(), Z()-1)->Sub()
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

bool Player::ForbiddenAdminCommands() const {
    if ( GetCreativeMode() || COMMANDS_ALWAYS_ON ) {
        return false;
    } else {
        emit Notify(tr("You are not in Creative Mode."));
        return true;
    }
}

constexpr quint64 Player::UniqueIntFromString(const char * const chars) {
    return chars[0] == '\0' ?
        0 : (UniqueIntFromString(chars + 1) << 5) | (chars[0]-'a');
}

void Player::ProcessCommand(QString command) {
    QTextStream comm_stream(&command);
    QByteArray request;
    comm_stream >> request;
    const QMutexLocker locker(world->GetLock());
    switch ( UniqueIntFromString(request.constData()) ) {
    case UniqueIntFromString("give"):
    case UniqueIntFromString("get" ): {
        if ( ForbiddenAdminCommands() ) return;
        Inventory * const inv = PlayerInventory();
        if ( inv == nullptr ) return;
        QString kind, sub;
        comm_stream >> kind >> sub;
        const int kind_code = BlockManager::StringToKind(kind);
        if ( kind_code == LAST_KIND ) {
            emit Notify(tr("%1 command: invalid kind!").arg(QString(request)));
            return;
        } // else:
        const int sub_code = sub.isEmpty() ?
            static_cast<int>(STONE) : BlockManager::StringToSub(sub);
        if ( sub_code == LAST_SUB ) {
            emit Notify(tr("%1 command: invalid substance!")
                .arg(QString(request)));
            return;
        } // else:
        Block * const block = BlockManager::NewBlock(kind_code, sub_code);
        if ( inv->Get(block) ) {
            emit Updated();
        } else {
            block_manager.DeleteBlock(block);
        }
        } break;
    case UniqueIntFromString("move"): {
        int direction;
        comm_stream >> direction;
        Move(static_cast<dirs>(direction));
        } break;
    case UniqueIntFromString("time"):
        if ( ForbiddenAdminCommands() ) return;
        emit Notify(GetWorld()->TimeOfDayStr());
        break;
    case UniqueIntFromString("version"):
        emit Notify(tr("freg version: %1. Compiled on %2 at %3 with Qt %4.")
            .arg(VER)
            .arg(__DATE__)
            .arg(__TIME__)
            .arg(QT_VERSION_STR));
        emit Notify(tr("Current Qt version: %1. Build type: %2. Compiler: %3.")
            .arg(qVersion())
            .arg(DEBUG ? tr("debug") : tr("release"))
            .arg(COMPILER));
        break;
    case UniqueIntFromString("help"):
        comm_stream >> request;
        if ( request.isEmpty() ) {
            request = "help";
        }
        emit ShowFile( QString("help_%1/%2.txt")
            .arg(locale.left(2)).arg(QString(request)) );
        break;
    default:
        emit Notify(tr("Don't know such command: \"%1\".").arg(command));
        break;
    }
} // void Player::ProcessCommand(QString command)

bool Player::Visible(const int x_to, const int y_to, const int z_to) const {
    return ( (X()==x_to && Y()==y_to && Z()==z_to) || GetCreativeMode() ) ?
        true : (
            world->Enlightened(x_to, y_to, z_to) &&
            world->Visible(X(), Y(), Z(), x_to, y_to, z_to));
}

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
        ( player = BlockManager::NewBlock(CREATOR, DIFFERENT)->IsAnimal() )->
            SetXyz(X(), Y(), Z());
        GetShred()->Register(player);
    } else {
        World * const world = GetWorld();
        for ( ; z_self < HEIGHT-2; ++z_self ) {
            Block * const block = world->GetBlock(X(), Y(), z_self);
            if ( AIR == block->Sub() || nullptr != block->IsAnimal() ) {
                break;
            }
        }
        if ( (player=world->GetBlock(X(), Y(), Z())->IsAnimal()) == nullptr ) {
            world->Build( (player = BlockManager::
                    NewBlock(DWARF, PLAYER_SUB)->IsAnimal()),
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
}

Player::Player() {
    QSettings sett(QDir::currentPath() + '/' + world->WorldName()
        + "/settings.ini", QSettings::IniFormat);
    sett.beginGroup("player");
    homeLongi = sett.value("home_longitude",
        qlonglong(world->GetMap()->GetSpawnLongitude())).toLongLong();
    homeLati  = sett.value("home_latitude",
        qlonglong(world->GetMap()->GetSpawnLatitude ())).toLongLong();
    homeX  = sett.value("home_x", 0).toInt();
    homeY  = sett.value("home_y", 0).toInt();
    homeZ  = sett.value("home_z", HEIGHT/2).toInt();
    usingType     = sett.value("using_type",      USAGE_TYPE_NO).toInt();
    usingSelfType = sett.value("using_self_type", USAGE_TYPE_NO).toInt();
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
}

Player::~Player() {
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
    sett.setValue("using_type", usingType);
    sett.setValue("using_self_type", usingSelfType);
}
