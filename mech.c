extern short view;

void map();

short mechtoggle;

void allmech() {
	mechtoggle=(mechtoggle) ? 0 : 1;
	if (view!=5) map();
}
