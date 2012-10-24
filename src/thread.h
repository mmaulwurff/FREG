#ifndef THREAD_H
#define THREAD_H

#include <QThread>
#include <world.h>

class Thread : public QThread {
	Q_OBJECT
	
	public:
		Thread(World *);
		void Stop();

	protected:
		void run();

	private:
		World * w;
		volatile bool stopped;
};

#endif
