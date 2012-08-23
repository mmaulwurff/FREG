#ifndef BLOCKS_H
#define BLOCKS_H

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
	unsigned short durability;
	public:

	virtual Block * GetThis() { return this; }
	void SetWeight(double m) { 
		weight=shown_weight;
		shown_weight=m;
	}
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
	virtual bool CanBeOut() { return true; }
	virtual int Movable() { return NOT_MOVABLE; }
	virtual int Transparent() {
		switch (sub) {
			case WATER:
			case GLASS: return 1;
			default: return 0; //0 - totally invisible blocks, 1 - block is visible, but light can pass through it, 2 - invisible
		}
	}
	virtual void FullName(char * str) {
		switch (sub) {
			case WATER: WriteName(str, "Ice"); break;
			case STONE: WriteName(str, "Stone"); break;
			case NULLSTONE: WriteName(str, "Nullstone"); break;
			case SOIL: WriteName(str, "Soil"); break;
			case GLASS: WriteName(str, "Glass"); break;
			case STAR: case SUN_MOON:
			case SKY: WriteName(str, "Air"); break;
			default:
				fprintf(stderr, "Block::FullName(char *): Block has unknown substance: %d", int(sub));
				WriteName(str, "Unknown block");
		}
	}
	virtual void BeforeMove(dirs) {}
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
		return (durability<0) ? 0 : durability;
	}
	void Restore() { durability=max_durability; }
	virtual bool DropAfterDamage() {
		switch (sub) {
			case GLASS: return false;
			default: return true;
		}
	}
	virtual void * HasInventory() { return NULL; }
	virtual bool ActiveBlock() { return false; }
	virtual Block * Drop(int n) {}
	virtual int Get(Block *, int=0) {}

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
	       	fprintf(out, "_%d_%f_%d_%hd", sub, weight, direction, durability);
		if (NULL!=note) fprintf(out, "_%d/%s", strlen(note), note);
		else fprintf(out, "_0/");
	}

	Block(subs n) : sub(n), shown_weight(1), weight(1), direction(NORTH), note(NULL), durability(max_durability) {}
	Block(char * str) {
		unsigned short note_len;
	       	sscanf(str, "%*d_%d_%f_%d_%hd_%hd", &sub, &weight, &direction, &durability, &note_len);
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
			fprintf(stderr, command);
			system(command);
		}
	}

	virtual void SaveAttributes(FILE * out) { Block::SaveAttributes(out); }

	Telegraph() : Block::Block(DIFFERENT) {}
	Telegraph(char * str) : Block::Block(str) {}
};

class Weapons : public Block {
	protected:
	public:
	virtual kinds Kind() const=0;
	bool Weapon() { return true; }
	virtual bool Carving()  { return false; }
	bool CanBeOut() { return false; }

	virtual void SaveAttributes(FILE * out) {
		Block::SaveAttributes(out);
	}

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

class Active {
	Active * next;
	Active * prev;

	protected:
	unsigned short x_self, y_self, z_self;
	World * whereWorld;

	public:
	void Register(World *);
	void Unregister();
	virtual bool ActiveBlock() { return true; }
	virtual void Move(dirs dir) {
		switch (dir) {
			case NORTH: --y_self; break;
			case SOUTH: ++y_self; break;
			case EAST:  ++x_self; break;
			case WEST:  --x_self; break;
			case UP:    ++z_self; break;
			case DOWN:  --z_self; break;
		}
	}
	Active * GetNext() { return next; }
	void GetSelfXYZ(unsigned short & x, unsigned short & y, unsigned short & z) {
		x=x_self;
		y=y_self;
		z=z_self;
	}
	World * GetWorld() { return whereWorld; }
	virtual void Act() {}
	virtual bool IfToDestroy() { return false; }

	virtual char MakeSound() { return ' '; }
	virtual void FullName(char *)=0;
	virtual kinds Kind() const=0;
	virtual unsigned short Noise() { return 0; }

	virtual bool ShouldFall() { return true; }

	virtual void SaveAttributes(FILE * out) {}

	Active(World * w, unsigned short x, unsigned short y, unsigned short z) :
			x_self(x), y_self(y), z_self(z)
		{ Register(w); }
	virtual ~Active() { Unregister(); }
};

class Animal : public Active {
	protected:
	//int health;
	public:
	virtual int Movable() { return MOVABLE; }
	virtual bool Stackable() { return false; }
	virtual void FullName(char *)=0;

	virtual void SaveAttributes(FILE * out) { Active::SaveAttributes(out); }

	Animal(World * w, unsigned short i, unsigned short j, unsigned short k, char * str=NULL) :
		Active::Active(w, i, j, k) {}
};

class Inventory {
	protected:
	int Number(int i) {
		if (inventory_size<=i) return 0;
		unsigned short n;
		for (n=0; n<max_stack_size && NULL!=inventory[i][n]; ++n);
		return n;
	}
	public:
	Block * inventory[inventory_size][max_stack_size];
	void InvFullName(char * str, int i) { (NULL==inventory[i][0]) ? WriteName(str, "") : inventory[i][0]->FullName(str); }
	void NumStr(char * str, int i) {
		unsigned short n=Number(i);
		if (1>=n) strcpy(str, "");
		else sprintf(str, "(%hdx) ", n);
	}
	double GetInvWeight(int i) { return (NULL==inventory[i][0]) ? 0     : inventory[i][0]->Weight()*Number(i); }
	subs GetInvSub(int i)      { return (NULL==inventory[i][0]) ? AIR   : inventory[i][0]->Sub(); }
	kinds GetInvKind(int i)    { return (NULL==inventory[i][0]) ? BLOCK : inventory[i][0]->Kind(); }
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
						if ( !Get(temp) )
							from->Get(temp);
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

	void SaveAttributes(FILE * out) {
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

class Dwarf : public Block, public Animal, public Inventory {
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
	virtual int Movable() { return true; }
	bool CanBeIn() { return false; }

	inline virtual void BeforeMove(dirs);
	virtual void Move(dirs dir) { Animal::Move(dir); }

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

	void SaveAttributes(FILE * out) {
		Block::SaveAttributes(out);
		Animal::SaveAttributes(out);
		Inventory::SaveAttributes(out);
		fprintf(out, "%hd/", noise);
	}
	virtual void SaveToFile(FILE * out) {
		fprintf(out, "%d", DWARF);
		SaveAttributes(out);
		fprintf(out, "\n");
	}

	inline virtual float LightRadius();

	Dwarf(World * w, unsigned short x, unsigned short y, unsigned short z) :
			Block::Block(H_MEAT),
			Animal::Animal(w, x, y, z),
			noise(1),
			onHead(inventory[0][0]), onBody(inventory[1][0]), onFeet(inventory[2][0]),
			inRightHand(inventory[3][0]), inLeftHand(inventory[4][0]) {
		inventory[7][0]=new Pick(IRON);
	}
	Dwarf(World * w, unsigned short x, unsigned short y, unsigned short z, char * str, FILE * in) :
			Block::Block(str),
			Animal::Animal(w, x, y, z),
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

	virtual void SaveAttributes(FILE * out) {
		Block::SaveAttributes(out);
		Inventory::SaveAttributes(out);
	}

	Chest(subs s) :	Block::Block(s) {}
	Chest() : Block::Block(WOOD) {}
	Chest(char * str, FILE * in) :
		Block::Block(str),
		Inventory::Inventory(str, in) {}
};

class Pile : public Chest , public Active {
	unsigned short lifetime;

	public:
	virtual kinds Kind() const { return PILE; }
	virtual void FullName(char * str) { WriteName(str, "Pile"); }
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
		Block * temp=Chest::Drop(n);
		for (unsigned short i=0; i<max_stack_size; ++i)
			if ( Number(i) ) return temp;
		lifetime=0;
		return temp;
	}
	virtual Block * GetThis() { return this; }

	virtual int Movable() { return MOVABLE; }
	inline virtual void BeforeMove(dirs);
	virtual void Move(dirs dir) { Active::Move(dir); } 
	virtual bool CanBeIn() { return false; }
	virtual bool DropAfterDamage() { return false; }
	virtual bool Access() { return true; }
	int Damage() { return durability-=10; }

	virtual void SaveAttributes(FILE * out) {
		Chest::SaveAttributes(out);
		Active::SaveAttributes(out);
		fprintf(out, "%hd/", lifetime);
	}

	Pile(World * w, unsigned short x, unsigned short y, unsigned short z) :
			Chest(DIFFERENT),
			Active::Active(w, x, y, z),
			lifetime(seconds_in_day) {}
	Pile(World * w, unsigned short x, unsigned short y, unsigned short z, char * str, FILE * in) :
			Chest(str, in),
			Active(w, x, y, z) {
		sscanf(str, " %hd\n", &lifetime);
		CleanString(str);
	}
};

class Liquid : public Block, public Active {
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
	virtual bool DropAfterDamage() { return false; }

	inline virtual void Act();
	virtual void Move(dirs dir) { Active::Move(dir); }

	virtual int Temperature() {
		if (WATER==sub) return 0;
		else return 1000;
	}

	void SaveAttributes(FILE * out) {
		Block::SaveAttributes(out);
		Active::SaveAttributes(out);
	}

	Liquid(World * w, unsigned short x, unsigned short y, unsigned short z, subs sub) :
			Block::Block(sub),
			Active(w, x, y, z) {}
	Liquid(World * w, unsigned short x, unsigned short y, unsigned short z, char * str, FILE * in) :
			Block::Block(str),
			Active::Active(w, x, y, z) {}
};

#endif