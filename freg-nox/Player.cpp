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
#include <Shred.h>
#include <QFile>
#include <QTextStream>
#include <QString>

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

void Player::Examine() const {
	ushort i, j, k;
	world->ReadLock();
	Focus(i, j, k);
	
	emit Notify("");

	QString str;
	emit Notify( world->FullName(str, i, j, k) );

	if ( ""!=world->GetNote(str, i, j, k) )
		emit Notify("Inscription: "+str);

	emit Notify("Temperature: "+
		QString::number(world->Temperature(i, j, k)));

	emit Notify("Durability: "+
		QString::number(world->Durability(i, j, k)));

	emit Notify("Weight: "+
		QString::number(world->Weight(i, j, k)));
	world->Unlock();
	emit Notify(str);
}

void Player::Jump() {
	world->WriteLock();
	usingType=NO;
	world->Jump(x, y, z);
	world->WriteLock();
}

int Player::Move(const int dir) {
	world->WriteLock();
	usingType=NO;
	const int ret=world->Move(x, y, z, dir);
	world->Unlock();
	return ret;
}

void Player::Turn(const int dir) {
	world->WriteLock();
	usingType=NO;
	Dir(dir);
	emit Updated();
	world->Unlock();
}

void Player::Backpack() {
	if ( player->HasInventory() ) {
		usingSelfType=( OPEN==usingSelfType ) ? NO : OPEN;
		emit Updated();
	}
}

void Player::Use() {
	ushort i, j, k;
	world->WriteLock();
	Focus(i, j, k);
	const int us_type=world->Use(i, j, k);
	usingType=( us_type==usingType ) ?
		NO :
		us_type;
	world->Unlock();
}

void Player::Inscribe() const {
	world->WriteLock();
	world->Inscribe(x, y, z);
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
	return inv->ShowBlock(num);
}

void Player::Use(const ushort num) {
	world->WriteLock();
	Block * const block=ValidBlock(num);
	if ( block )
		block->Use();
	else
		emit Notify("Nothing here.");
	world->Unlock();
}

void Player::Throw(const ushort num) {
	world->WriteLock();
	Block * const block=ValidBlock(num);
	if ( !block ) {
		world->Unlock();
		emit Notify("Nothing here.");
		return;
	}
	if ( !world->Drop(x, y, z, num) )
		emit Notify("No place to drop.");

	world->Unlock();
}

void Player::Eat(const ushort n) {
	if ( !player )
		return;
	world->WriteLock();
	Animal * const pl=player->IsAnimal();
	if ( !pl ) {
		world->Unlock();
		return;
	}

	Block * food=Drop(n);
	if ( !food ) {
		world->Unlock();
		return;
	}
	if ( !pl->Eat(food) ) {
		Get(food);
		emit Notify("You can't eat this.");
	} else
		emit Notify("Yum!");
	if ( seconds_in_day*time_steps_in_sec < pl->Satiation() )
		emit Notify("You have gorged yourself!");
	world->Unlock();
}

void Player::Build(const ushort n) {
	world->WriteLock();
	Block * const temp=Drop(n);
	ushort i, j, k;
	Focus(i, j, k);
	if ( !world->Build(temp, i, j, k) )
		Get(temp);
	world->Unlock();
}

Block * Player::Drop(const ushort n) {
	if ( !player )
		return 0;

	Inventory * const inv=player->HasInventory();
	return inv ? inv->Drop(n) : 0;
}

void Player::Get(Block * block) {
	if ( !player )
		return;

	Inventory * inv=player->HasInventory();
	if ( inv )
		inv->Get(block);
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

void Player::Damage() {
	ushort i, j, k;
	world->WriteLock();
	Focus(i, j, k);
	world->Damage(i, j, k);
	world->Unlock();
}

void Player::CheckOverstep(const int dir) {
	UpdateXYZ();
	if ( shred!=world->GetShred(x, y) ) {
		shred=world->GetShred(x, y);
		emit OverstepBorder(dir);
		UpdateXYZ();
	}
}

Player::Player(World * const w) :
		world(w),
		usingType(NO),
		usingSelfType(NO),
		cleaned(false)
{
	world->WriteLock();

	QFile file("player_save");
	if ( !file.open(QIODevice::ReadOnly | QIODevice::Text) ) {
		homeLongi=world->GetSpawnLongi();
		homeLati= world->GetSpawnLati();
		x=homeX=0;
		y=homeY=0;
		z=homeZ=height/2;
	} else {
		QTextStream in(&file);
		QString temp, temp2;
		in >> temp >> temp;
		if ( temp!=world->WorldName(temp2) ) {
			homeLongi=world->GetSpawnLongi();
			homeLati= world->GetSpawnLati();
			x=homeX=0;
			y=homeY=0;
			z=homeZ=height/2;
		} else {
			in >> temp >> homeLongi;
			in >> temp >> homeLati;
			in >> temp >> homeX;
			in >> temp >> homeY;
			in >> temp >> homeZ;
			in >> temp >> x;
			in >> temp >> y;
			in >> temp >> z;
		} 

		file.close();
	}

	const ushort plus=world->NumShreds()/2*shred_width;
	homeX+=plus;
	homeY+=plus;
	x+=plus;
	y+=plus;
	
	shred=world->GetShred(x, y);

	if ( DWARF!=world->Kind(x, y, z) ) {
		Block * const temp=world->GetBlock(x, y, z);
		if ( !temp->Normal() )
			delete temp;
		player=new Dwarf(shred, x, y, z);
		world->SetBlock(player, x, y, z);
	} else
		player=world->ActiveBlock(x, y, z);
	dir=world->GetBlock(x, y, z)->GetDir();

	connect(player, SIGNAL(Moved(int)),
		this, SLOT(CheckOverstep(int)),
		Qt::DirectConnection);
	connect(player, SIGNAL(Destroyed()),
		this, SLOT(BlockDestroy()),
		Qt::DirectConnection);
	connect(this, SIGNAL(OverstepBorder(int)),
		world, SLOT(ReloadShreds(int)),
		Qt::DirectConnection);

	world->Unlock();
}

void Player::CleanAll() {
	world->WriteLock();
	if ( cleaned ) {
		world->Unlock();
		return;
	}
	cleaned=true;

	QString str;
	QFile file("player_save");
	if ( !file.open(QIODevice::WriteOnly | QIODevice::Text) ) {
		QTextStream errors(stderr);
		errors << "Player::Cleanall(): Savefile write error: "
			<< str << endl;
		world->Unlock();
		return;
	}
	
	const ushort min=world->NumShreds()/2*shred_width;
	QTextStream out(&file);
	out << "World:\n" << world->WorldName(str) << endl
		<< "Home_longitude:\n" << homeLongi << endl
		<< "Home_latitude:\n" << homeLati << endl
		<< "Home_x:\n" << homeX-min << endl
		<< "Home_y:\n" << homeY-min << endl
		<< "Home_z:\n" << homeZ << endl
		<< "Current_x:\n" << x-min << endl
		<< "Current_y:\n" << y-min << endl
		<< "Current_z:\n" << z << endl;
	file.close();

	world->Unlock();
}
