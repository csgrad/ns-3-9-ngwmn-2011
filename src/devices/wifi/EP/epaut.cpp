#include "epaut.h"

#include <stdio.h>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <cmath>

epaut::epaut(){
   n=0;     //no states means empty automata
   states=(state *)0;  //an nil this just for safety
}

epaut::epaut(int nv){
   n=0;     //no states means empty automata
   states=(state *)0;  //an nil this just for safety
   create(nv);  //and then call the create method
}

epaut::epaut(const epaut &b){
int i,j;

  n=b.n;                     //copy the number of states
  states=new state[n];       //create an appropriate state array
  for(i=0;i<n;i++)for(j=0;j<Vstates;j++){ //looping over states
    states[i].resp[j]=b.states[i].resp[j];  //copy states and
    states[i].tran[j]=b.states[i].tran[j];  //copy transitions
  }
  fst=b.fst;
  cur=b.cur;
}


epaut::~epaut(){
  clear();
}

const epaut &epaut::operator=(const epaut &right){//overload "="

int i,j;

  if(&right != this){//for self assignment just don't do a thing
    if(n!=right.n){//fix the size, if it needs to be fixed
       delete [] states;
       n=right.n;
       states=new state[n];
    }  
    fst=right.fst;   //copy initial state
    cur=right.cur;   //copy current state
    for(i=0;i<n;i++)for(j=0;j<Vstates;j++){//loop over states and inputs
      states[i].resp[j]=right.states[i].resp[j];  //copy response
      states[i].tran[j]=right.states[i].tran[j];  //copy transition
    }
  }

  return *this;

}

void epaut::clear(){
   delete [] states;  //return the dynamically allocated state array
   n=0;               //record no states
   states=0;          //make the state array a nil pointer
}

void epaut::create(int nv){//allocate the requested number of states and
                        //call the recreate method to fill in 
                       //random transitions and responses
   blank(nv);
   recreate();

}
void epaut::recreate(){//fill in the random transition and response 
                     //values in an automata of known size.
  int i,j;
  for(i=0;i<n;i++){           //loop over states
    for(j=0;j<Vstates;j++){//loop over inputs
      states[i].resp[j]=lrand48()%VAsize;  //create response
      states[i].tran[j]=lrand48()%n;       //create transitions
    }
  }
  fst = 0;
  cur = 0;
}

void epaut::blank(int nv){//create an uninitialized automata of a given size
   if(states)clear();  //idiot check...

   n=nv;
   states=new state[nv];
}


void epaut::reset(){
   cur=fst;
}

int epaut::run(int inp){
   int val;
   val=states[cur].resp[inp];
   cur=states[cur].tran[inp];
   return(val);
}

void epaut::mutout(){
   states[lrand48()%n].resp[lrand48()%Vstates]=lrand48()%VAsize;
}

void epaut::muttrn(){
   states[lrand48()%n].tran[lrand48()%Vstates]=lrand48()%n;
}

void epaut::mutfst(){
   fst = lrand48()%n;
}

int epaut::mutadd(){
   int i,j;
   if(n>=Nmax)return 0;
   state *next = new state[n+1];
   for(i=0;i<n;i++){
	for(j=0;j<Vstates;j++){ //looping over states
   		next[i].resp[j]=states[i].resp[j];  //copy states and
   		next[i].tran[j]=states[i].tran[j];  //copy transitions
	}
   }
   next[lrand48()%n].tran[lrand48()%Vstates]=n;
   for(j=0;j<Vstates;j++){ //looping over states
   	next[n].resp[j]=lrand48()%VAsize;
   	next[n].tran[j]=lrand48()%(n);
   }
   n++;
   delete [] states;
   states = next;
   return 1;
}

int epaut::mutdel(){
   int i,j;
   int remove = lrand48()%n;
   if(n<=1){printf("HI");return 0;}
   state *next = new state[n-1];
   if(remove==fst)fst = lrand48()%(n-1);
   else if(fst>=remove)fst--;	
   for(i=0;i<remove;i++){
	for(j=0;j<Vstates;j++){ //looping over states
   		next[i].resp[j]=states[i].resp[j];  //copy states and
   		next[i].tran[j]=states[i].tran[j];  //copy transitions
		if(next[i].tran[j]==remove)next[i].tran[j]=lrand48()%(n-1);
	}
   }  
   for(i=remove;i<n-1;i++){
	for(j=0;j<Vstates;j++){ //looping over states
   		next[i].resp[j]=states[i+1].resp[j];  //copy states and
   		next[i].tran[j]=states[i+1].tran[j];  //copy transitions
		if(next[i].tran[j]==remove)next[i].tran[j]=lrand48()%(n-1);
	}
   }
   n--;
   delete[] states;
   states = next;
   return 1;
}


//automata printing routine
void epaut::printn(ostream &aus){//numerical printing routine

int i,j;
char cv[18]="LH";

  aus << "--------------" << endl;
  aus << "States:" << n << "." << endl;
  aus << "Start:" << fst;
  aus << "   ";
  for(i=0;i<Vstates;i++){
    aus << "If " << cv[i] << " ";
    if(i<Vstates-1)aus << "|"; else aus << endl;
  }
  for(i=0;i<3+6*Vstates;i++)aus << "-";
  aus << endl;
  for(i=0;i<n;i++){ 
    aus.width(2);aus << i;
    aus << ")";
    for(j=0;j<Vstates;j++){
      aus << cv[states[i].resp[j]] << "->";
      aus.width(2);aus << states[i].tran[j];
      if(j<Vstates-1)aus << "|"; else aus << endl;
    }
  }
  for(i=0;i<3+6*Vstates;i++)aus << "-";
  aus << endl;

}
