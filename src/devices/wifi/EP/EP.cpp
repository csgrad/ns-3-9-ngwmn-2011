#ifndef EP_CC
#define EP_CC

#define PROUNDS 20
#define POPSIZE 10
#define MATING_EVENTS 10
#define MAXMUT 3
//#define VERBOSE 1

#include "EP.h"
#include "epaut.h"
#include <stdio.h>
#include <cstdlib>

using namespace std;

EP::EP()
{
	int i;
	countoff = 0;

	//initalize population
	fitness = new double[POPSIZE];
	population = new epaut[POPSIZE];
	for(i=0;i<POPSIZE;i++){
		population[i].create(Nmax/2);
		fitness[i]=0.0;
	}
	inuse=population[lrand48()%POPSIZE];

	//setup
	previous = new double[PROUNDS+1];
	current = new double[PROUNDS+1];
	prevp = new int[PROUNDS];
	currp = new int[PROUNDS];
	for(i=0;i<PROUNDS;i++){
		previous[i]=0.0;
		current[i]=0.0;
		prevp[i]=0;
		currp[i]=0;
	}
	previous[PROUNDS]=0.0;
	current[PROUNDS]=0.0;
}

void EP::mutate(epaut aut){
	switch(lrand48()%5){
		case 0:aut.mutout();break;
		case 1:aut.muttrn();break;
		case 2:aut.mutfst();break;
		case 3:aut.mutadd();break;
		case 4:aut.mutdel();break;
		default:aut.mutout();aut.muttrn();aut.mutfst();break;
	}
}

/*
output = 0 if lower
         1 if rise to alpha
*/
int EP::predict(double change){
	int i,j;
	int best;
	double bestfit;
	double *temp;
	int *itemp;
	epaut child;
	double fit;
	//do some mating
	for(i=0;i<MATING_EVENTS;i++){
		//the child is created by one of the parrent objects
		child = population[lrand48()%POPSIZE];
		for(j=0;j<(lrand48()%MAXMUT)+1;j++)mutate(child);
		//evaluate fitness of the child on the previous scores
		child.reset();
		fit = 0.0;
		for(j=0;j<PROUNDS;j++){
			fit += (child.run((previous[j]>0)?1:0)==prevp[j])?previous[j+1]:-previous[j+1];
			//fit += (child.run((previous[j]>0)?1:0)==(previous[j+1])?1:0)?previous[j+1]:-previous[j+1];
		}
		int select = lrand48()%POPSIZE;
		if(fitness[select]<=fit){
			population[select]=child;
			fitness[select]=fit;
		}
	}

	countoff++;//move in though the window
	//if we are at the end of the window
	if(countoff==PROUNDS){
		best = 0; 
		bestfit = fitness[0];
		for(i=1;i<POPSIZE;i++){
			if(fitness[i]>bestfit){bestfit = fitness[i];best=i;}
		}
		//the inuse machine is that which is the best - ready it for prediction via a reset of state
		inuse = population[best];
		inuse.reset();
		//swap the current and prev change lists
		temp = previous;
		previous =  current;
		current = previous;
		//swap the current and prev prediction lists
		itemp = prevp;
		prevp = currp;
		currp = itemp;

		current[0]=previous[PROUNDS];

		for(i=0;i<POPSIZE;i++){
			population[i].reset();
			fitness[i] = 0.0;
			for(j=0;j<PROUNDS;j++){
				fitness[i] += (population[i].run((previous[j]>0)?1:0)==prevp[j])?previous[j+1]:-previous[j+1];
				//fitness[i] += (population[i].run((previous[j]>0)?1:0)==(previous[j+1])?1:0)?previous[j+1]:-previous[j+1];
			}
		}

		countoff=0;
	}
	current[countoff+1]=change; //current change is saved to the current change list
	currp[countoff] = inuse.run((change>0)?1:0); //inuse machine makes its prediction which is saved into the current predition list

	return currp[countoff]; //retrun the prediction
}

/*
int main(){
long i;
initEP();
inuse.printn(cout);
for(i<0;i<1000;i++) predict(drand48());
}*/

/*
int testmain(){
int i;
epaut test1(5);
//epaut *test2;

//test1 = new epaut(5);
test1.printn(cout);  

for(i=0;i<5;i++){
test1.mutout();
test1.printn(cout); 
test1.muttrn();
test1.printn(cout); 
test1.mutfst();
test1.printn(cout);  
}

for(i=0;i<5;i++){
test1.mutdel();
test1.printn(cout); 
}
for(i=0;i<5;i++){
test1.mutadd();
test1.printn(cout);  
}

}
*/

#endif
