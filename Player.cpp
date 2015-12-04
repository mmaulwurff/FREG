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

#include "Player.h"
#include "World.h"
#include "Shred.h"
#include "WorldMap.h"
#include "BlockFactory.h"
#include "DeferredAction.h"
#include "TrManager.h"
#include "blocks/Animal.h"
#include "blocks/Inventory.h"
#include "Id.h"

#include <QSettings>
#include <QTextStream>
#include <QMutexLocker>

struct PlayerFocusXyz : public XyzInt {
    explicit PlayerFocusXyz(const Player* const player) : XyzInt() {
        emit player->GetFocus(&x_self, &y_self, &z_self);
    }
};

int Player::X() const {
    return GetShred()->ShredX() << SHRED_WIDTH_BITSHIFT | Xyz::X();
}

int Player::Y() const {
    return GetShred()->ShredY() << SHRED_WIDTH_BITSHIFT | Xyz::Y();
}

Block*Player::GetBlock() const { return player; }
const Block*Player::GetConstBlock() const { return player; }

dirs Player::GetDir() const { return player->GetDir(); }

qint64 Player::GlobalX() const { return GetShred()->GlobalX(X()); }
qint64 Player::GlobalY() const { return GetShred()->GlobalY(Y()); }
Shred* Player::GetShred() const { return player->GetShred(); }

void Player::StopUseAll() { usingType = usingSelfType = USAGE_TYPE_NO; }
int  Player::GetUsingInInventory() const { return usingInInventory; }
qint64 Player::GetLongitude() const { return GetShred()->Longitude(); }
qint64 Player::GetLatitude()  const { return GetShred()->Latitude();  }

void Player::SetCreativeMode(const bool creative_on) {
    creativeMode = creative_on;
    player->disconnect();
    SaveState();
    Animal* const previous_player = player;
    SetPlayer(X(), Y(), Z());
    player->SetDir(previous_player->GetDir());
    Inventory* const inv = PlayerInventory();
    if ( inv && previous_player->HasInventory() ) {
        inv->GetAll(previous_player->HasInventory());
    }
    emit Updated();
}

int Player::SatiationPercent() const {
    return ( GetCreativeMode()
            || GROUP_MEAT != Block::GetSubGroup(player->Sub()) ) ?
        50 : player->Satiation()*100/World::SECONDS_IN_DAY;
}

int Player::BreathPercent() const {
    return player->Breath() * 100 / Animal::MAX_BREATH;
}

Inventory* Player::PlayerInventory() const {
    if ( Inventory* const inv = player->HasInventory() ) {
        return inv;
    } else {
        Notify(tr("You have no inventory."));
        return nullptr;
    }
}

void Player::UpdateXYZ() {
    SetXyz(player->X(), player->Y(), player->Z());
    emit Updated();
}

void Player::Examine() const {
    const QMutexLocker locker(World::GetWorld()->GetLock());
    const PlayerFocusXyz focus(this);
    Examine(XYZ(focus));
}

void Player::Examine(const int x, const int y, const int z) const {
    switch ( Visible(x, y, z) ) {
    case VISIBLE: break;
    case IN_SHADOW: Notify(tr("Too dark to see.")); return;
    case OBSCURED:  Notify(tr("It is obscured by something.")); return;
    }
    const World* const world = World::GetCWorld();
    if ( DEBUG ) {
        Notify(Str("Light: %1.").arg(world->Enlightened(x, y, z)));
    }
    Examine(world->GetBlock(x, y, z));
}

void Player::Examine(const Block* const block) const {
    Notify( block->FullName() + Str(". ") + block->Description());
    if ( DEBUG ) {
        Notify(Str(
            "DEBUG. Kind: %1, sub: %2, id: %3. Weight: %4. Light radius: %5.").
            arg(block->Kind()).
            arg(block->Sub()).
            arg(Id(block->Kind(), block->Sub()).id).
            arg(block->Weight()).
            arg(block->LightRadius()));
        Notify(Str(
            "DEBUG. Durability: %1/%2. Norm: %3. Dir: %4. Transparency: %5.").
            arg(block->GetDurability()).
            arg(Block::MAX_DURABILITY).
            arg(block == BlockFactory::Normal(block->Sub())).
            arg(block->GetDir()).
            arg(block->Transparent()));
    }
    if ( Block::GetSubGroup(block->Sub()) == GROUP_AIR ) return;
    Notify(tr("Durability: %1%.").
        arg(block->GetDurability() * 100 / Block::MAX_DURABILITY));
    const QString& str = block->GetNote();
    if ( not str.isEmpty() ) {
        Notify(tr("Inscription: ") + str);
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

void Player::Notify(const QString& message) const {
    emit World::GetCWorld()->Notify(message);
}

void Player::Backpack() {
    if ( PlayerInventory() ) {
        usingSelfType = ( USAGE_TYPE_OPEN==usingSelfType ) ?
            USAGE_TYPE_NO : USAGE_TYPE_OPEN;
    }
    emit Updated();
}

void Player::Use() {
    World* const world = World::GetWorld();
    const QMutexLocker locker(world->GetLock());
    const PlayerFocusXyz focus(this);
    Block* const block = world->GetBlock(XYZ(focus));
    if ( player->NutritionalValue(block->Sub()) ) {
        Notify(tr("To eat %1, you must first pick it up.").
            arg(block->FullName()));
        return;
    } // else:
    const int us_type = block->Use(player);
    if ( us_type == USAGE_TYPE_NO ) {
        Notify(tr("You cannot use %1.").arg(block->FullName()));
        return;
    }
    usingType = ( us_type==usingType ) ? USAGE_TYPE_NO : us_type;
    emit Updated();
}

void Player::TurnBlockToFace() const {
    const PlayerFocusXyz focus(this);
    World* const world = World::GetWorld();
    Block* block = world->GetBlock(XYZ(focus));
    if (Block::GetSubGroup(block->Sub()) != GROUP_AIR) {
        if (block == BlockFactory::Normal(block->Sub())) {
            block = BlockFactory::NewBlock(block->Kind(), block->Sub());
            world->GetShred(XY(focus))->SetBlockNoCheck(block,
                Shred::CoordInShred(focus.X()),
                Shred::CoordInShred(focus.Y()),
                focus.Z());
        }
        const dirs dir = World::Anti(GetDir());
        block->SetDir(dir);
        Notify(block->FullName()
            + tr(": new direction: ")
            + TrManager::DirName(dir).toLower()
            + Str("."));
    }
}

void Player::Inscribe() const {
    World* const world = World::GetWorld();
    const QMutexLocker locker(world->GetLock());
    const PlayerFocusXyz focus(this);
    const QString block_name = world->GetBlock(XYZ(focus))->FullName();
    Notify(player ?
        (world->Inscribe(XYZ(focus)) ?
            tr("Inscribed %1.").arg(block_name) :
            tr("Cannot inscribe %1.").arg(block_name)) :
        tr("No player."));
}

Block* Player::ValidBlock(const int num) const {
    Inventory* const inv = PlayerInventory();
    if ( not inv ) {
        return nullptr;
    } // else:
    if ( num >= inv->Size() ) {
        Notify(tr("No such place."));
        return nullptr;
    } // else:
    Block* const block = inv->ShowBlock(num);
    if ( not block ) {
        Notify(tr("Nothing at slot '%1'.").arg(char(num + 'a')));
        return nullptr;
    } else {
        return block;
    }
}

usage_types Player::Use(const int num) {
    const QMutexLocker locker(World::GetWorld()->GetLock());
    Block* const block = ValidBlock(num);
    if ( block == nullptr ) return USAGE_TYPE_NO;
    if ( player->Eat(block->Sub()) ) {
        PlayerInventory()->Pull(num);
        BlockFactory::DeleteBlock(block);
        return USAGE_TYPE_NO;
    } // else:
    const usage_types result = block->Use(player);
    switch ( result ) {
    case USAGE_TYPE_READ:
        usingInInventory = num;
        usingType = USAGE_TYPE_READ_IN_INVENTORY;
        break;
    case USAGE_TYPE_POUR: {
        const PlayerFocusXyz target(this);
        player->GetDeferredAction()->SetPour(XYZ(target), num);
        } break;
    case USAGE_TYPE_SET_FIRE: {
        const PlayerFocusXyz target(this);
        player->GetDeferredAction()->SetSetFire(XYZ(target));
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
    const PlayerFocusXyz focus(this);
    player->GetDeferredAction()->SetThrow(XYZ(focus), src, dest, num);
}

bool Player::Obtain(const int src, const int dest, const int num) {
    World* const world = World::GetWorld();
    const QMutexLocker locker(world->GetLock());
    const PlayerFocusXyz focus(this);
    bool is_success = world->Get(player, XYZ(focus), src, dest, num);
    emit Updated();
    return is_success;
}

void Player::Wield(const int from) {
    const QMutexLocker locker(World::GetWorld()->GetLock());
    if ( Block* const block = ValidBlock(from) ) {
        Inventory* const inv = PlayerInventory();
        inv->Pull(from);
        inv->Get(block, (from >= inv->Start()) ? 0 : inv->Start());
        emit Updated();
    }
}

void Player::MoveInsideInventory(const int from, const int to, const int num) {
    const QMutexLocker locker(World::GetWorld()->GetLock());
    if ( ValidBlock(from) ) {
        PlayerInventory()->MoveInside(from, to, num);
        emit Updated();
    }
}

void Player::Inscribe(const int num) {
    const QMutexLocker locker(World::GetWorld()->GetLock());
    if ( ValidBlock(num) ) {
        QString str;
        emit GetString(str);
        PlayerInventory()->InscribeInv(num, str);
    }
}

void Player::Build(const int slot) {
    World* const world = World::GetWorld();
    const QMutexLocker locker(world->GetLock());
    Block* const block = ValidBlock(slot);
    if ( block && (AIR != world->GetBlock(X(), Y(), Z()-1)->Sub()
            || 0 == player->Weight()) )
    {
        const PlayerFocusXyz target(this);
        player->GetDeferredAction()->SetBuild(XYZ(target), slot);
    }
}

void Player::Craft(const int num) {
    const QMutexLocker locker(World::GetWorld()->GetLock());
    Inventory* const inv = PlayerInventory();
    if ( inv && inv->MiniCraft(num) ) {
        emit Updated();
    }
}

bool Player::ForbiddenAdminCommands() const {
    if ( GetCreativeMode() || DEBUG ) {
        return false;
    } else {
        Notify(tr("You are not in Creative Mode."));
        return true;
    }
}

void Player::ProcessCommand(QString command) {
    QTextStream command_stream(&command);
    QString request;
    command_stream >> request;
    World* const world = World::GetWorld();
    const QMutexLocker locker(world->GetLock());
    switch ( UniqueIntFromString(request.toLatin1()) ) {
    case UniqueIntFromString(""): break;
    case UniqueIntFromString("weather"):
        Notify(TrManager::GetWeatherString(GetShred()->GetWeather()));
        break;
    case UniqueIntFromString("give"):
    case UniqueIntFromString("get" ):
        ProcessGetCommand(command_stream);
        break;
    case UniqueIntFromString("move"): {
        int direction;
        command_stream >> direction;
        Move(static_cast<dirs>(direction));
        } break;
    case UniqueIntFromString("time"):
        if ( ForbiddenAdminCommands() ) return;
        Notify(world->TimeOfDayStr());
        break;
    case UniqueIntFromString("version"):
        Notify(tr("freg version: %1. Compiled on %2 at %3 with Qt %4.")
            .arg(VER)
            .arg(Str(__DATE__))
            .arg(Str(__TIME__))
            .arg(Str(QT_VERSION_STR)));
        Notify(tr("Current Qt version: %1. Build type: %2.")
            .arg(QString::fromLatin1(qVersion()))
            .arg(DEBUG ? tr("debug") : tr("release")));
        break;
    case UniqueIntFromString("warranty"):
        command_stream << "warranty";
        // no break;
    case UniqueIntFromString("help"):
        command_stream >> request;
        if ( request.isEmpty() ) {
            request = Str("help");
        }
        emit ShowFile( home_path + Str("/help_%1/%2.md")
            .arg(QLocale::system().name().left(2)).arg(request) );
        break;
    case UniqueIntFromString("about"):
        emit ShowFile(home_path + Str("README.md"));
        break;
    case UniqueIntFromString("test_damage"):
        Block::TestDamage();
        break;
    default:
        Notify(tr("Don't know such command: \"%1\".").arg(command));
        break;
    }
} // void Player::ProcessCommand(QString command)

void Player::ProcessGetCommand(QTextStream& command_stream) {
    if ( ForbiddenAdminCommands() ) return;
    Inventory* const inv = PlayerInventory();
    if ( inv == nullptr ) return;
    QString kind, sub;
    command_stream >> kind >> sub;

    auto listKindsMessage = [this]() {
        QStringList kindNames;
        kindNames.reserve(KIND_COUNT);
        for (int i = 0; i < KIND_COUNT; ++i) {
            kindNames.append(TrManager::KindName(i));
        }
        Notify(tr("Available kinds: ") + kindNames.join(Str(", ")) + Str("."));
    };

    if (kind.isEmpty()) {
        listKindsMessage();
        return;
    }
    kinds kind_code;
    subs sub_code;
    switch (UniqueIntFromString(kind.toLatin1())) { // aliases
    case UniqueIntFromString("logger"):
        kind_code = KIND_TEXT;
        sub_code = IRON;
        break;
    case UniqueIntFromString("button"):
        kind_code = SIGNALLER;
        sub_code = WOOD;
        break;
    default:
        kind_code = TrManager::StrToKind(kind);
        if ( kind_code == LAST_KIND ) {
            sub_code = TrManager::StrToSub(kind);
            if (sub_code != LAST_SUB) {
                kind_code = BLOCK;
                break;
            } else {
                Notify(tr("There is no kind \"") + kind + Str("\"."));
                listKindsMessage();
                return;
            }
        }
        sub_code = sub.isEmpty() ?
            STONE : TrManager::StrToSub(sub);
        if ( sub_code == LAST_SUB ) {
            Notify(tr("There is no substance \"") + sub + Str("\"."));
            return;
        }
        break;
    }

    Block* const block = BlockFactory::NewBlock(kind_code, sub_code);
    if ( inv->Get(block) ) {
        emit Updated();
    } else {
        BlockFactory::DeleteBlock(block);
    }
}

Player::visible Player::Visible(const_int(x_to, y_to, z_to)) const {
    if ( GetCreativeMode() ) return VISIBLE;
    const World* const world = World::GetCWorld();
    return world->Visible(Xyz(X(), Y(), Z()), Xyz(x_to, y_to, z_to)) ?
        (world->Enlightened(x_to, y_to, z_to) ?
            VISIBLE : IN_SHADOW) :
        OBSCURED;
}

void Player::SetDir(const dirs direction) {
    usingType = USAGE_TYPE_NO;
    player->SetDir(direction);
    emit Updated();
}

bool Player::Damage() const {
    const PlayerFocusXyz focus(this);
    if ( World::GetCWorld()->InBounds(XYZ(focus)) ) {
        player->GetDeferredAction()->SetDamage(XYZ(focus));
        return true;
    } else {
        return false;
    }
}

void Player::CheckOverstep(const int direction) {
    SetXyz(Shred::CoordInShred(player->X()), Shred::CoordInShred(player->Y()),
        player->Z());
    if ( direction > DOWN
            && not World::GetCWorld()->ShredInCentralZone(
                GetShred()->Longitude(), GetShred()->Latitude()) )
    {
        emit OverstepBorder(direction);
    }
    emit Moved(GlobalX(), GlobalY(), Z());
}

void Player::BlockDestroy() {
    if ( not cleaned ) {
        usingType = usingSelfType = USAGE_TYPE_NO;
        Notify(tr("You die!"));
        emit Destroyed();
        player = nullptr;
        World* const world = World::GetWorld();
        const int plus = world->NumShreds() / 2 * SHRED_WIDTH;
        world->ReloadAllShreds(World::WorldName(),
            homeLati, homeLongi, homeX+plus, homeY+plus, homeZ);
        world->ActivateFullReload();
    }
}

void Player::SetPlayer(int x, int y, int z) {
    LoadState();
    World* const world = World::GetWorld();
    if ( z == -1 ) {
        const int plus = world->NumShreds() / 2;
        x = Xyz::X() + (latitude  - world->Latitude()  + plus) * SHRED_WIDTH;
        y = Xyz::Y() + (longitude - world->Longitude() + plus) * SHRED_WIDTH;
    } else {
        SetXyz(Shred::CoordInShred(x), Shred::CoordInShred(y), z);
    }
    if ( GetCreativeMode() ) {
        player = creator.get();
        creator->SetXyz(Xyz::X(), Xyz::Y(), Xyz::Z());
        world->GetShred(x, y)->Register(player);
    } else {
        Q_ASSERT(z_self <= HEIGHT-2);
        Block* candidate = world->GetBlock(x, y, z_self);
        for ( ; z_self < HEIGHT-2; ++z_self ) {
            candidate = world->GetBlock(x, y, z_self);
            if ( AIR == candidate->Sub()
                    || ((player == creator.get() || player == nullptr)
                        && candidate->IsAnimal()) )
            {
                break;
            }
        }
        if ( candidate->IsAnimal() ) {
            player = candidate->IsAnimal();
        } else {
            if ( player == nullptr || player == creator.get() ) {
                player = BlockFactory::NewBlock(DWARF, PLAYER_SUB)->IsAnimal();
            }
            world->DestroyAndReplace(x, y, Z());
            world->DestroyAndReplace(x, y, Z()); // blocks can drop something
            world->Build(player, x, y, Z());
        }
    }
    connect(player, &QObject::destroyed, this, &Player::BlockDestroy,
        Qt::DirectConnection);
    connect(player, &Animal::Moved, this, &Player::CheckOverstep,
        Qt::DirectConnection);
    connect(player, &Animal::Updated, this, &Player::Updated,
        Qt::DirectConnection);
    connect(player, &Animal::ReceivedText, world, &World::Notify,
        Qt::DirectConnection);
    connect(player, &Animal::CauseTeleportation,
        world, &World::ActivateFullReload, Qt::DirectConnection);
} // Player::SetPlayer(int x, y, z)

void Player::Disconnect() {
    if ( player == nullptr ) { // dead player
        player = BlockFactory::NewBlock(DWARF, PLAYER_SUB)->IsAnimal();
    } else {
        SaveState();
        GetShred()->SetBlockNoCheck(BlockFactory::Normal(AIR),
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
        creator(BlockFactory::NewBlock(DWARF, DIFFERENT)->IsAnimal()),
        usingType(), usingSelfType(),
        usingInInventory(),
        creativeMode()
{
    World* const world = World::GetWorld();
    SetPlayer(0, 0, -1);
    connect(world, &World::NeedPlayer, this, &Player::SetPlayer,
        Qt::DirectConnection);
    connect(this, &Player::OverstepBorder, world, &World::SetReloadShreds,
        Qt::DirectConnection);
    connect(world, &World::StartReloadAll, this, &Player::Disconnect,
        Qt::DirectConnection);
}

Player::~Player() {
    SaveState();
}

void Player::SaveState() const {
    QSettings settings(World::WorldPath() + Str("/player_state.ini"),
        QSettings::IniFormat);
    settings.setValue(Str("home_longitude"), homeLongi);
    settings.setValue(Str("home_latitude"),  homeLati);
    settings.setValue(Str("home_x"), homeX);
    settings.setValue(Str("home_y"), homeY);
    settings.setValue(Str("home_z"), homeZ);
    settings.setValue(Str("current_longitude"), GetShred()->Longitude());
    settings.setValue(Str("current_latitude" ), GetShred()->Latitude ());
    settings.setValue(Str("current_x"), Xyz::X());
    settings.setValue(Str("current_y"), Xyz::Y());
    settings.setValue(Str("current_z"), Xyz::Z());
    settings.setValue(Str("using_type"),      usingType);
    settings.setValue(Str("using_self_type"), usingSelfType);
    settings.setValue(Str("creative_mode"), GetCreativeMode());
    settings.setValue(Str("creator_dir"), creator->GetDir());
}

void Player::LoadState() {
    const World* const world = World::GetCWorld();
    const QSettings settings(World::WorldPath() + Str("/player_state.ini"),
        QSettings::IniFormat);
    homeLongi = settings.value(Str("home_longitude"),
        world->GetMap()->GetSpawnLongitude()).toLongLong();
    homeLati  = settings.value(Str("home_latitude"),
        world->GetMap()->GetSpawnLatitude ()).toLongLong();
    homeX = settings.value(Str("home_x")).toInt();
    homeY = settings.value(Str("home_y")).toInt();
    homeZ = settings.value(Str("home_z"), HEIGHT/2 + 1).toInt();
    usingType     = settings.value(Str("using_type"), USAGE_TYPE_NO).toInt();
    usingSelfType = settings.value(Str("using_self_type"),
        USAGE_TYPE_NO).toInt();
    creativeMode  = settings.value(Str("creative_mode")).toBool();
    SetXyz(settings.value(Str("current_x"), homeX).toInt(),
           settings.value(Str("current_y"), homeY).toInt(),
           settings.value(Str("current_z"), homeZ).toInt());
    longitude = settings.value(Str("current_longitude"),
        homeLongi).toLongLong();
    latitude  = settings.value(Str("current_latitude"),
        homeLati ).toLongLong();
    creator->SetDir(settings.value(Str("creator_dir")).toInt());
}
