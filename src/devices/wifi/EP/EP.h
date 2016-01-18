#ifndef EPP_HEADER
#define EPP_HEADER

#include "epaut.h"

class EP
{
	public:
		EP();
		int predict(double change);
	private:
		void mutate(epaut aut);
		epaut inuse;
		epaut *population;
		double *fitness;
		int countoff;
		double *current;
		double *previous;
		int *currp;
		int *prevp;
};

#endif
