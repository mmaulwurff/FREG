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

#ifndef BLOCKS_H
#define BLOCKS_H

#include "header.h"

class World;
class Block;
void BlockFromFile(FILE *, Block * &, World * =NULL, unsigned short=0, unsigned short=0, unsigned short=0);

class Block { //blocks without special physics and attributes
	protected:
	subs sub;
	float weight;
	float shown_weight;
	dirs direction;
	char * note;
	void CleanString(char * const str) {
		unsigned short i;
		for (i=0; str[i]!='/'; ++i) str[i]=' ';
		str[i]=' ';
	}
	short durability;

	public:
	short enlightened;
	virtual char * FullName(char * const str) const {
		switch (sub) {
			case WATER:      return WriteName(str, "Ice");
			case STONE:      return WriteName(str, "Stone");
			case MOSS_STONE: return WriteName(str, "Moss stone");
			case NULLSTONE:  return WriteName(str, "Nullstone");
			case GLASS:      return WriteName(str, "Glass");
			case STAR: case SUN_MOON:
			case SKY:        return WriteName(str, "Air");
			case SOIL:       return WriteName(str, "Soil");
			case HAZELNUT:   return WriteName(str, "Hazelnut");
			case WOOD:       return WriteName(str, "Wood");
			case GREENERY:   return WriteName(str, "Leaves");
			case ROSE:       return WriteName(str, "Rose");
			case A_MEAT:     return WriteName(str, "Animal meat");
			case H_MEAT:     return WriteName(str, "Not animal meat");
			default:
				fprintf(stderr, "Block::FullName(char *): Block has unknown substance: %d", int(sub));
				return WriteName(str, "Unknown block");
		}
	}

	void SetWeight(const double m) { shown_weight=m; }
	void SetWeight() { shown_weight=weight; }
	virtual double Weight() const { return shown_weight; }
	dirs GetDir() const { return direction; }
	void SetDir(const dirs dir) { direction=dir; }
	subs Sub() const { return sub; }
	virtual void Inscribe(const char * const str) {
		if (NULL==note) note=new char[note_length];
		strncpy(note, str, note_length);
		if ('\n'==note[0]) {
			delete [] note;
			note=NULL;
		}
	}
	virtual bool GetNote(char * const str) const {
		if (NULL==note) {
			str[0]='\0';
			return false;
		}
			
		strncpy(str, note, note_length);
		return true;
	}

	virtual kinds Kind() const { return BLOCK; }
	virtual bool CanBeIn() const { return true; }
	virtual bool CanBeOut() const {
		switch (sub) {
			case HAZELNUT: return false;
			default: return true;
		}
	}
	virtual int Movable() const { return NOT_MOVABLE; }
	virtual int Transparent() const { //0 - normal block, 1 - block is visible, but light can pass through it, 2 - invisible block
		switch (sub) {
			case WATER: case GREENERY:
			case GLASS: return 1;
			default: return 0;
		}
	}
	virtual before_move_return BeforeMove(const dirs) { return NOTHING; }
	virtual int Move(const dirs) { return 0; }
	virtual char MakeSound() const { return ' '; }
	virtual usage_types Use() { return NO; }
	virtual int Damage(const unsigned short dmg, const damage_kinds dmg_kind=CRUSH) {
		if (0>=durability)
			return 0;
		switch (sub) {
			case GLASS: return durability=0;
			case MOSS_STONE:
			case STONE:  return ((MINE==dmg_kind) ? durability-=2*dmg : durability-=dmg);
			case GREENERY: case ROSE: case HAZELNUT:
			case WOOD:   return  ((CUT==dmg_kind) ? durability-=2*dmg : durability-=dmg);
			case SAND:
			case SOIL:   return  ((DIG==dmg_kind) ? durability-=2*dmg : durability-=dmg);
			case A_MEAT:
			case H_MEAT: return ((THRUST==dmg_kind) ? durability-=2*dmg : durability-=dmg);
			case DIFFERENT: case AIR: case SKY: case SUN_MOON: case WATER:
			case NULLSTONE: return durability;
			default: return durability-=dmg;
		}
	}
	virtual short Max_durability() const { return max_durability; }
	void Restore() { durability=Max_durability(); }
	short Durability() const { return durability; }
	virtual Block * DropAfterDamage() const {
		if (BLOCK==Kind() && GLASS!=sub)
			return new Block(sub);
		return NULL;
	}
	virtual void * HasInventory() { return NULL; }
	virtual void * ActiveBlock() { return NULL; }
	virtual Block * Drop(int n) { return NULL; }

	virtual bool Armour() const { return false; }
	virtual bool Weapon() const { return false; }
	virtual bool Carving() const { return false; }

	virtual void ToDestroy(const bool=false) {}
	virtual void Unregister() {}
	virtual int Eat(Block *) { return 0; }

	virtual float LightRadius() const { return 0; }

	virtual int Temperature() const { 
		switch (sub) {
			case WATER: return -100;
			default: return 0;
		}
	}

	virtual bool operator==(const Block& block) const {
		return ( block.Kind()==Kind() && block.Sub()==Sub() ) ? true : false;
	}

	void SaveToFile(FILE * const out) const {
		fprintf(out, "%d", (int)Kind());
		SaveAttributes(out);
		fprintf(out, "\n");
	}
	virtual void SaveAttributes(FILE * const out) const {
	       	fprintf(out, "_%d_%f_%d_%hd_%hd", sub, weight, direction, durability, enlightened);
		if (NULL!=note) fprintf(out, "_%lu/%s", strlen(note), note);
		else fprintf(out, "_0/");
	}

	Block(const subs n=STONE, const short dur=max_durability, const double w=0) :
			sub(n), direction(NORTH), note(NULL), durability(dur), enlightened(1) {
		if (w) {
			weight=w;
			shown_weight=weight;
			return;
		}

		switch (sub) {
			case NULLSTONE: weight=4444; break;
			case SOIL:      weight=1500; break;
			case GLASS:     weight=2500; break;
			case WOOD:      weight=999;  break;
			case IRON:      weight=7874; break;
			case GREENERY:  weight=2;    break;
			case SAND:      weight=1250; break;
			case ROSE:
			case HAZELNUT:  weight=0.1;  break;
			case MOSS_STONE:
			case STONE:     weight=2600; break;
			case A_MEAT:    weight=1;    break;
			case H_MEAT:    weight=1;    break;
			default: weight=1000;
		}
		shown_weight=weight;
	}
	Block(char * const str) {
		unsigned short note_len;
	       	sscanf(str, "%*d_%d_%f_%d_%hd_%hd_%hd", (int *)(&sub), &weight, (int *)(&direction), &durability, &enlightened, &note_len);
		shown_weight=weight;
		CleanString(str);
		if (0!=note_len) {
			note=new char[note_length];
			sscanf(str, " %s", note);
			unsigned short len, i;
			for (len=0; ' '==str[len]; ++len);
			for (i=len; i<len+note_len; ++i) str[i]=' ';
		} else note=NULL;
	}
	virtual ~Block() { if (NULL!=note) delete [] note; }
};

class Telegraph : public Block {
	public:
	kinds Kind() const { return TELEGRAPH; }
	char * FullName(char * const str) const { return WriteName(str, "Telegraph"); }

	void Inscribe(const char * const str) {
		Block::Inscribe(str);
		char command[note_length+40];
		if (NULL!=note) {
			strcpy(command, "echo '");
			strcat(command, note);
			strcat(command, "' | ttytter 2>&1 > /dev/null&");
			system(command);
		}
	}
	Block * DropAfterDamage() const { return new Telegraph(); }

	void SaveAttributes(FILE * const out) const { Block::SaveAttributes(out); }

	Telegraph() : Block(IRON) {}
	Telegraph(char * const str) : Block(str) {}
};

class Weapons : public Block {
	public:
	virtual kinds Kind() const=0;
	bool Weapon() const { return true; }
	bool CanBeOut() const { return false; }

	virtual void SaveAttributes(FILE * const out) const { Block::SaveAttributes(out); }

	Weapons(const subs sub, const short dur=max_durability) : Block(sub, dur) {}
	Weapons(char * const str) : Block(str) {}
};

class Pick : public Weapons {
	public:
	virtual kinds Kind() const { return PICK; }
	virtual char * FullName(char * const str) const { 
		switch (sub) {
			case IRON: return WriteName(str, "Iron pick");
			default:
				fprintf(stderr, "Pic::FullName(char *): Pick has unknown substance: %d\n", int(sub));
				return WriteName(str, "Strange pick");
		}
	}

	virtual bool Carving() const { return true; }
	double Weight() const { return 10; }

	virtual void SaveAttributes(FILE * const out) const { Weapons::SaveAttributes(out); }

	Pick(const subs sub, const short durability=max_durability) : Weapons(sub, durability) {}
	Pick(char * const str) : Weapons(str) {}
};

class Active : public Block {
	Active * next;
	Active * prev;
	bool ifToDestroy;

	protected:
	unsigned short x_self, y_self, z_self;
	World * whereWorld;

	public:
	virtual char * FullName(char * const str) const {
		switch (sub) {
			case SAND: return WriteName(str, "Sand");
			default:
				fprintf(stderr, "Active:FullName(char *): Unlisted sub: %d\n", (int)sub);
				return WriteName(str, "Unkown active block");
		}
	}
	virtual kinds Kind() const { return ACTIVE; }

	void * ActiveBlock() { return this; }
	virtual int Move(const dirs dir) {
		switch (dir) {
			case NORTH: --y_self; break;
			case SOUTH: ++y_self; break;
			case EAST:  ++x_self; break;
			case WEST:  --x_self; break;
			case UP:    ++z_self; break;
			case DOWN:  --z_self; break;
			default: fprintf(stderr, "Active::Move(dirs): unlisted dir: %d\n", (int)dir);
		}
		return 0;
	}

	Active * GetNext() const { return next; }
	void GetSelfXYZ(unsigned short & x, unsigned short & y, unsigned short & z) const {
		x=x_self;
		y=y_self;
		z=z_self;
	}
	void GetSelfXY(unsigned short & x, unsigned short & y) const {
		x=x_self;
		y=y_self;
	}
	void GetSelfZ(unsigned short & z) const { z=z_self; }
	World * GetWorld() const { return whereWorld; }
	virtual void Act() {}
	void SafeMove();
	void SafeJump();

	virtual char MakeSound() const { return ' '; }
	virtual unsigned short Noise() const { return 0; }

	virtual bool IfToDestroy() const { return ifToDestroy; }
	void ToDestroy(const bool to_dest=true) { ifToDestroy=to_dest; }
	virtual int Movable() const { return MOVABLE; }
	virtual bool ShouldFall() const { return true; }

	virtual void SaveAttributes(FILE * const out) const { Block::SaveAttributes(out); }

	void ReloadToNorth() { y_self+=shred_width; }
	void ReloadToSouth() { y_self-=shred_width; }
	void ReloadToWest()  { x_self+=shred_width; }
	void ReloadToEast()  { x_self-=shred_width; }

	void Register(World *, int, int, int);
	void Unregister();

	Active(const subs sub, const short dur=max_durability) :
		Block(sub, dur),
		ifToDestroy(false),
		whereWorld(NULL) {}
	Active(World * const w, const unsigned short x, const unsigned short y, const unsigned short z, subs sub, const short dur=max_durability) :
			Block(sub, dur),
			ifToDestroy(false)
		{ Register(w, x, y, z); }
	Active(World * const w, const unsigned short x, const unsigned short y, const unsigned short z, char * str) :
			Block(str),
			ifToDestroy(false)	
		{ Register(w, x, y, z); }
	virtual ~Active() { Unregister(); }
};

class Animal : public Active {
	protected:
	short breath;
	int satiation;
	public:
	virtual char * FullName(char * const) const=0;

	short Breath() const { return breath; }
	short Satiation() const { return satiation; }
	virtual int Eat(Block *)=0;

	virtual void Act();

	virtual void SaveAttributes(FILE * const out) const {
		Active::SaveAttributes(out);
		fprintf(out, "%hd_%d/", breath, satiation);
	}

	Animal(World * const w, const unsigned short i, const unsigned short j, const unsigned short k, subs sub=A_MEAT, const short dur=max_durability) :
		Active(w, i, j, k, sub, dur), breath(max_breath), satiation(seconds_in_day*time_steps_in_sec) {}
	Animal(World * const w, const unsigned short i, const unsigned short j, const unsigned short k, char * str) :
		Active(w, i, j, k, str) {
			sscanf(str, " %hd_%d/", &breath, &satiation);
			CleanString(str);		
		}
};

class Inventory {
	protected:
	Block * inventory[inventory_size][max_stack_size];

	public:
	char * InvFullName(char * const str, const int i) const { return (NULL==inventory[i][0]) ? WriteName(str, "") : inventory[i][0]->FullName(str); }
	char * NumStr(char * const str, const int i) const {
		unsigned short n=Number(i);
		if (1>=n)
			str[0]='\0';
		else
			sprintf(str, " (%hdx)", n);
		return str;
	}
	double GetInvWeight(const int i) const { return (NULL==inventory[i][0]) ? 0     : inventory[i][0]->Weight()*Number(i); }
	subs GetInvSub(const int i)      const { return (NULL==inventory[i][0]) ? AIR   : inventory[i][0]->Sub(); }
	kinds GetInvKind(const int i)    const { return (NULL==inventory[i][0]) ? BLOCK : inventory[i][0]->Kind(); }
	double InvWeightAll() const {
		float sum=0;
		for (unsigned short i=0; i<inventory_size; ++i)
			sum+=GetInvWeight(i)*Number(i);
		return sum;
	}
	int Number(const int i) const {
		if (inventory_size<=i)
			return 0;

		unsigned short n;
		for (n=0; n<max_stack_size && NULL!=inventory[i][n]; ++n);
		return n;
	}

	virtual char * FullName(char * const) const=0;
	virtual kinds Kind() const=0;
	virtual subs Sub() const=0;
	virtual bool Access() const=0;

	void * HasInventory() { return this; }
	usage_types Use() { return OPEN; }

	Block * Drop(int n) {
		if (0>n || inventory_size<=n) return NULL;
		unsigned short temp_n=Number(n);
		if (temp_n) {
			Block * temp=inventory[n][temp_n-1];
			inventory[n][temp_n-1]=NULL;
			return temp;
		} else
			return NULL;
	}
	virtual int Get(Block * block, int n=0) {
		if (NULL==block) return 1;
		if (0>n || inventory_size<=n) n=0;
		for (unsigned short i=n; i<inventory_size; ++i) {
			if ( NULL==inventory[i][0] ||
					(*block==*inventory[i][0] && (Number(i) < max_stack_size)) ) {
				inventory[i][Number(i)]=block;
				return 1;
			}
		}
		return 0;
	}
	void GetAll(Block * block) {
		if (NULL==block)
			return;

		Inventory * from=(Inventory *)(block->HasInventory());
		if ( NULL==from || !from->Access() )
			return;

		for (unsigned short i=0; i<inventory_size; ++i)
			while ( from->Number(i) ) {
				Block * temp=from->Drop(i);
				if ( !Get(temp) ) {
					from->Get(temp);
					return;
				}
			}
	}
	void RangeForWield(unsigned short & i, unsigned short & j) const {
		for (i=5; i<inventory_size; ++i)
			if (NULL!=inventory[i][0] &&
					(inventory[i][0]->Weapon() || inventory[i][0]->Armour())) break;
		if (i<inventory_size) {
			unsigned short t;
			for (t=i; t<inventory_size; ++t)
				if (NULL!=inventory[t][0] &&
						(inventory[t][0]->Weapon() || inventory[t][0]->Armour())) j=t;
		}
	}

	virtual void SaveAttributes(FILE * const out) const {
		fprintf(out, "\n");
		for (unsigned short i=0; i<inventory_size; ++i)
		for (unsigned short j=0; j<max_stack_size; ++j) {
			if (NULL!=inventory[i][j]) inventory[i][j]->SaveToFile(out);
			else fprintf(out, "-1\n");
		}
	}

	Inventory() {
		for (unsigned short i=0; i<inventory_size; ++i)
		for (unsigned short j=0; j<max_stack_size; ++j)
			inventory[i][j]=NULL;
	}
	Inventory(char * const str, FILE * const in) {
		for (unsigned short i=0; i<inventory_size; ++i)
		for (unsigned short j=0; j<max_stack_size; ++j)
			BlockFromFile(in, inventory[i][j]);
		fgets(str, 300, in);
	}
	~Inventory() {
		for (unsigned short i=0; i<inventory_size; ++i)
		for (unsigned short j=0; j<max_stack_size; ++j)
			delete inventory[i][j];
	}
};

class Dwarf : public Animal, public Inventory {
	Block * &onHead;
	Block * &onBody;
	Block * &onFeet;
	Block * &inRightHand;
	Block * &inLeftHand;
	unsigned short noise;

	public:
	unsigned short Noise() const { return noise; }
	bool CarvingWeapon() const {
		if ( (NULL!=inRightHand && inRightHand->Carving()) ||
		     (NULL!=inLeftHand  && inLeftHand->Carving()) ) return true;
		else return false;
	}

	virtual kinds Kind() const { return DWARF; }
	subs Sub() const { return Block::Sub(); }
	virtual char * FullName(char * const str) const {
		switch (sub) {
			default: return WriteName(str, "Dwarf");
		}
	}
	virtual char MakeSound() const { return (rand()%10) ? ' ' : 's'; }
	bool CanBeIn() const { return false; }
	double Weight() const { return InvWeightAll()+100; }

	virtual before_move_return BeforeMove(const dirs);
	int Move(const dirs);
	void Act();

	int Eat(Block * to_eat) {
		if ( NULL==to_eat )
			return 2;

		switch ( to_eat->Sub() ) {
			case HAZELNUT: satiation+=seconds_in_hour*time_steps_in_sec; break;
			case H_MEAT:   satiation+=seconds_in_hour*time_steps_in_sec*2.5; break;
			case A_MEAT:   satiation+=seconds_in_hour*time_steps_in_sec*2; break;
			default: return 0; //not ate
		}
		
		if ( seconds_in_day*time_steps_in_sec < satiation )
			satiation=1.1*seconds_in_day*time_steps_in_sec;

		return 1; //ate
	}

	void * HasInventory() { return Inventory::HasInventory(); }
	virtual bool Access() const { return false; }
	Block * Drop(int n) { return Inventory::Drop(n); }
	int Wield(Block * block) {
		if ( block->Weapon() ) {
			if (NULL==inventory[3][0]) inventory[3][0]=block;
			else if (NULL==inventory[4][0]) inventory[3][0]=block;
			return 1;
		} return 0;
	}
	virtual int Get(Block * block, int n=5) { return Inventory::Get(block, (5>n) ? 5 : n);
	}
	virtual Block * DropAfterDamage() const { return new Block(H_MEAT); }

	virtual void SaveAttributes(FILE * const out) const {
		Animal::SaveAttributes(out);
		Inventory::SaveAttributes(out);
		fprintf(out, "%hd/", noise);
	}

	float LightRadius() const { return 1.8; }

	Dwarf(World * const w, const unsigned short x, const unsigned short y, const unsigned short z) :
			Animal(w, x, y, z, H_MEAT, 100),
			onHead(inventory[0][0]), onBody(inventory[1][0]), onFeet(inventory[2][0]),
			inRightHand(inventory[3][0]), inLeftHand(inventory[4][0]),
			noise(1) {
		inventory[7][0]=new Pick(IRON);
	}
	Dwarf(World * const w, const unsigned short x, const unsigned short y, const unsigned short z, char * const str, FILE * const in) :
			Animal(w, x, y, z, str),
			Inventory(str, in),
			onHead(inventory[0][0]), onBody(inventory[1][0]), onFeet(inventory[2][0]),
			inRightHand(inventory[3][0]), inLeftHand(inventory[4][0]) {
		sscanf(str, " %hd\n", &noise);
		CleanString(str);
	}
};

class Chest : public Block, public Inventory {
	public:
	virtual kinds Kind() const { return CHEST; }
	subs Sub() const { return Block::Sub(); }
	virtual char * FullName(char * const str) const {
		switch (sub) {
			case WOOD: return WriteName(str, "Wooden Chest");
			default:
				fprintf(stderr, "Chest::FullName(char *): Chest has unknown substance: %d\n", int(sub));
				return WriteName(str, "Chest");
		}
	}
	virtual void * HasInventory() { return Inventory::HasInventory(); }
	virtual Block * Drop(const int n) { return Inventory::Drop(n); }
	int Get(Block * const block, int n=0) { return Inventory::Get(block, n); }
	virtual bool Access() const { return true; }
	double Weight() const { return InvWeightAll()+300; }

	usage_types Use() { return Inventory::Use(); }

	Block * DropAfterDamage() const { return new Chest(); }
	
	virtual void SaveAttributes(FILE * const out) const {
		Block::SaveAttributes(out);
		Inventory::SaveAttributes(out);
	}

	Chest(const subs s=WOOD, const short dur=max_durability) : Block(s, dur) {}
	Chest(char * const str, FILE * const in) :
		Block(str),
		Inventory(str, in) {}
};

class Pile : public Active, public Inventory {
	unsigned short lifetime;

	public:
	virtual kinds Kind() const { return PILE; }
	subs Sub() const { return Block::Sub(); }
	virtual char * FullName(char * const str) const { return WriteName(str, "Pile"); }

	virtual void * HasInventory() { return Inventory::HasInventory(); }
	int Get(Block * const block, int n=0) { return Inventory::Get(block, n); }
	usage_types Use() { return Inventory::Use(); }
	double Weight() const { return InvWeightAll(); }

	void Act() { if (lifetime) --lifetime; }
	bool IfToDestroy() const {
		bool empty_flag=true;
		for (unsigned short i=0; i<inventory_size; ++i)
			if ( Number(i) ) {
				empty_flag=false;
				break;
			}
		if (!lifetime || empty_flag) return true;
		else return false;
	}
	
	virtual Block * Drop(const int n) {
		Block * temp=Inventory::Drop(n);
		for (unsigned short i=0; i<max_stack_size; ++i)
			if ( Number(i) ) return temp;
		lifetime=0;
		return temp;
	}

	virtual before_move_return BeforeMove(const dirs);
	virtual bool CanBeIn() const { return false; }
	virtual bool Access()  const{ return true; }

	char MakeSound() const { return 't'; }

	virtual void SaveAttributes(FILE * const out) const {
		Active::SaveAttributes(out);
		Inventory::SaveAttributes(out);
		fprintf(out, "%hd/", lifetime);
	}

	Pile(World * const w, const unsigned short x, const unsigned short y, const unsigned short z, Block * const block=NULL) :
			Active(w, x, y, z, DIFFERENT),
			lifetime(seconds_in_day) {
		Get(block);
	}
	Pile(World * const w, const unsigned short x, const unsigned short y, const unsigned short z, char * const str, FILE * const in) :
			Active(w, x, y, z, str),
			Inventory(str, in) {
		sscanf(str, " %hd\n", &lifetime);
		CleanString(str);
	}
};

class Liquid : public Active {
	bool CheckWater(dirs) const;

	public:
	virtual int Movable() const { return ENVIRONMENT; }

	virtual kinds Kind() const { return LIQUID; }
	virtual char * FullName(char * const str) const {
		switch (sub) {
			case WATER: return WriteName(str, "Water");
			case STONE: return WriteName(str, "Lava");
			default:
				fprintf(stderr, "Liquid::FullName(char *): Liquid has unknown substance: %d\n", int(sub));
				return WriteName(str, "Unknown liquid");
		}
	}

	virtual int Transparent() const {
		switch (sub) {
			case WATER: return 1;
			default: return 0; //0 - totally invisible blocks, 1 - block is visible, but light can pass through it, 2 - invisible
		}
	}

	virtual int Damage(const unsigned short, const damage_kinds) { return durability; }

	virtual void Act();

	bool IfToDestroy() const {
		if ( !(rand()%10) &&
				!CheckWater(DOWN)  && !CheckWater(UP) &&
				!CheckWater(NORTH) && !CheckWater(SOUTH) &&
				!CheckWater(EAST)  && !CheckWater(WEST))
			return true;
		return false;
	}

	virtual int Temperature() const {
		if (WATER==sub) return 0;
		else return 1000;
	}

	void SaveAttributes(FILE * const out) const { Active::SaveAttributes(out); }

	Liquid(World * const w, const unsigned short x, const unsigned short y, const unsigned short z, const subs sub=WATER) :
			Active(w, x, y, z, sub) {}
	Liquid(World * const w, const unsigned short x, const unsigned short y, const unsigned short z, char * const str, FILE * const in) :
			Active(w, x, y, z, str) {}
};

class Grass : public Active {
	public:
	virtual char * FullName(char * const str) const {
		switch (sub) {
			case GREENERY: return WriteName(str, "Grass");
			default:
				fprintf(stderr, "Grass::FullName(char *): unlisted sub\n");
				return WriteName(str, "Unknown plant");
		}
	}
	virtual kinds Kind() const { return GRASS; }

	short Max_durability() const { return 1; } 

	virtual int Transparent() const { return 1; }
	virtual bool ShouldFall() const { return false; }

	virtual before_move_return BeforeMove(dirs) { return DESTROY; }
	virtual void Act();

	virtual void SaveAttributes(FILE * const out) const { Active::SaveAttributes(out); }

	Grass() : Active(GREENERY, 1) {}
	Grass(World * const w, const unsigned short x, const unsigned short y, const unsigned short z) :
			Active(w, x, y, z, GREENERY, 1) {}
	Grass(World * const w, const unsigned short x, const unsigned short y, const unsigned short z, char * const str, FILE * const in) :
			Active(w, x, y, z, str) {}
};

class Bush : public Active, public Inventory {
	public:
	virtual char * FullName(char * const str) const { return WriteName(str, "Bush"); }
	virtual kinds Kind() const { return BUSH; }
	subs Sub() const { return Block::Sub(); }

	virtual bool Access() const { return true; }
	usage_types Use() { return Inventory::Use(); }
	virtual void * HasInventory() { return Inventory::HasInventory(); }
	virtual int Movable() const { return NOT_MOVABLE; }
	double Weight() const { return InvWeightAll()+Block::Weight(); }

	virtual void Act() {
		if (0==rand()%seconds_in_hour) {
			Block * tempNut=new Block(HAZELNUT);
			if (!Get(tempNut) && NULL!=tempNut)
				delete tempNut;
		}
	}

	Block * DropAfterDamage() const { return new Block(WOOD); }

	virtual void SaveAttributes(FILE * const out) const {
		Active::SaveAttributes(out);
		Inventory::SaveAttributes(out);
	}

	Bush(World * const w, const unsigned short x, const unsigned short y, const unsigned short z) :
		Active(w, x, y, z, WOOD) {}
	Bush(World * const w, const unsigned short x, const unsigned short y, const unsigned short z, char * const str, FILE * const in) :
		Active(w, x, y, z, str),
		Inventory(str, in) {}
};

class Rabbit : public Animal {
	public:
	char * FullName(char * const str) const { return WriteName(str, "Rabbit"); }
	kinds Kind() const { return RABBIT; }

	void Act();
	double Weight() const { return 2; }

	int Eat(Block * to_eat) {
		if ( NULL==to_eat )
			return 2;
		if ( GREENERY==to_eat->Sub() ) {
			satiation+=seconds_in_hour*time_steps_in_sec*4;
			return 1;
		}
		return 0;
	}

	Block * DropAfterDamage() const { return new Block(A_MEAT); }

	void SaveAttributes(FILE * const out) const { Animal::SaveAttributes(out); }

	Rabbit(World * const w, const unsigned short x, const unsigned short y, const unsigned short z) :
		Animal(w, x, y, z) {}
	Rabbit(World * const w, const unsigned short x, const unsigned short y, const unsigned short z, char * str) :
		Animal(w, x, y, z, str) {}
};
#endif
