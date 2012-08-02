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
	subs Sub() { return sub; }
	virtual kinds Kind() { return BLOCK; }
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
	virtual char MakeSound() { return 'b'; }
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
	virtual char MakeSound()=0;
	virtual unsigned short Noise() { return 0; }
	Active(World *, unsigned short, unsigned short, unsigned short);
	~Active();
};

class Animal : public Block, public Active {
	protected:
	int health;
	public:
	virtual int Movable() { return MOVABLE; }
	virtual bool Stackable() { return false; }
	virtual kinds Kind() { return ANIMAL; }
	virtual void FullName(char * str) {
		switch (sub) {
			default: WriteName(str, "Some animal");
		}
	}
	virtual char MakeSound() { return 'm'; }
	virtual void Move(dirs dir) { Active::Move(dir); }
	Animal(World * w, unsigned short x, unsigned short y, unsigned short z, subs s) :
		Block::Block(s), Active::Active(w, x, y, z) {}
	Animal(World * w, unsigned short x, unsigned short y, unsigned short z) :
		Block::Block(A_MEAT), Active::Active(w, x, y, z) {}
};

class Inventory {
	protected:
	struct {
		Block * block;
		unsigned short number;
	} inventory[inventory_size];
	public:
	void InvFullName(char * str, int i) { (NULL==inventory[i].block) ? WriteName(str, "") : inventory[i].block->FullName(str); }
	void NumStr(char * str, int i) {
		if (1==inventory[i].number)
			strcpy(str, "");
		else
			sprintf(str, "(%hdx) ", inventory[i].number);
	}
	double GetInvWeight(int i) { return (NULL==inventory[i].block) ? 0 : inventory[i].block->Weight()*inventory[i].number; }
	subs GetInvSub(int i)   { return (NULL==inventory[i].block) ? AIR : inventory[i].block->Sub(); }
	kinds GetInvKind(int i) { return (NULL==inventory[i].block) ? BLOCK : inventory[i].block->Kind(); }
	virtual void FullName(char *)=0;
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
	virtual kinds Kind() { return DWARF; }
	virtual void FullName(char * str) {
		switch (sub) {
			default: WriteName(str, "Dwarf");
		}
	}
	virtual char MakeSound() { return (random()%10) ? ' ' : 's'; }
	Dwarf(World * w, unsigned short x, unsigned short y, unsigned short z) :
			Animal::Animal(w, x, y, z, H_MEAT),
			onHead(inventory[0].block), onBody(inventory[1].block), onFeet(inventory[2].block),
			inRightHand(inventory[3].block), inLeftHand(inventory[4].block),
			noise(1) {
		unsigned short i;
		for (i=0; i<inventory_size; ++i) {
			inventory[i].block=new Block(STONE);
			inventory[i].number=2;
		}
	}
	~Dwarf() {
		unsigned short i;
		for (i=0; i<inventory_size; ++i)
			delete inventory[i].block;
	}
};

#endif
