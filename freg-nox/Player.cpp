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

#include "blocks.h"
#include "Player.h"
#include "world.h"
#include "Shred.h"
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

ulong Player::GetLongitude() const { return shred->Longitude(); }
ulong Player::GetLatitude()  const { return shred->Latitude();  }

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
	if ( creativeMode ) { //know more
		emit Notify(QString(tr("Light: %1, fire: %2, sun: %3. Transp: %4. Normal: %5. Direction: %6.")).
			arg(world->Enlightened(i, j, k)).
			arg(world->FireLight(i, j, k)/16).
			arg(world->SunLight(i, j, k)).
			arg(world->Transparent(i, j, k)).
			arg(world->GetBlock(i, j, k)->Normal()).
			arg(world->GetBlock(i, j, k)->GetDir()));
	}

	const int sub=world->Sub(i, j, k);
	if ( AIR==sub || SKY==sub || SUN_MOON==sub ) {
		world->Unlock();
		return;	
	}

	if ( ""!=world->GetNote(str, i, j, k) )
		emit Notify(tr("Inscription: ")+str);
	emit Notify(tr("Temperature: ")+
		QString::number(world->Temperature(i, j, k)));
	emit Notify(tr("Durability: ")+
		QString::number(world->Durability(i, j, k)));
	emit Notify(tr("Weight: ")+
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
		world->SetDeferredAction(
			0, 0, 0,
			UP, //doesn't matter here
			DEFERRED_THROW,
			x, y, z,
			0,
			num);
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
				emit Notify(( SECONDS_IN_DAY < pl->Satiation() ) ?
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
	if ( block && (AIR!=world->Sub(x, y, z-1) || 0==player->Weight()) ) {
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

void Player::ProcessCommand(QString & command) {
	//don't forget lock and unlock world mutex where needed:
	//world->WriteLock();
	//or
	//world->ReadLock();
	//world->Unlock();
	QTextStream comm_stream(&command);
	QString request;
	comm_stream >> request;
	if ( "give"==request ) {
		world->WriteLock();
		Inventory * const inv=player->HasInventory();
		if ( !creativeMode ) {
			emit Notify(tr("You are not in Creative Mode."));
		} else if ( !inv ) {
			emit Notify(tr("No room."));
		} else {
			int kind, sub, num;
			comm_stream >> kind >> sub >> num;
			if ( !num ) {
				num=1;
			}
			while ( num && inv->HasRoom() ) {
				for (ushort i=9; i && num; --i) {
					inv->Get(shred->NewBlock(kind, sub));
					--num;
				}
			}
			if ( num > 0 ) {
				emit Notify(QString(tr("No place for %1 things.")).arg(num));
			}
		}
		world->Unlock();
	} else if ( "move"==request ) {
		int dir;
		comm_stream >> dir;
		Move(dir);
	} else if ( "what"==request ) {
		ushort x_what, y_what, z_what;
		comm_stream >> x_what >> y_what >> z_what;
		world->ReadLock();
		if ( creativeMode ) {
			Examine(x_what, y_what, z_what);
		} else {
			if (
					qAbs(x-x_what) > 1 ||
					qAbs(y-y_what) > 1 ||
					qAbs(z-z_what) > 1)
			{
				emit Notify(tr("Too far."));
			} else {
				Examine(x_what, y_what, z_what);
			}
		}
		world->Unlock();
	} else {
		emit Notify(QString(tr("Don't know such command: \"%1\".")).arg(command));
	}
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
			0, //direction doesn't matter here
			DEFERRED_DAMAGE,
			x, y, z,
			0, //what block - doesn't matter
			( creativeMode ? MAX_DURABILITY : player->DamageLevel() ),
			( creativeMode ? CREATIVE : player->DamageKind() ));
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
			x <  (world->NumShreds()/2-1)*SHRED_WIDTH ||
			y <  (world->NumShreds()/2-1)*SHRED_WIDTH ||
			x >= (world->NumShreds()/2+2)*SHRED_WIDTH ||
			y >= (world->NumShreds()/2+2)*SHRED_WIDTH )
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
	homeX-=world->NumShreds()/2*SHRED_WIDTH;
	homeY-=world->NumShreds()/2*SHRED_WIDTH;
}

void Player::WorldSizeReloadFinish() {
	homeX+=world->NumShreds()/2*SHRED_WIDTH;
	homeY+=world->NumShreds()/2*SHRED_WIDTH;
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
	const ushort num_shreds=world->NumShreds();
	if ( num < 5 ) {
		emit Notify(QString(
			"Shreds number too small: %1x%2.").arg(num).arg(num));
		emit Notify(QString("Shreds number is %1x%2.")
			.arg(num_shreds).arg(num_shreds));
	} else if ( 1 != num%2 ) {
		emit Notify(QString(
			"Invalid shreds number: %1x%2.").arg(num).arg(num));
		emit Notify(QString("Shreds number is %1x%2.")
			.arg(num_shreds).arg(num_shreds));
	} else {
		const short shift=num/2 - num_shreds/2;
		world->ReloadAllShreds(
			//put loaded zone center to where player is
			world->Latitude()  + x/SHRED_WIDTH - num_shreds/2,
			world->Longitude() + y/SHRED_WIDTH - num_shreds/2,
			//new x and y correspond to player stanging in loaded zone center
			x - (x/SHRED_WIDTH)*SHRED_WIDTH + (num_shreds/2+shift)*SHRED_WIDTH,
			y - (y/SHRED_WIDTH)*SHRED_WIDTH + (num_shreds/2+shift)*SHRED_WIDTH,
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
		z=homeZ=HEIGHT/2;
	} else {
		homeLongi=sett.value("home_longitude", qlonglong(world->GetSpawnLongi())).toLongLong();
		homeLati =sett.value("home_latitude", qlonglong(world->GetSpawnLati())).toLongLong();
		homeX    =sett.value("home_x", 0).toInt();
		homeY    =sett.value("home_y", 0).toInt();
		homeZ    =sett.value("home_z", HEIGHT/2).toInt();
		x        =sett.value("current_x", 0).toInt();
		y        =sett.value("current_y", 0).toInt();
		z        =sett.value("current_z", HEIGHT/2).toInt();
		creativeMode=sett.value("creative_mode", false).toBool();
	}

	const ushort plus=world->NumShreds()/2*SHRED_WIDTH;
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

	const ushort min=world->NumShreds()/2*SHRED_WIDTH;
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
	sett.setValue("creative_mode", creativeMode);

	world->Unlock();
}
