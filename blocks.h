#ifndef BLOCKS_H
#define BLOCKS_H

class World;

class Block { //blocks without special physics and attributes
	protected:
	const subs sub;
	double weight;
	int shown_weight;
	dirs direction;
	public:

	void SetWeight(double m) { 
		weight=shown_weight;
		shown_weight=m;
	}
	void SetWeight() { shown_weight=weight; }
	double Weight() { return shown_weight; }
	dirs GetDir() { return direction; }
	void SetDir(dirs dir) { direction=dir; }
	subs Sub() const { return sub; }

	virtual kinds Kind() const { return BLOCK; }
	virtual bool Stackable() { return  true; }
	virtual int Movable() { return NOT_MOVABLE; }
	virtual int Transparent() {
		switch (sub) {
			case GLASS: return 1;
			default: return 0; //0 - totally invisible blocks, 1 - block is visible, but light can pass through it, 2 - invisible
		}
	}
	virtual void FullName(char * str) {
		switch (sub) {
			case STONE: WriteName(str, "Stone"); break;
			case NULLSTONE: WriteName(str, "Nullstone"); break;
			case SOIL: WriteName(str, "Soil"); break;
			case GLASS: WriteName(str, "Glass"); break;
			case STAR: case SUN_MOON:
			case SKY: WriteName(str, "Air"); break;
			default: WriteName(str, "Unknown block");
		}
	}
	virtual void Move(dirs) {}
	virtual char MakeSound() { return ' '; }
	virtual void Use() {}
	virtual bool HasInventory() const { return false; }
	virtual Block * Drop(int n) { fprintf(stderr, "hollo\n"); }
	virtual int Get(Block *) {}

	virtual bool operator==(const Block& block) const {
		return ( block.Kind()==Kind() && block.Sub()==Sub() ) ? true : false;
	}

	Block(subs n) : sub(n), shown_weight(1), weight(1), direction(NORTH) {}
	virtual ~Block() {}
};

class Active {
	Active * next;
	Active * prev;
	World * whereWorld;
	unsigned short x_self, y_self, z_self;

	public:
	void Move(dirs dir) {
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
	virtual unsigned short Noise() { return 0; }

	Active(World *, unsigned short, unsigned short, unsigned short);
	virtual ~Active();
};

class Animal : public Active {
	protected:
	//int health;
	public:
	virtual int Movable() { return MOVABLE; }
	virtual bool Stackable() { return false; }
	virtual void FullName(char *)=0;

	Animal(World * w, unsigned short i, unsigned short j, unsigned short k) :
		Active::Active(w, i, j, k) {}
};

class Inventory {
	protected:
	Block * inventory[inventory_size][max_stack_size];
	int Number(int i) {
		if (inventory_size<=i) return 0;
		unsigned short n;
		for (n=0; n<max_stack_size && NULL!=inventory[i][n]; ++n);
		return n;
	}
	public:
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
	virtual Block * GetThis()=0;

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
	int Get(Block * block) {
		for (unsigned short i=0; i<inventory_size; ++i)
			if ( NULL==inventory[i][0] ||
					(*block==*inventory[i][0] && (Number(i) < max_stack_size)) ) {
				inventory[i][Number(i)]=block;
				return 1;
			}
		return 0;
	}

	Inventory() {
		for (unsigned short i=0; i<inventory_size; ++i)
		for (unsigned short j=0; j<max_stack_size; ++j)
			inventory[i][j]=NULL;
		inventory[3][0]=new Block(STONE);
		inventory[3][1]=new Block(STONE);
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
	virtual kinds Kind() const { return DWARF; }
	virtual void FullName(char * str) {
		switch (sub) {
			default: WriteName(str, "Dwarf");
		}
	}
	virtual char MakeSound() { return (random()%10) ? ' ' : 's'; }
	virtual int Movable() { return true; }
	virtual void Move(dirs dir) { Animal::Move(dir); }
	bool HasInventory() const { return true; }
	Block * Drop(int n) { return Inventory::Drop(n); }
	int Get(Block * block) { return Inventory::Get(block); }
	virtual Block * GetThis() { return this; }

	Dwarf(World * w, unsigned short x, unsigned short y, unsigned short z) :
			Animal::Animal(w, x, y, z),
			Block::Block(H_MEAT),
			onHead(inventory[0][0]), onBody(inventory[1][0]), onFeet(inventory[2][0]),
			inRightHand(inventory[3][0]), inLeftHand(inventory[4][0]),
			noise(1) {}
};

class Chest : public Block, public Active, public Inventory {
	public:
	virtual kinds Kind() const { return CHEST; }
	virtual void FullName(char * str) {
		switch (sub) {
			case WOOD: WriteName(str, "Wooden Chest"); break;
			default: WriteName(str, "Chest");
		}
	}
	virtual void Use();
	bool HasInventory() const { return true; }
	virtual Block * Drop(int n) { return Inventory::Drop(n); }
	int Get(Block * block) { return Inventory::Get(block); }
	virtual Block * GetThis() { return this; }

	Chest(World * w, unsigned short x, unsigned short y, unsigned short z, subs s) :
		Active::Active(w, x, y, z), Block::Block(s) {}
	Chest(World * w, unsigned short x, unsigned short y, unsigned short z) :
		Active::Active(w, x, y, z), Block::Block(WOOD) {}
};

class Pile : public Chest {
	unsigned short lifetime;

	public:
	virtual kinds Kind() { return PILE; }
	virtual void FullName(char * str) { WriteName(str, "Pile"); }
	void Act() { if (lifetime) --lifetime; }
	bool IfToDestroy() { 
		if (lifetime) return false;
		else return true;
	}
	
	virtual Block * Drop(int n) {
		Block * temp=Chest::Drop(n);
		for (unsigned short i=0; i<max_stack_size; ++i)
			if ( Number(i) ) return temp;
		lifetime=0;
		return temp;
	}
	virtual Block * GetThis() { return this; }

	Pile(World * w, unsigned short x, unsigned short y, unsigned short z) :
		Chest(w , x, y, z, DIFFERENT), lifetime(seconds_in_day) {}
};


#endif
