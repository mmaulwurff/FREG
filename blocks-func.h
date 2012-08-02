#ifndef BLOCKS_FUNC_H
#define BLOCKS_FUNC_H

Active::Active(World * w, unsigned short x, unsigned short y, unsigned short z) :
		x_self(x), y_self(y), z_self(z), whereWorld(w), prev(NULL) {
	if (NULL==whereWorld->activeList)
		next=NULL;
	else {
		next=whereWorld->activeList;
		whereWorld->activeList->prev=this;
	}
	whereWorld->activeList=this;
}

Active::~Active() {
	if (NULL!=next)
		next->prev=prev;
	if (NULL!=prev)
		prev->next=next;
	else {
		whereWorld->activeList=next;
		if (NULL!=whereWorld->activeList)
			whereWorld->activeList->prev=NULL;
	}
}

#endif
