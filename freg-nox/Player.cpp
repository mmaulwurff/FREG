	/*
	*This file is part of FREG.
	*
	*FREG is free software: you can redistribute it and/or modify
	*it under the terms of the GNU General Public License as published by
	*the Free Software Foundation, either version 3 of the License, or
	*(at your option) any later version.
	*
	*FREG is distributed in the hope that it will be useful,
	*but WITHOUT ANY WARRANTY; without even the implied warranty of
	*MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	*GNU General Public License for more details.
	*
	*You should have received a copy of the GNU General Public License
	*along with FREG. If not, see <http://www.gnu.org/licenses/>.
	*/

#include <blocks.h>
#include <Player.h>
#include <world.h>
#include <QFile>
#include <QTextStream>
#include <QString>
#include <QSettings>

short Player::HP() const {
	return player ? player->Durability() : 0;
}

short Player::Breath() const {
	if ( !player )
		return -1;
	Animal const * const animal=player->IsAnimal();
	return animal ? animal->Breath() : -1;
}

short Player::Satiation() const {
	if ( !player )
		return -1;
	Animal const * const animal=player->IsAnimal();
	return animal ? animal->Satiation() : -1;
}

Inventory * Player::PlayerInventory() {
	return player ? player->HasInventory() : 0;
}

void Player::UpdateXYZ() {
	if ( player ) {
		x=player->X();
		y=player->Y();
		z=player->Z();
	}
}

void Player::Focus(
		ushort & i_target,
		ushort & j_target,
		ushort & k_target) const
{
	if ( player )
	world->Focus(x, y, z,
		i_target, j_target, k_target,
		player->GetDir());
	else {
		i_target=x;
		j_target=y;
		k_target=z;
	}
}

void Player::Examine(
		const short i,
		const short j,
		const short k) const
{
	world->ReadLock();

	emit Notify("------");
	QString str;
	emit Notify( world->FullName(str, i, j, k) );

	const int sub=world->Sub(i, j, k);
	if ( AIR==sub || SKY==sub || SUN_MOON==sub ) {
		world->Unlock();
		return;	
	}

	if ( ""!=world->GetNote(str, i, j, k) )
		emit Notify("Inscription: "+str);
	emit Notify("Temperature: "+
		QString::number(world->Temperature(i, j, k)));
	emit Notify("Durability: "+
		QString::number(world->Durability(i, j, k)));
	emit Notify("Weight: "+
		QString::number(world->Weight(i, j, k)));
	world->Unlock();
}

void Player::Jump() {
	world->WriteLock();
	usingType=NO;
	world->SetDeferredAction(x, y, z, dir, DEFERRED_JUMP);
	world->Unlock();
}

int Player::Move(const int dir) {
	world->WriteLock();
	usingType=NO;
	world->SetDeferredAction(x, y, z, dir, DEFERRED_MOVE);
	world->Unlock();
	return 0;
}

void Player::Turn(const int dir) {
	world->WriteLock();
	usingType=NO;
	Dir(( (DOWN==Dir() && UP!=dir) ||
			(UP==Dir() && DOWN!=dir) ) ?
		NORTH :
		dir);
	emit Updated();
	world->Unlock();
}

void Player::Backpack() {
	if ( player && player->HasInventory() ) {
		usingSelfType=( OPEN==usingSelfType ) ? NO : OPEN;
		emit Updated();
	}
}

void Player::Use(
		const short x,
		const short y,
		const short z)
{
	world->WriteLock();
	const int us_type=world->Use(x, y, z);
	usingType=( us_type==usingType ) ? NO : us_type;
	world->Unlock();
}

void Player::Inscribe(
		const short x,
		const short y,
		const short z) const
{
	world->WriteLock();
	if ( player ) {
		if ( ((Dwarf *)player)->CarvingWeapon() ) {
			if ( !world->Inscribe(x, y, z) )
				emit Notify("Cannot inscribe this.");
		} else
			emit Notify("You need some tool to inscribe.");
	}
	world->Unlock();
}

Block * Player::ValidBlock(const ushort num) const {
	if ( !player ) {
		emit Notify("Player does not exist.");
		return 0;
	}
	Inventory * const inv=player->HasInventory();
	if ( !inv ) {
		emit Notify("Player has no inventory.");
		return 0;
	}
	if ( num>=inv->Size() ) {
		emit Notify("No such place.");
		return 0;
	}
	Block * const block=inv->ShowBlock(num);
	if ( !block ) {
		emit Notify("Nothing here.");
		return 0;
	}
	return block;
}

void Player::Use(const ushort num) {
	world->WriteLock();
	Block * const block=ValidBlock(num);
	if ( block )
		block->Use();
	world->Unlock();
}

void Player::Throw(const ushort num) {
	world->WriteLock();
	Block * const block=ValidBlock(num);
	if ( block ) {
		const int err=world->Drop(x, y, z, num);
		if ( 5==err || 4==err )
			emit Notify("No place to drop.");
		else
			emit Updated();
	}
	world->Unlock();
}

void Player::Obtain(const ushort num) {
	world->WriteLock();
	const int err=world->Get(x, y, z, num);
	if ( 5==err || 3==err || 6==err )
		emit Notify("Nothing here.");
	else if ( 4==err || 2==err )
		emit Notify("No room.");
	else
		emit Updated();
	world->Unlock();
}

void Player::Wield(const ushort num) {
	world->WriteLock();
	if ( ValidBlock(num) ) {
		if ( DWARF==player->Kind() ) {
			const int wield_code=((Dwarf *)player)->Wield(num);
			if ( 1==wield_code )
				emit Notify("Nothing here.");
			else if ( 2==wield_code )
				emit Notify("Cannot wield this.");
			else
				emit Updated();
		} else
			emit Notify("You can wield nothing.");
	}
	world->Unlock();
}

void Player::Inscribe(const ushort num) {
	world->WriteLock();
	if ( ValidBlock(num) ) {
		QString str;
		emit GetString(str);
		Inventory * const inv=PlayerInventory();
		const int err=inv->InscribeInv(num, str);
		if ( 1==err )
			emit Notify("Cannot inscribe this.");
		else
			emit Notify("Inscribed.");
	}
	world->Unlock();
}

void Player::Eat(const ushort num) {
	world->WriteLock();
	Block * const food=ValidBlock(num);
	if ( food ) {
		Animal * const pl=player->IsAnimal();
		if ( pl ) {
			const int eat=pl->Eat(food);
			if ( 2==eat )
				emit Notify("You can't eat this.");
			else {
				emit Notify(( seconds_in_day < pl->Satiation() ) ?
					"You have gorged yourself!" : "Yum!");
				PlayerInventory()->Pull(num);
				if ( !food->Normal() )
					delete food;
				emit Updated();
			}
		} else
			emit Notify("You can't eat.");
	}
	world->Unlock();
}

void Player::Build(
		const short x_target,
		const short y_target,
		const short z_target,
		const ushort num)
{
	world->WriteLock();
	Block * const block=ValidBlock(num);
	if ( block && !player->IsFalling() ) {
		world->SetDeferredAction(
			x_target, y_target, z_target,
			Dir(),
			DEFERRED_BUILD,
			x, y, z,
			block,
			num);
	}
	world->Unlock();
}

void Player::Craft(const ushort num) {
	world->WriteLock();
	Inventory * const inv=PlayerInventory();
	if ( inv ) {
		const int craft=inv->MiniCraft(num);
		if ( 1==craft )
			Notify("Nothing here.");
		else if ( 2==craft )
			Notify("You don't know how to make something from this.");
		else {
			Notify("Craft successful.");
			emit Updated();
		}
	} else
		Notify("Cannot craft.");
	world->Unlock();
}

void Player::TakeOff(const ushort num) {
	world->WriteLock();
	if ( ValidBlock(num) ) {
		Inventory * const inv=PlayerInventory();
		if ( inv->HasRoom() )
			inv->Drop(num, inv);
		else
			emit Notify("No place to take off.");
	}
	world->Unlock();
}

void Player::Get(Block * const block) {
	if ( player ) {
		Inventory * const inv=player->HasInventory();
		if ( inv )
			inv->Get(block);
	}
}

bool Player::Visible(
		const ushort x_to,
		const ushort y_to,
		const ushort z_to) const
{
	return world->Visible(x, y, z, x_to, y_to, z_to);
}

Block * Player::UsingBlock() const {
	ushort x, y, z;
	Focus(x, y, z);
	return world->GetBlock(x, y, z);
}

void Player::Dir(const int direction) {
	if ( player )
		player->SetDir(direction);
	dir=direction;
}

int Player::Dir() const { return dir; }

void Player::Damage(
		const short x_target,
		const short y_target,
		const short z_target) const
{
	world->WriteLock();
	world->SetDeferredAction(
			x_target, y_target, z_target,
			UP, //doesn't matter here
			DEFERRED_DAMAGE,
			x, y, z);
	world->Unlock();
}

int Player::DamageKind() const {
	return player ? player->DamageKind() : NO_HARM;
}

ushort Player::DamageLevel() const {
	return player ? player->DamageLevel() : 0;
}

void Player::CheckOverstep(const int dir) {
	UpdateXYZ();
	if (
			x <  (world->NumShreds()/2-1)*shred_width ||
			y <  (world->NumShreds()/2-1)*shred_width ||
			x >= (world->NumShreds()/2+2)*shred_width ||
			y >= (world->NumShreds()/2+2)*shred_width )
	{
		emit OverstepBorder(dir);
		UpdateXYZ();
	}
	shred=world->GetShred(x, y);
}

void Player::BlockDestroy() {
	if ( cleaned )
		return;
	emit Notify("You died.");
	player=0;
	usingType=NO;
	usingSelfType=NO;

	world->ReloadAllShreds(
		homeLati,
		homeLongi,
		homeX,
		homeY,
		homeZ,
		world->NumShreds());
}

void Player::WorldSizeReloadStart() {
	if ( player )
		disconnect(player, SIGNAL(Destroyed()), 0, 0);
	homeX-=world->NumShreds()/2*shred_width;
	homeY-=world->NumShreds()/2*shred_width;
}

void Player::WorldSizeReloadFinish() {
	homeX+=world->NumShreds()/2*shred_width;
	homeY+=world->NumShreds()/2*shred_width;
}

void Player::SetPlayer(
		const ushort player_x,
		const ushort player_y,
		const ushort player_z)
{
	x=player_x;
	y=player_y;
	z=player_z;
	shred=world->GetShred(x, y);
	if ( DWARF!=world->Kind(x, y, z) ) {
		Block * const temp=world->GetBlock(x, y, z);
		if ( !temp->Normal() )
			delete temp;
		world->SetBlock(
			(player=new Dwarf(shred, x, y, z)),
			x, y, z);
	} else {
		player=world->ActiveBlock(x, y, z);
	}
	Dir(world->GetBlock(x, y, z)->GetDir());

	connect(player, SIGNAL(Destroyed()),
		this, SLOT(BlockDestroy()),
		Qt::DirectConnection);
	connect(player, SIGNAL(Moved(int)),
		this, SLOT(CheckOverstep(int)),
		Qt::DirectConnection);
	connect(player, SIGNAL(Updated()),
		this, SIGNAL(Updated()),
		Qt::DirectConnection);
}

void Player::SetNumShreds(ushort num) const {
	if ( num < 5 ) {
		emit Notify(QString(
			"Shreds number too small: %1x%2.").arg(num).arg(num));
		emit Notify(QString("Shreds number is %1x%2.")
			.arg(world->NumShreds()).arg(world->NumShreds()));
	} else if ( 1 != num%2 ) {
		emit Notify(QString(
			"Invalid shreds number: %1x%2.").arg(num).arg(num));
		emit Notify(QString("Shreds number is %1x%2.")
			.arg(world->NumShreds()).arg(world->NumShreds()));
	} else {
		world->ReloadAllShreds(
			world->Latitude(),
			world->Longitude(),
			x+(num/2-world->NumShreds()/2)*shred_width,
			y+(num/2-world->NumShreds()/2)*shred_width,
			z,
			num);
		emit Notify(QString("Shreds number is %1x%2.")
			.arg(num).arg(num));
	}
}

Player::Player(World * const w) :
		world(w),
		usingType(NO),
		usingSelfType(NO),
		cleaned(false)
{
	QString temp;
	world->WorldName(temp);
	QSettings sett;
	sett.beginGroup("player");
	if ( sett.value("world").toString() != temp ) {
		homeLongi=world->GetSpawnLongi();
		homeLati= world->GetSpawnLati();
		x=homeX=0;
		y=homeY=0;
		z=homeZ=height/2;
	} else {
		homeLongi=sett.value("home_longitude", qlonglong(world->GetSpawnLongi())).toLongLong();
		homeLati =sett.value("home_latitude", qlonglong(world->GetSpawnLati())).toLongLong();
		homeX    =sett.value("home_x", 0).toInt();
		homeY    =sett.value("home_y", 0).toInt();
		homeZ    =sett.value("home_z", height/2).toInt();
		x        =sett.value("current_x", 0).toInt();
		y        =sett.value("current_y", 0).toInt();
		z        =sett.value("current_z", height/2).toInt();
	}

	const ushort plus=world->NumShreds()/2*shred_width;
	homeX+=plus;
	homeY+=plus;
	x+=plus;
	y+=plus;
	SetPlayer(x, y, z);

	connect(world, SIGNAL(NeedPlayer(
			const ushort,
			const ushort,
			const ushort)),
		this, SLOT(SetPlayer(
			const ushort,
			const ushort,
			const ushort)),
		Qt::DirectConnection);
	connect(this, SIGNAL(OverstepBorder(int)),
		world, SLOT(ReloadShreds(int)),
		Qt::DirectConnection);
	connect(world, SIGNAL(StartReloadAll()),
		this, SLOT(WorldSizeReloadStart()),
		Qt::DirectConnection);
	connect(world, SIGNAL(FinishReloadAll()),
		this, SLOT(WorldSizeReloadFinish()),
		Qt::DirectConnection);
}

void Player::CleanAll() {
	world->WriteLock();
	if ( cleaned ) {
		world->Unlock();
		return;
	}
	cleaned=true;

	const ushort min=world->NumShreds()/2*shred_width;
	QSettings sett;
	sett.beginGroup("player");
	QString temp;
	world->WorldName(temp);
	sett.setValue("world", temp);
	sett.setValue("home_longitude", qlonglong(homeLongi));
	sett.setValue("home_latitude", qlonglong(homeLati));
	sett.setValue("home_x", homeX-min);
	sett.setValue("home_y", homeY-min);
	sett.setValue("home_z", homeZ);
	sett.setValue("current_x", x-min);
	sett.setValue("current_y", y-min);
	sett.setValue("current_z", z);

	world->Unlock();
}
