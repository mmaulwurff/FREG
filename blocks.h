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
	void CleanString(char * str) {
		unsigned short i;
		for (i=0; str[i]!='/'; ++i) str[i]=' ';
		str[i]=' ';
	}
	short durability;

	public:
	short enlightened;
	virtual void FullName(char * str) {
		switch (sub) {
			case WATER:      WriteName(str, "Ice"); break;
			case STONE:      WriteName(str, "Stone"); break;
			case MOSS_STONE: WriteName(str, "Moss stone"); break;
			case NULLSTONE:  WriteName(str, "Nullstone"); break;
			case GLASS:      WriteName(str, "Glass"); break;
			case STAR: case SUN_MOON:
			case SKY:        WriteName(str, "Air"); break;
			case SOIL:       WriteName(str, "Soil"); break;
			case HAZELNUT:   WriteName(str, "Hazelnut"); break;
			case WOOD:       WriteName(str, "Wood"); break;
			case GREENERY:   WriteName(str, "Leaves"); break;
			case ROSE:       WriteName(str, "Rose"); break;
			case A_MEAT:     WriteName(str, "Animal meat"); break;
			case H_MEAT:     WriteName(str, "Not animal meat"); break;
			default:
				fprintf(stderr, "Block::FullName(char *): Block has unknown substance: %d", int(sub));
				WriteName(str, "Unknown block");
		}
	}

	virtual Block * GetThis() { return this; }
	void SetWeight(double m) { shown_weight=m; }
	void SetWeight() { shown_weight=weight; }
	double Weight() { return shown_weight; }
	dirs GetDir() { return direction; }
	void SetDir(dirs dir) { direction=dir; }
	subs Sub() const { return sub; }
	virtual void Inscribe(char * str) {
		if (NULL==note) note=new char[note_length];
		strncpy(note, str, note_length);
		if ('\n'==note[0]) {
			delete [] note;
			note=NULL;
		}
	}
	virtual void GetNote(char * str) {
		if (NULL!=note)
			strncpy(str, note, note_length);
		else str[0]='\0';
	}

	virtual kinds Kind() const { return BLOCK; }
	virtual bool CanBeIn() { return true; }
	virtual bool CanBeOut() {
		switch (sub) {
			case HAZELNUT: return false;
			default: return true;
		}
	}
	virtual int Movable() { return NOT_MOVABLE; }
	virtual int Transparent() {
		switch (sub) {
			case WATER: case GREENERY:
			case GLASS: return 1;
			default: return 0; //0 - normal block, 1 - block is visible, but light can pass through it, 2 - invisible block
		}
	}
	virtual before_move_return BeforeMove(dirs) { return NOTHING; }
	virtual void Move(dirs) {}
	virtual char MakeSound() { return ' '; }
	virtual usage_types Use() { return NO; }
	virtual int Damage() {
		switch (sub) {
			case GLASS: durability-=10; break;
			case STONE: durability-=5; break;
			case NULLSTONE: break;
			default: --durability; break;
		}
		return durability;
	}
	void Restore() { durability=max_durability; }
	virtual Block * DropAfterDamage() {
		if (BLOCK==Kind() && GLASS!=sub)
			return new Block(sub);
		return NULL;
	}
	virtual void * HasInventory() { return NULL; }
	virtual void * ActiveBlock() { return NULL; }
	virtual Block * Drop(int n) { return NULL; }

	virtual bool Armour() { return false; }
	virtual bool Weapon() { return false; }
	virtual bool Carving() { return false; }

	virtual float LightRadius() { return 0; }

	virtual int Temperature() { 
		switch (sub) {
			case WATER: return -100;
			default: return 0;
		}
	}

	virtual bool operator==(const Block& block) const {
		return ( block.Kind()==Kind() && block.Sub()==Sub() ) ? true : false;
	}

	void SaveToFile(FILE * out) {
		fprintf(out, "%d", (int)Kind());
		SaveAttributes(out);
		fprintf(out, "\n");
	}
	virtual void SaveAttributes(FILE * out) {
	       	fprintf(out, "_%d_%f_%d_%hd_%hd", sub, weight, direction, durability, enlightened);
		if (NULL!=note) fprintf(out, "_%lu/%s", strlen(note), note);
		else fprintf(out, "_0/");
	}

	Block(subs n=STONE) : sub(n), shown_weight(1), weight(1), direction(NORTH), note(NULL), durability(max_durability), enlightened(0) {}
	Block(char * str) {
		unsigned short note_len;
	       	sscanf(str, "%*d_%d_%f_%d_%hd_%hd_%hd", &sub, &weight, &direction, &durability, &enlightened, &note_len);
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
	void FullName(char * str) { WriteName(str, "Telegraph"); }

	void Inscribe(char * str) {
		Block::Inscribe(str);
		char command[note_length+40];
		if (NULL!=note) {
			strcpy(command, "echo '");
			strcat(command, note);
			strcat(command, "' | ttytter 2>&1 > /dev/null&");
			system(command);
		}
	}
	virtual Block * DropAfterDamage() { return new Telegraph(); }

	virtual void SaveAttributes(FILE * out) { Block::SaveAttributes(out); }

	Telegraph() : Block(DIFFERENT) {}
	Telegraph(char * str) : Block(str) {}
};

class Weapons : public Block {
	protected:
	public:
	virtual kinds Kind() const=0;
	bool Weapon() { return true; }
	virtual bool Carving()  { return false; }
	bool CanBeOut() { return false; }

	virtual void SaveAttributes(FILE * out) { Block::SaveAttributes(out); }

	Weapons(subs sub) : Block::Block(sub) {}
	Weapons(char * str) : Block::Block(str) {}
};

class Pick : public Weapons {
	public:
	virtual kinds Kind() const { return PICK; }
	virtual void FullName(char * str) { 
		switch (sub) {
			case IRON: WriteName(str, "Iron pick"); break;
			default:
				fprintf(stderr, "Pic::FullName(char *): Pick has unknown substance: %d\n", int(sub));
				WriteName(str, "Strange pick");
		}
	}

	virtual bool Carving() { return true; }

	virtual void SaveAttributes(FILE * out) { Weapons::SaveAttributes(out); }

	Pick(subs sub) : Weapons(sub) {}
	Pick(char * str) : Weapons(str) {}
};

class Active : public Block {
	Active * next;
	Active * prev;

	protected:
	unsigned short x_self, y_self, z_self;
	World * whereWorld;

	public:
	virtual void FullName(char * str) {
		WriteName(str, "Active block");
	}
	virtual kinds Kind() const { return ACTIVE; }

	void * ActiveBlock() { return this; }
	virtual void Move(dirs dir) {
		switch (dir) {
			case NORTH: --y_self; break;
			case SOUTH: ++y_self; break;
			case EAST:  ++x_self; break;
			case WEST:  --x_self; break;
			case UP:    ++z_self; break;
			case DOWN:  --z_self; break;
			default: fprintf(stderr, "Active::Move(dirs): unlisted dir: %d\n", (int)dir);
		}
	}

	Active * GetNext() { return next; }
	void GetSelfXYZ(unsigned short & x, unsigned short & y, unsigned short & z) {
		x=x_self;
		y=y_self;
		z=z_self;
	}
	void GetSelfXY(unsigned short & x, unsigned short & y) {
		x=x_self;
		y=y_self;
	}
	void GetSelfZ(unsigned short & z) { z=z_self; }
	World * GetWorld() { return whereWorld; }
	virtual void Act() {}
	void SafeMove();
	void SafeJump();

	virtual char MakeSound() { return ' '; }
	virtual unsigned short Noise() { return 0; }

	virtual bool IfToDestroy() { return false; }
	virtual int Movable() { return MOVABLE; }
	virtual bool ShouldFall() { return true; }

	virtual void SaveAttributes(FILE * out) { Block::SaveAttributes(out); }

	void ReloadToNorth() { y_self+=shred_width; }
	void ReloadToSouth() { y_self-=shred_width; }
	void ReloadToWest()  { x_self+=shred_width; }
	void ReloadToEast()  { x_self-=shred_width; }

	void Register(World *, int, int, int);
	void Unregister();

	Active(subs sub) :
		Block(sub),
		whereWorld(NULL) {}
	Active(World * w, unsigned short x, unsigned short y, unsigned short z, subs sub) :
			Block(sub)
		{ Register(w, x, y, z); }
	Active(World * w, unsigned short x, unsigned short y, unsigned short z, char * str) :
			Block(str)
		{ Register(w, x, y, z); }
	virtual ~Active() { Unregister(); }
};

class Animal : public Active {
	protected:
	//int health;
	public:
	virtual bool Stackable() { return false; }
	virtual void FullName(char *)=0;

	virtual void SaveAttributes(FILE * out) { Active::SaveAttributes(out); }

	Animal(World * w, unsigned short i, unsigned short j, unsigned short k, subs sub) :
		Active(w, i, j, k, sub) {}
	Animal(World * w, unsigned short i, unsigned short j, unsigned short k, char * str) :
		Active(w, i, j, k, str) {}
};

class Inventory {
	protected:
	Block * inventory[inventory_size][max_stack_size];
	public:
	void InvFullName(char * str, int i) { (NULL==inventory[i][0]) ? WriteName(str, "") : inventory[i][0]->FullName(str); }
	void NumStr(char * str, int i) {
		unsigned short n=Number(i);
		if (1>=n) strcpy(str, "");
		else sprintf(str, "(%hdx)", n);
	}
	double GetInvWeight(int i) { return (NULL==inventory[i][0]) ? 0     : inventory[i][0]->Weight()*Number(i); }
	subs GetInvSub(int i)      { return (NULL==inventory[i][0]) ? AIR   : inventory[i][0]->Sub(); }
	kinds GetInvKind(int i)    { return (NULL==inventory[i][0]) ? BLOCK : inventory[i][0]->Kind(); }
	int Number(int i) {
		if (inventory_size<=i) return 0;
		unsigned short n;
		for (n=0; n<max_stack_size && NULL!=inventory[i][n]; ++n);
		return n;
	}
	virtual void FullName(char *)=0;
	virtual kinds Kind() const=0;
	virtual bool Access()=0;

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
		if (NULL!=block) {
			Inventory * from;
			if ( NULL!=(from=(Inventory *)(block->HasInventory())) )
				for (unsigned short i=0; i<inventory_size; ++i)
					while ( from->Number(i) ) {
						Block * temp=from->Drop(i);
						if ( !Get(temp) ) {
							from->Get(temp);
							return;
						}
					}
		}
	}
	void RangeForWield(unsigned short & i, unsigned short & j) {
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

	virtual void SaveAttributes(FILE * out) {
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
	Inventory(char * str, FILE * in) {
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
	unsigned short Noise() { return noise; }
	bool CarvingWeapon() {
		if ( (NULL!=inRightHand && inRightHand->Carving()) ||
		     (NULL!=inLeftHand  && inLeftHand->Carving()) ) return true;
		else return false;
	}

	virtual kinds Kind() const { return DWARF; }
	virtual void FullName(char * str) {
		switch (sub) {
			default: WriteName(str, "Dwarf");
		}
	}
	virtual char MakeSound() { return (random()%10) ? ' ' : 's'; }
	bool CanBeIn() { return false; }

	virtual before_move_return BeforeMove(dirs);
	void Move(dirs);

	void * HasInventory() { return Inventory::HasInventory(); }
	bool Stackable() { return false; }
	virtual bool Access() { return false; }
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
	virtual Block * DropAfterDamage() { return new Block(H_MEAT); }

	virtual void SaveAttributes(FILE * out) {
		Animal::SaveAttributes(out);
		Inventory::SaveAttributes(out);
		fprintf(out, "%hd/", noise);
	}

	float LightRadius() { return 1.8; }

	Dwarf(World * w, unsigned short x, unsigned short y, unsigned short z) :
			Animal(w, x, y, z, H_MEAT),
			noise(1),
			onHead(inventory[0][0]), onBody(inventory[1][0]), onFeet(inventory[2][0]),
			inRightHand(inventory[3][0]), inLeftHand(inventory[4][0]) {
		inventory[7][0]=new Pick(IRON);
	}
	Dwarf(World * w, unsigned short x, unsigned short y, unsigned short z, char * str, FILE * in) :
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
	virtual void FullName(char * str) {
		switch (sub) {
			case WOOD: WriteName(str, "Wooden Chest"); break;
			default:
				fprintf(stderr, "Chest::FullName(char *): Chest has unknown substance: %d\n", int(sub));
				WriteName(str, "Chest");
		}
	}
	virtual void * HasInventory() { return Inventory::HasInventory(); }
	virtual Block * Drop(int n) { return Inventory::Drop(n); }
	int Get(Block * block, int n=0) { return Inventory::Get(block, n); }
	virtual Block * GetThis() { return this; }
	virtual bool Access() { return true; }

	usage_types Use() { return Inventory::Use(); }

	virtual int Damage() { return durability-=4; }
	Block * DropAfterDamage() { return new Chest(); }

	virtual void SaveAttributes(FILE * out) {
		Block::SaveAttributes(out);
		Inventory::SaveAttributes(out);
	}

	Chest(subs s=WOOD) : Block(s) {}
	Chest(char * str, FILE * in) :
		Block(str),
		Inventory(str, in) {}
};

class Pile : public Active, public Inventory {
	unsigned short lifetime;

	public:
	virtual kinds Kind() const { return PILE; }
	virtual void FullName(char * str) { WriteName(str, "Pile"); }

	virtual void * HasInventory() { return Inventory::HasInventory(); }
	int Get(Block * block, int n=0) { return Inventory::Get(block, n); }
	usage_types Use() { return Inventory::Use(); }

	void Act() { if (lifetime) --lifetime; }
	bool IfToDestroy() {
		bool empty_flag=true;
		for (unsigned short i=0; i<inventory_size; ++i)
			if ( Number(i) ) {
				empty_flag=false;
				break;
			}
		if (!lifetime || empty_flag) return true;
		else return false;
	}
	
	virtual Block * Drop(int n) {
		Block * temp=Inventory::Drop(n);
		for (unsigned short i=0; i<max_stack_size; ++i)
			if ( Number(i) ) return temp;
		lifetime=0;
		return temp;
	}
	virtual Block * GetThis() { return this; }

	virtual before_move_return BeforeMove(dirs);
	virtual bool CanBeIn() { return false; }
	virtual bool Access() { return true; }
	int Damage() { return durability-=10; }

	virtual void SaveAttributes(FILE * out) {
		Active::SaveAttributes(out);
		Inventory::SaveAttributes(out);
		fprintf(out, "%hd/", lifetime);
	}

	Pile(World * w, unsigned short x, unsigned short y, unsigned short z, Block * block=NULL) :
			Active(w, x, y, z, DIFFERENT),
			lifetime(seconds_in_day) {
		Get(block);
	}
	Pile(World * w, unsigned short x, unsigned short y, unsigned short z, char * str, FILE * in) :
			Active(w, x, y, z, str),
			Inventory(str, in) {
		sscanf(str, " %hd\n", &lifetime);
		CleanString(str);
	}
};

class Liquid : public Active {
	bool CheckWater(dirs);

	public:
	virtual int Movable() { return ENVIRONMENT; }

	virtual kinds Kind() const { return LIQUID; }
	virtual void FullName(char * str) {
		switch (sub) {
			case WATER: WriteName(str, "Water"); break;
			case STONE: WriteName(str, "Lava"); break;
			default:
				fprintf(stderr, "Liquid::FullName(char *): Liquid has unknown substance: %d\n", int(sub));
				WriteName(str, "Unknown liquid");
		}
	}

	virtual int Transparent() {
		switch (sub) {
			case WATER: return 1;
			default: return 0; //0 - totally invisible blocks, 1 - block is visible, but light can pass through it, 2 - invisible
		}
	}

	virtual int Damage() { return durability; }

	virtual void Act();

	bool IfToDestroy() {
		if ( !(random()%10) &&
				!CheckWater(DOWN)  && !CheckWater(UP) &&
				!CheckWater(NORTH) && !CheckWater(SOUTH) &&
				!CheckWater(EAST)  && !CheckWater(WEST))
			return true;
		return false;
	}

	virtual int Temperature() {
		if (WATER==sub) return 0;
		else return 1000;
	}

	void SaveAttributes(FILE * out) { Active::SaveAttributes(out); }

	Liquid(World * w, unsigned short x, unsigned short y, unsigned short z, subs sub=WATER) :
			Active(w, x, y, z, sub) {}
	Liquid(World * w, unsigned short x, unsigned short y, unsigned short z, char * str, FILE * in) :
			Active(w, x, y, z, str) {}
};

class Grass : public Active {
	public:
	virtual void FullName(char * str) {
		switch (sub) {
			case GRASS: WriteName(str, "Grass");
			default:
				fprintf(stderr, "Plant::FullName(char *): unlisted sub\n");
				WriteName(str, "Unknown plant");
		}
	}
	virtual kinds Kind() const { return GRASS; }

	virtual int Transparent() { return 1; }
	virtual bool ShouldFall() { return false; }

	virtual int Damage() { return durability=0; }

	virtual before_move_return BeforeMove(dirs) { return DESTROY; }
	virtual void Act();

	virtual void SaveAttributes(FILE * out) { Active::SaveAttributes(out); }

	Grass() : Active(GREENERY) {}
	Grass(World * w, unsigned short x, unsigned short y, unsigned short z) :
			Active(w, x, y, z, GREENERY) {}
	Grass(World * w, unsigned short x, unsigned short y, unsigned short z, char * str, FILE * in) :
			Active(w, x, y, z, str) {}
};

class Bush : public Active, public Inventory {
	public:
	virtual void FullName(char * str) { WriteName(str, "Bush"); }
	virtual kinds Kind() const { return BUSH; }

	virtual bool Access() { return true; }
	usage_types Use() { return Inventory::Use(); }
	virtual void * HasInventory() { return Inventory::HasInventory(); }
	virtual int Movable() { return NOT_MOVABLE; }

	virtual void Act() {
		if (0==random()%seconds_in_hour) {
			Block * tempNut=new Block(HAZELNUT);
			if (!Get(tempNut) && NULL!=tempNut)
				delete tempNut;
		}
	}

	Block * DropAfterDamage() { return new Block(WOOD); }
	virtual int Damage() { return durability-=4; }

	virtual void SaveAttributes(FILE * out) {
		Active::SaveAttributes(out);
		Inventory::SaveAttributes(out);
	}

	Bush(World * w, unsigned short x, unsigned short y, unsigned short z) :
		Active(w, x, y, z, GREENERY) {}
	Bush(World * w, unsigned short x, unsigned short y, unsigned short z, char * str, FILE * in) :
		Active(w, x, y, z, str),
		Inventory(str, in) {}
};

class Rabbit : public Active {
	public:
	void FullName(char * str) { WriteName(str, "Rabbit"); }
	kinds Kind() const { return RABBIT; }

	void Act();

	int Damage() { return durability-=4; }
	Block * DropAfterDamage() { return new Block(A_MEAT); }

	void SaveAttributes(FILE * out) { Active::SaveAttributes(out); }

	Rabbit(World * w, unsigned short x, unsigned short y, unsigned short z) :
		Active(w, x, y, z, A_MEAT) {}
	Rabbit(World * w, unsigned short x, unsigned short y, unsigned short z, char * str) :
		Active(w, x, y, z, str) {}
};
#endif
