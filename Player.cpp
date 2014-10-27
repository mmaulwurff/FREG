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
#include <QMutexLocker>
#include <QSettings>
#include "blocks/Inventory.h"
#include "Player.h"
#include "World.h"
#include "Shred.h"
#include "BlockManager.h"
#include "DeferredAction.h"
#include "Weather.h"
#include "worldmap.h"

const bool COMMANDS_ALWAYS_ON = DEBUG;

//const subs PLAYER_SUB = ADAMANTINE;
const subs PLAYER_SUB = H_MEAT;

int Player::X() const {
    return GetShred()->ShredX() << SHRED_WIDTH_SHIFT | Xyz::X();
}

int Player::Y() const {
    return GetShred()->ShredY() << SHRED_WIDTH_SHIFT | Xyz::Y();
}

long Player::GlobalX() const { return GetShred()->GlobalX(X()); }
long Player::GlobalY() const { return GetShred()->GlobalY(Y()); }
Shred * Player::GetShred() const { return player->GetShred(); }
World * Player::GetWorld() const { return world; }

int  Player::UsingType() const { return usingType; }
void Player::StopUseAll() { usingType = usingSelfType = USAGE_TYPE_NO; }
int  Player::UsingSelfType() const { return usingSelfType; }
void Player::SetUsingTypeNo() { usingType = USAGE_TYPE_NO; }
int  Player::GetUsingInInventory() const { return usingInInventory; }
long Player::GetLongitude() const { return GetShred()->Longitude(); }
long Player::GetLatitude()  const { return GetShred()->Latitude();  }
const Block * Player::GetBlock() const { return player; }

void Player::SetCreativeMode(const bool creative_on) {
    creativeMode = creative_on;
    player->disconnect(SIGNAL(destroyed()), nullptr);
    player->disconnect(SIGNAL(Moved(int)), nullptr);
    player->disconnect(SIGNAL(Updated()), nullptr);
    player->disconnect(SIGNAL(ReceivedText(QString)), nullptr);
    player->disconnect(SIGNAL(CauseTeleportation()), nullptr);
    SaveState();
    Animal * const prev_player = player;
    SetPlayer(X(), Y(), Z());
    player->SetDir(prev_player->GetDir());
    Inventory * const inv = PlayerInventory();
    if ( inv != nullptr ) {
        inv->GetAll(prev_player->HasInventory());
    }
    emit Updated();
}

int Player::BreathPercent() const { return player->Breath()*100/MAX_BREATH; }

int Player::SatiationPercent() const {
    return ( GetCreativeMode()
            || GROUP_MEAT != Block::GetSubGroup(player->Sub()) ) ?
        50 : player->Satiation()*100/SECONDS_IN_DAY;
}

Inventory * Player::PlayerInventory() const {
    Inventory * const inv = player->HasInventory();
    if ( inv != nullptr ) {
        return inv;
    } else {
        emit Notify(tr("You have no inventory."));
        return nullptr;
    }
}

void Player::UpdateXYZ() {
    SetXyz(player->X(), player->Y(), player->Z());
    emit Updated();
}

void Player::Examine() {
    QMutexLocker locker(world->GetLock());
    int x, y, z;
    emit GetFocus(&x, &y, &z);
    Examine(x, y, z);
}

void Player::Examine(const int x, const int y, const int z) {
    if ( not Visible(x, y, z) ) {
        emit Notify(tr("You can't see what is there."));
        return;
    }
    const Block * const block = world->GetBlock(x, y, z);
    emit Notify( tr("You see %1.").arg(block->FullName()) );
    if ( DEBUG ) {
        emit Notify(QString("Weight: %1. Id: %2.").
            arg(block->Weight()).
            arg(block->GetId()));
        emit Notify(QString("Kind: %1, substance: %2. LightRadius: %3").
            arg(block->Kind()).
            arg(block->Sub()).
            arg(block->LightRadius()));
        emit Notify(QString("Light: %1, fire: %2, sun: %3. Transp: %4.").
            arg(world->Enlightened(x, y, z)).
            arg(world->FireLight(x, y, z)/16).
            arg(world->SunLight(x, y, z)).
            arg(block->Transparent()));
        emit Notify(QString("Norm: %1. Dir: %2").
            arg(block==block_manager->Normal(block->Sub())).
            arg(block->GetDir()));
    }
    if ( Block::GetSubGroup(block->Sub()) == GROUP_AIR ) return;
    const QString str = block->GetNote();
    if ( not str.isEmpty() ) {
        emit Notify(tr("Inscription: ") + str);
    }
}

void Player::Jump() {
    usingType = USAGE_TYPE_NO;
    if ( GetCreativeMode() ) {
        player->GetDeferredAction()->SetGhostMove(GetDir());
    } else {
        player->GetDeferredAction()->SetJump();
    }
}

void Player::Move(const dirs direction) {
    if ( GetCreativeMode() ) {
        player->GetDeferredAction()->SetGhostMove(direction);
    } else {
        player->GetDeferredAction()->SetMove(direction);
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
    QMutexLocker locker(world->GetLock());
    int x, y, z;
    emit GetFocus(&x, &y, &z);
    Block * const block = world->GetBlock(x, y, z);
    const int us_type = block->Use(player);
    if ( us_type == USAGE_TYPE_NO ) {
        locker.unlock();
        Notify(tr("You cannot use %1.").arg(block->FullName()));
        return;
    }
    usingType = ( us_type==usingType ) ? USAGE_TYPE_NO : us_type;
    emit Updated();
}

void Player::Inscribe() const {
    const QMutexLocker locker(world->GetLock());
    int x, y, z;
    emit GetFocus(&x, &y, &z);
    const QString block_name = GetWorld()->GetBlock(x, y, z)->FullName();
    emit Notify(player ?
        (world->Inscribe(x, y, z) ?
            tr("Inscribed %1.").arg(block_name) :
            tr("Cannot inscribe %1.").arg(block_name)) :
        tr("No player."));
}

Block * Player::ValidBlock(const int num) const {
    Inventory * const inv = PlayerInventory();
    if ( not inv ) {
        return nullptr;
    } // else:
    if ( num >= inv->Size() ) {
        emit Notify("No such place.");
        return nullptr;
    } // else:
    Block * const block = inv->ShowBlock(num);
    if ( not block ) {
        emit Notify(tr("Nothing at slot '%1'.").arg(char(num + 'a')));
        return nullptr;
    } else {
        return block;
    }
}

usage_types Player::Use(const int num) {
    QMutexLocker locker(world->GetLock());
    Block * const block = ValidBlock(num);
    if ( block == nullptr ) return USAGE_TYPE_NO;
    if ( player->Eat(static_cast<subs>(block->Sub())) ) {
        PlayerInventory()->Pull(num);
        block_manager->DeleteBlock(block);
        emit Updated();
        return USAGE_TYPE_NO;
    } // else:
    const usage_types result = block->Use(player);
    switch ( result ) {
    case USAGE_TYPE_READ:
        usingInInventory = num;
        usingType = USAGE_TYPE_READ_IN_INVENTORY;
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
    case USAGE_TYPE_INNER: break;
    default:
        Notify(tr("You cannot use %1.").arg(block->FullName()));
        break;
    }
    emit Updated();
    return result;
}

void Player::Throw(const int src, const int dest, const int num) {
    if ( ValidBlock(src) == nullptr ) return;
    int x, y, z;
    emit GetFocus(&x, &y, &z);
    player->GetDeferredAction()->SetThrow(x, y, z, src, dest, num);
}

bool Player::Obtain(const int src, const int dest, const int num) {
    const QMutexLocker locker(world->GetLock());
    int x, y, z;
    emit GetFocus(&x, &y, &z);
    bool is_success = world->Get(player, x, y, z, src, dest, num);
    Inventory * const from = world->GetBlock(x, y, z)->HasInventory();
    if ( from != nullptr && from->IsEmpty() ) {
        usingType = USAGE_TYPE_NO;
    }
    emit Updated();
    return is_success;
}

void Player::Wield(const int from) {
    const QMutexLocker locker(world->GetLock());
    Block * const block = ValidBlock(from);
    if ( block != nullptr ) {
        Inventory * const inv = PlayerInventory();
        inv->Pull(from);
        inv->Get(block, (from >= inv->Start()) ? 0 : inv->Start());
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
        player->GetDeferredAction()->SetBuild(x_targ, y_targ, z_targ, slot);
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

void Player::ProcessCommand(QString command) {
    QTextStream comm_stream(&command);
    QByteArray request;
    comm_stream >> request;
    const QMutexLocker locker(world->GetLock());
    switch ( UniqueIntFromString(request.constData()) ) {
    case UniqueIntFromString(""): break;
    case UniqueIntFromString("weather"):
        emit Notify(Weather::GetWeatherString(GetShred()->GetWeather()));
        break;
    case UniqueIntFromString("give"):
    case UniqueIntFromString("get" ): {
        if ( ForbiddenAdminCommands() ) return;
        Inventory * const inv = PlayerInventory();
        if ( inv == nullptr ) return;
        QByteArray kind, sub;
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
            block_manager->DeleteBlock(block);
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
    case UniqueIntFromString("warranty"):
        comm_stream <<  "warranty";
        // no break;
    case UniqueIntFromString("help"):
        comm_stream >> request;
        if ( request.isEmpty() ) {
            request = "help";
        }
        emit ShowFile( QString(":/help_%1/%2.md")
            .arg(locale.left(2)).arg(QString(request)) );
        break;
    default:
        emit Notify(tr("Don't know such command: \"%1\".").arg(command));
        break;
    }
} // void Player::ProcessCommand(QString command)

bool Player::Visible(const int x_to, const int y_to, const int z_to) const {
    return ( GetCreativeMode()
        || ( world->Enlightened(x_to, y_to, z_to)
            && world->Visible(X(), Y(), Z(), x_to, y_to, z_to)) );
}

void Player::SetDir(const dirs direction) {
    usingType = USAGE_TYPE_NO;
    player->SetDir(direction);
    emit Updated();
}

bool Player::Damage() const {
    int x, y, z;
    emit GetFocus(&x, &y, &z);
    if ( GetWorld()->InBounds(x, y, z) ) {
        player->GetDeferredAction()->SetDamage(x, y, z);
        return true;
    } else {
        return false;
    }
}

void Player::CheckOverstep(const int direction) {
    SetXyz(Shred::CoordInShred(player->X()), Shred::CoordInShred(player->Y()),
        player->Z());
    if ( direction > DOWN
            && not GetWorld()->ShredInCentralZone(
                GetShred()->Longitude(), GetShred()->Latitude()) )
    {
        emit OverstepBorder(direction);
    }
    emit Moved(GlobalX(), GlobalY(), Z());
}

void Player::BlockDestroy() {
    if ( not cleaned ) {
        usingType = usingSelfType = USAGE_TYPE_NO;
        emit Notify(tr("^ You die. ^"));
        emit Destroyed();
        player = nullptr;
        const int plus = world->NumShreds() / 2 * SHRED_WIDTH;
        world->ReloadAllShreds(world->WorldName(),
            homeLati, homeLongi, homeX+plus, homeY+plus, homeZ);
        world->ActivateFullReload();
    }
}

void Player::SetPlayer(int _x, int _y, int _z) {
    LoadState();
    if ( _z == -1 ) {
        const int plus = world->NumShreds() / 2;
        _x = Xyz::X() + (latitude  - world->Latitude()  + plus) * SHRED_WIDTH;
        _y = Xyz::Y() + (longitude - world->Longitude() + plus) * SHRED_WIDTH;
    } else {
        SetXyz(Shred::CoordInShred(_x), Shred::CoordInShred(_y), _z);
    }
    if ( GetCreativeMode() ) {
        creator->SetXyz(Xyz::X(), Xyz::Y(), Xyz::Z());
        GetWorld()->GetShred(_x, _y)->Register((player = creator));
    } else {
        World * const world = GetWorld();
        Q_ASSERT(z_self <= HEIGHT-2);
        Block * candidate = world->GetBlock(_x, _y, z_self);
        for ( ; z_self < HEIGHT-2; ++z_self ) {
            candidate = world->GetBlock(_x, _y, z_self);
            if ( AIR == candidate->Sub()
                    || ((player==creator || player==nullptr)
                        && candidate->IsAnimal()) )
            {
                break;
            }
        }
        if ( candidate->IsAnimal() ) {
            player = candidate->IsAnimal();
        } else {
            if ( player == nullptr || player == creator ) {
                player = BlockManager::NewBlock(DWARF, PLAYER_SUB)->IsAnimal();
            }
            world->Build(player, _x, _y, Z(), GetDir(), nullptr, true);
        }
    }
    connect(player, SIGNAL(destroyed()), SLOT(BlockDestroy()),
        Qt::DirectConnection);
    connect(player, SIGNAL(Moved(int)), SLOT(CheckOverstep(int)),
        Qt::DirectConnection);
    connect(player, SIGNAL(Updated()), SIGNAL(Updated()),
        Qt::DirectConnection);
    connect(player, SIGNAL(ReceivedText(QString)), SIGNAL(Notify(QString)),
        Qt::DirectConnection);
    connect(player, SIGNAL(CauseTeleportation()),
        world, SLOT(ActivateFullReload()), Qt::DirectConnection);
} // Player::SetPlayer(int _x, _y, _z)

void Player::Disconnect() {
    if ( player == nullptr ) { // dead player
        player = BlockManager::NewBlock(DWARF, PLAYER_SUB)->IsAnimal();
    } else {
        SaveState();
        GetShred()->SetBlockNoCheck(block_manager->Normal(AIR),
            x_self, y_self, z_self);
        player->Unregister();
        player->disconnect();
    }
}

Player::Player() :
        longitude(), latitude(),
        homeLongi(), homeLati(),
        homeX(), homeY(), homeZ(),
        player(nullptr),
        creator(BlockManager::NewBlock(DWARF, DIFFERENT)->IsAnimal()),
        usingType(), usingSelfType(),
        usingInInventory(),
        creativeMode()
{
    SetPlayer(0, 0, -1);
    connect(world, SIGNAL(NeedPlayer(int, int, int)),
        SLOT(SetPlayer(int, int, int)),
        Qt::DirectConnection);
    connect(this, SIGNAL(OverstepBorder(int)),
        world, SLOT(SetReloadShreds(int)),
        Qt::DirectConnection);
    connect(world, SIGNAL(StartReloadAll()), SLOT(Disconnect()),
        Qt::DirectConnection);
}

Player::~Player() {
    SaveState();
    delete creator;
}

void Player::SaveState() const {
    QSettings settings(home_path + world->WorldName() + "/player_state.ini",
        QSettings::IniFormat);
    settings.setValue("home_longitude", qlonglong(homeLongi));
    settings.setValue("home_latitude",  qlonglong(homeLati));
    settings.setValue("home_x", homeX);
    settings.setValue("home_y", homeY);
    settings.setValue("home_z", homeZ);
    settings.setValue("current_longitude", qlonglong(GetShred()->Longitude()));
    settings.setValue("current_latitude",  qlonglong(GetShred()->Latitude()));
    settings.setValue("current_x", Xyz::X());
    settings.setValue("current_y", Xyz::Y());
    settings.setValue("current_z", Xyz::Z());
    settings.setValue("using_type",      usingType);
    settings.setValue("using_self_type", usingSelfType);
    settings.setValue("creative_mode", GetCreativeMode());
}

void Player::LoadState() {
    const QSettings settings(home_path + world->WorldName() +
        "/player_state.ini", QSettings::IniFormat);
    homeLongi = settings.value("home_longitude",
        qlonglong(world->GetMap()->GetSpawnLongitude())).toLongLong();
    homeLati  = settings.value("home_latitude",
        qlonglong(world->GetMap()->GetSpawnLatitude ())).toLongLong();
    homeX = settings.value("home_x", 0).toInt();
    homeY = settings.value("home_y", 0).toInt();
    homeZ = settings.value("home_z", HEIGHT/2).toInt();
    usingType     = settings.value("using_type",      USAGE_TYPE_NO).toInt();
    usingSelfType = settings.value("using_self_type", USAGE_TYPE_NO).toInt();
    creativeMode  = settings.value("creative_mode", false).toBool();
    SetXyz(settings.value("current_x", 0).toInt(),
           settings.value("current_y", 0).toInt(),
           settings.value("current_z", HEIGHT/2+1).toInt());
    longitude = settings.value("current_longitude", homeLongi).toLongLong();
    latitude  = settings.value("current_latitude",  homeLati ).toLongLong();
}
