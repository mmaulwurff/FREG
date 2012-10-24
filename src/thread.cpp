#include "thread.h"
#include "header.h"
#include <QThread>

Thread::Thread(World * world) : w(world), stopped(false) {}

void Thread::run() {
	while ( !stopped ) {
		if ( NULL!=w )
			w->PhysEvents();
		msleep(1000/time_steps_in_sec);
	}
}

void Thread::Stop() {
	stopped=true;
}
