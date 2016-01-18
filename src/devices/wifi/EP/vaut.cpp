/*************************************************************/
/*                                                           */
/*            Implementation for variable automata           */
/*                                                           */
/*************************************************************/
/*

                Finite state machines
                Variable size
                Dan Ashlock 12/20/00
	        Additions Joseph Alexander Brown 04/11/10

*/
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <cmath>

#include "vaut.h"

int debug_VAUT=0;

aut::aut(){//creates a completely empty automata

   n=0;     //no states means empty automata
   fpz=0;   //no fingerprint either
   states=(state *)0;  //an nil this just for safety

}

aut::aut(int nv){//calls the create method to 
                 //allocate a random n-state automata

  n=0;                //null
  fpz=0;              //the 
  states=(state *)0;  //automata
  create(nv);  //and then call the create method

}

aut::aut(const aut &b){//copy constructor

int i,j;

  n=b.n;                     //copy the number of states
  states=new state[n];       //create an appropriate state array
  inita=b.inita;             //copy the intial action
  inits=b.inits;             //copy the initial state
  for(i=0;i<n;i++)for(j=0;j<Vstates;j++){ //looping over states
    states[i].resp[j]=b.states[i].resp[j];  //copy states and
    states[i].tran[j]=b.states[i].tran[j];  //copy transitions
  }
  if(b.fpz){
    fpz=b.fpz;
    fp=new double[fpz];
    for(i=0;i<fpz;i++)fp[i]=b.fp[i];
  } else fpz=0;
}

aut::~aut(){//call the clear method to deallocate everything

  clear();

}

void aut::clearfp(){//clear the fingerprint, if any

  if(fpz){//if there is a fingerprint
    delete [] fp;  //deallocate it
    fpz=0;
  }

}

void aut::clear(){//clear the automata and reset

   delete [] states;  //return the dynamically allocated state array
   n=0;               //record no states
   states=0;          //make the state array a nil pointer
   clearfp();
}

void aut::blank(int nv){//create an uninitialized automata of a given size

int i;

   if(states)clear();  //idiot check...

   n=nv;
   states=new state[nv];
   
}

//cellular encoding creation method
void aut::ccreate(int *encoding,int el){

int ns;       //holds the current number of states in the growing machine
state *scr;   //scratch states for building the machine
int *creator; //point back to the state that created you
int curst;    //the current state 
int pinned;   //is there a pinned transition arrow?
int pinsnm;   //pinned transition number
int pintyp;   //pinned transition type  "c" or "d"
int i;        //loop index variable
int sq;       //scratch variable for squared transitions

  //don't create a memory leak when ccreating 
  if(states)clear();   //idiot check...

  ns=1;                //we start with an echo machine with one state
  curst=0;             //start pointing to the machine's sole state
  scr=new state[el];   //allocate teh state scratch space
  creator=new int[el]; //allocate the creator links
  creator[0]=0;        //the initial state is self-created
  pinned=0;            //start with no pinned transition arrows
  inita=0;             //stores current initial action
  inits=0;             //the default for cellular machines
  //initialize an echo machine
  scr[0].resp[0]=0;
  scr[0].resp[1]=1;
  scr[0].tran[0]=0;
  scr[0].tran[1]=0;
  for(i=0;i<el;i++){//loop over the cellular rules  
    switch(encoding[i]){//switch statement to pick out the rule executed
    case 0:
      if(debug_VAUT)cerr << "rule 0" << endl;
      inita=(inita+1)%2;  //increment the inital action
      break;
    case 1://increment the "c" transition
       if(debug_VAUT)cerr << "rule 1" << endl;
     scr[curst].resp[0]=(scr[curst].resp[0]+1)%2;
      break;
    case 2://increment the "d" transition
       if(debug_VAUT)cerr << "rule 2" << endl;
     scr[curst].resp[1]=(scr[curst].resp[1]+1)%2;
      break;
    case 3://move the curent state along the "c" trnsition
      if(debug_VAUT)cerr << "rule 3" << endl;
      curst=scr[curst].tran[0];
      if(pinned)scr[pinsnm].tran[pintyp]=curst;
     break;
    case 4://move the curent state along the "d" trnsition
      if(debug_VAUT)cerr << "rule 4" << endl;
      curst=scr[curst].tran[1];
      if(pinned)scr[pinsnm].tran[pintyp]=curst;
     break;
    case 5://duplicate the current state as destination of "c"
      if(debug_VAUT)cerr << "rule 5" << endl;
      scr[ns]=scr[curst];
      scr[curst].tran[0]=ns;
      creator[ns]=curst;
      ns++;
     break;
    case 6://duplicate the current state as destination of "d"
      if(debug_VAUT)cerr << "rule 6" << endl;
      scr[ns]=scr[curst];
      scr[curst].tran[1]=ns;
      creator[ns]=curst;
      ns++;
     break;
    case 7: //pin the current "c" to the current state pointer
      if(debug_VAUT)cerr << "rule 7" << endl;
      pinned=1;
      pinsnm=curst;
      pintyp=0;
      scr[curst].tran[0]=curst;
     break;
    case 8: //pin the current "d" to the current state pointer
      if(debug_VAUT)cerr << "rule 8" << endl;
      pinned=1;
      pinsnm=curst;
      pintyp=1;
      scr[curst].tran[1]=curst;
     break;
    case 9: //unpin any pinned arrow
      if(debug_VAUT)cerr << "rule 9" << endl;
      pinned=0;
     break;
    case 10: //square the "c" transition;
      if(debug_VAUT)cerr << "rule 10" << endl;
      sq=scr[curst].tran[0];
      scr[curst].tran[0]=scr[sq].tran[0];
     break;
    case 11: //square the "d" transition;
      if(debug_VAUT)cerr << "rule 11" << endl;
      sq=scr[curst].tran[1];
      scr[curst].tran[1]=scr[sq].tran[1];
     break;
    case 12: //
      if(debug_VAUT){
        cerr << "rule 12" << endl;
	cout << "CSP to " << creator[curst] << endl;
      }
      curst=creator[curst];
      if(pinned)scr[pinsnm].tran[pintyp]=curst;
     break;
    }
  }
  //transfer the machine from scratch to the class variables
  n=ns;
  states=new state[n];
  for(i=0;i<n;i++){
    states[i]=scr[i];
  }
  cur=inits;  //paranoia, I hate uninitialized variables
  //give back the dynamically allocated storage
  delete [] creator;
  delete [] scr;
  
}

void aut::create(int nv){//allocate the requested number of states and
                        //call the recreate method to fill in 
                       //random transitions and responses
int i;

   blank(nv);
   recreate();

}
void aut::recreate(){//fill in the random transition and response 
                     //values in an automata of known size.

int i,j;

  clearfp();                  //clear the fingerprint if there IS one
  inits=lrand48()%n;          //generate the initial state
  inita=lrand48()%VAsize;     //generate the initial action
  for(i=0;i<n;i++){           //loop over states
    for(j=0;j<Vstates;j++){//loop over inputs
      states[i].resp[j]=lrand48()%VAsize;  //create response
      states[i].tran[j]=lrand48()%n;       //create transitions
    }
  }
}

const aut &aut::operator=(const aut &right){//overload "="

int i,j;

  if(&right != this){//for self assignment just don't do a thing
    if(n!=right.n){//fix the size, if it needs to be fixed
       delete [] states;
       n=right.n;
       states=new state[n];
    }  
    inita=right.inita;   //copy initial action
    inits=right.inits;   //copy initial state
    for(i=0;i<n;i++)for(j=0;j<Vstates;j++){//loop over states and inputs
      states[i].resp[j]=right.states[i].resp[j];  //copy response
      states[i].tran[j]=right.states[i].tran[j];  //copy transition
    }
    if(right.fpz){//if there is a fingerprint
      clearfp();  //clear any old fingerprint
      fpz=right.fpz;  //copy the size
      fp=new double[fpz];  //allocate the space
      for(i=0;i<fpz;i++)fp[i]=right.fp[i]; //copy the numbers
    }
  }

  return *this;

}

void aut::initTFT(){//create a TFT, state streched to fill the automata

int i,j;

 if(n==0)create(1); else clearfp();
  inits=0;
  inita=0;
  for(i=0;i<n;i++){
    for(j=0;j<Vstates;j++){
      states[i].resp[j]=j%VAsize;
      states[i].tran[j]=(i+1)%n;
    }
  }
}

void aut::initPSYCO(){//create a PSYCHO, state streched to fill the automata

int i,j;

  if(n==0)create(1); else clearfp();
  inits=0;
  inita=0;
  for(i=0;i<n;i++){
    for(j=0;j<2;j++){
      states[i].resp[j]=1-j;
      states[i].tran[j]=(i+1)%n;
    }
  }
}

void aut::initAllD(){//create an AllD, state streched to fill the automata

int i,j;

  if(n==0)create(1); else clearfp();
  inits=0;
  inita=1;
  for(i=0;i<n;i++){
    for(j=0;j<Vstates;j++){
      states[i].resp[j]=1;
      states[i].tran[j]=(i+1)%n;
    }
  }
}

void aut::initAllC(){//reinitializes to stretched allC

int i,j;

  if(n==0)create(1); else clearfp();
  inits=0;
  inita=0;
  for(i=0;i<n;i++){
    for(j=0;j<Vstates;j++){
      states[i].resp[j]=0;
      states[i].tran[j]=(i+1)%n;
    }
  }

}

void aut::initPavlov(){//reinitializes to Pavlov

  clearfp();
  if(n==1)clear(); 
  if(n==0)blank(2);
  inits=0;              //start in the cooperative state
  inita=0;              //start by cooperating
  states[0].resp[0]=0;  //C state C's with C
  states[0].resp[1]=1;  //C state D's with D
  states[1].resp[0]=1;  //D state D's with C
  states[1].resp[1]=0;  //D state C's with D
  states[0].tran[0]=0;  //goto C state
  states[0].tran[1]=1;  //goto D state
  states[1].tran[0]=1;  //goto D state
  states[1].tran[1]=0;  //goto C state

}

void aut::initTF2T(){//reinitializes to tit-for-two-tats

  clearfp();
  if(n==1)clear();
  if(n==0)blank(2);
  inits=0;              //start in the cooperative state
  inita=0;              //start by cooperating
  states[0].resp[0]=0;  //C state C's with C
  states[0].resp[1]=0;  //C state C's with D
  states[1].resp[0]=0;  //D state C's with C
  states[1].resp[1]=1;  //D state D's with D
  states[0].tran[0]=0;  //C goto C state
  states[0].tran[1]=1;  //C goto D state
  states[1].tran[0]=0;  //D goto C state
  states[1].tran[1]=1;  //D goto D state

}

void aut::init2TFT(){//reinitializes to two-tits-for-tat

  clearfp();
  if(n==1)clear();
  if(n==0)blank(2);
  inits=0;              //start in the cooperative state
  inita=0;              //start by cooperating
  states[0].resp[0]=0;  //C state C's with C
  states[0].resp[1]=1;  //C state D's with D
  states[1].resp[0]=1;  //D state D's with C
  states[1].resp[1]=1;  //D state D's with D
  states[0].tran[0]=0;  //C goto C state
  states[0].tran[1]=1;  //C goto D state
  states[1].tran[0]=0;  //D goto C state
  states[1].tran[1]=1;  //D goto D state

}

void aut::initRipoff(){//reinitialize to Ripoff
 
  clearfp();
  if(n<3)clear();
  if(n==0)blank(3);
  inits=0;              //start in state 0
  inita=1;              //defecting
  states[0].resp[0]=0;  
  states[0].resp[1]=0;  
  states[1].resp[0]=1;  
  states[1].resp[1]=0;  
  states[2].resp[0]=0;  
  states[2].resp[1]=1;  
  states[0].tran[0]=1;  
  states[0].tran[1]=2;  
  states[1].tran[0]=0;  
  states[1].tran[1]=2;  
  states[2].tran[0]=2;  
  states[2].tran[1]=2;  

}

void aut::initFortress(int ns){//reinitialize to Fortress-n

  clearfp();
  if(ns>n)clear();
  if(n==0)blank(ns);
  inits=0;
  inita=1;
  for(int i=0;i<ns-1;i++){
    states[i].resp[0]=1;
    states[i].resp[1]=1;
    states[i].tran[0]=0;
    states[i].tran[1]=i+1;
  }
  states[ns-2].resp[1]=0;
  states[ns-1].resp[0]=0;
  states[ns-1].resp[1]=1;
  states[ns-1].tran[0]=ns-1;
  states[ns-1].tran[1]=0;

}

void aut::initAppoxMajority(int k){
  int ns = 2*k+1;
  clearfp();
  if(ns>n)clear();
  if(n==0)blank(ns);
  inits = 0;
  inita = 0;
  

}

void aut::initTrifecta(){
  clearfp();
  if(n<3)clear();
  if(n==0)blank(3);
  inits=0;              //start in state 0
  inita=1;              //defecting
  states[0].resp[0]=0;  
  states[0].resp[1]=1;  
  states[1].resp[0]=0;  
  states[1].resp[1]=1;  
  states[2].resp[0]=1;  
  states[2].resp[1]=0;  
  states[0].tran[0]=2;  
  states[0].tran[1]=1;  
  states[1].tran[0]=1;  
  states[1].tran[1]=0;  
  states[2].tran[0]=2;  
  states[2].tran[1]=1;  
}

void aut::initTrifectaM(){
  clearfp();
  if(n<3)clear();
  if(n==0)blank(3);
  inits=0;              //start in state 0
  inita=1;              //defecting
  states[0].resp[0]=0;  
  states[0].resp[1]=1;  
  states[1].resp[0]=0;  
  states[1].resp[1]=1;  
  states[2].resp[0]=1;  
  states[2].resp[1]=0;  
  states[0].tran[0]=2;  
  states[0].tran[1]=1;  
  states[1].tran[0]=1;  
  states[1].tran[1]=1;  
  states[2].tran[0]=2;  
  states[2].tran[1]=1;  
}

int aut::size(){//return the size (number of states) of the auomata

   return(n);

}

//this routine returns the fingerprint size.  If there IS no fingerprint
//then that size is zero and so the routine is also a predicate for the
//presence of a fingerprint
int aut::print(){//returns fingerprint size (0=none)

  return(fpz);

}

//the tolerance is explained in the fingerprintTFT routine, it is
//a confidence interval for the fingerprint
void aut::makeFP25(double tol){//create the 25-point fingerprint

int i,j,k;
double x,y;

  if(fpz){//make sure there is a fingerprint buffer of size 25
    if(fpz!=25){
      clearfp();
      fpz=25;
      fp=new double[fpz];
    }
  } else {
    fpz=25;
    fp=new double[fpz];
  }
  
  //repeatedly call the score routine to make the fingerprint
  k=0;
  x=1.0/6.0;
  for(i=1;i<6;i++){
    y=1.0/6.0;
    for(j=1;j<6;j++){
      fp[k++]=fingerprintTFT(x,y,tol);
      y+=1.0/6.0;  
    }
    x+=1.0/6.0;
  }
}

void aut::makeFP25(){//create the 25-point fingerprint

double x,y;
int **mark;      //holds the Markov chain state description
int *vis;        //state visited?
int i,j,k,q,m;   //loop index variables
int first;       //controls printing of +
double *sand;    //the probability vector
double *move;    //and its scratch counterpart
double *sw;      //fast copy hack pointer

  mark=new int*[4*n];                     //alocate the
  for(i=0;i<4*n;i++)mark[i]=new int[4];   //state space
  vis=new int[4*n];                       //visitation array
                                          //where-what where-what

  sand=new double[4*n];                   //allocate probability
  move=new double[4*n];                   //calculation space

  for(i=0;i<4*n;i++)vis[i]=0;             //initialize visitation
  
  //Create states init,JAC,inita
  MarkFall(mark,vis,4*inits+inita);
                //init,JAD,inita
  MarkFall(mark,vis,4*inits+2+inita);

  if(fpz){//make sure there is a fingerprint buffer of size 25
    if(fpz!=25){
      clearfp();
      fpz=25;
      fp=new double[fpz];
    }
  } else {
    fpz=25;
    fp=new double[fpz];
  }
  
  //repeatedly call the score routine to make the fingerprint
  k=0;
  x=1.0/6.0;
  for(i=1;i<6;i++){
    y=1.0/6.0;
    for(j=1;j<6;j++){
      //the Markov based scoring goes here
      for(q=0;q<4*n;q++)move[q]=sand[q]=0.0;  //zero prob registers
      sand[4*inits+inita]=0.5;   //place the probability on
      sand[4*inits+2+inita]=0.5; //the initial states of the Markov chain
      fp[k]=0.0;                 //zero the fingerprint value
      for(m=0;m<50+50*n;m++){
        for(q=0;q<4*n;q++)if(vis[q]){
          switch(mark[q][1]){
          case 0: move[mark[q][0]]+=sand[q]*x;
   	          move[mark[q][2]]+=sand[q]*(1-x);
		  break;
          case 1: move[mark[q][0]]+=sand[q]*(1-x);
   	          move[mark[q][2]]+=sand[q]*x;
		  break;
          case 2: move[mark[q][0]]+=sand[q]*y;
   	          move[mark[q][2]]+=sand[q]*(1-y);
		  break;
          case 3: move[mark[q][0]]+=sand[q]*(1-y);
   	          move[mark[q][2]]+=sand[q]*y;
		  break;
	  }
	}
        sw=sand;sand=move;move=sw;
        for(q=0;q<4*n;q++)move[q]=0.0;
      }
      //at this point the probability vector is okay
      for(q=0;q<4*n;q+=4){
        //amalgamate the scores - [q+2] is DC and so adds zero
        fp[k]+=3.0*sand[q]+5.0*sand[q+1]+sand[q+3];
      }
      k++;
      y+=1.0/6.0;  
    }
    x+=1.0/6.0;
  }
 
  delete [] sand;
  delete [] move;
  delete [] vis;
  for(i=0;i<4*n;i++)delete [] mark[i];
  delete [] mark;
}

void aut::tellfp(double *pb){//returns the fingerprint values

  for(int i=0;i<fpz;i++)pb[i]=fp[i];

}

void aut::sps(char *s){//returns the self play string

int *S;
int *A;
int i,j,k,flg,stop;
char cv[18]="CDABEFGHIJKLMNOPQ";

  S=new int[n*Vstates+2];
  A=new int[n*Vstates+2];
  i=0;
  S[i]=inits;
  A[i]=inita;
  flg=1;
  while(flg){
    i++;
    A[i]=states[S[i-1]].resp[A[i-1]];
    S[i]=states[S[i-1]].tran[A[i-1]];
    for(j=0;(j<i)&&flg;j++)if((A[i]==A[j])&&(S[i]==S[j])){
      flg=0;
      stop=j;
    }
  }
  for(k=0;k<stop;k++)s[k]=cv[A[k]];
  s[k++]=':';
  for(k=stop;k<i;k++)s[k+1]=cv[A[k]];
  s[k+1]='\0';
  delete [] A;
  delete [] S;

}

int aut::nas(){//reports states reachable from initial state

  if(n==0)return(0);

int *Q,*vis;
int i,h,t,cnt,nxt;

  Q=new int[2*n];
  vis=new int[n];
  cnt=0;
  Q[0]=inits;
  h=0;t=1;
  for(i=0;i<n;i++)vis[i]=0;
  while(h<t){
    if(!vis[Q[h]]){
      vis[Q[h]]=1;
      cnt++;
      nxt=states[Q[h]].tran[0];
      if(!vis[nxt])Q[t++]=nxt;
      nxt=states[Q[h]].tran[1];
      if(!vis[nxt])Q[t++]=nxt;
    }
    h++;      
  }
  delete [] vis;
  delete [] Q;
  return(cnt);
}

void aut::reachfall(int *row, int curs){//recursive helper for ncc 

int dx;

  for(int i=0;i<Vstates;i++){
    dx=states[curs].tran[i];
    if(!row[dx]){
      row[dx]=1;
      reachfall(row,dx);  
    }
  }
}

int aut::ncc(){//number of communicating classes

int **adj;  //reachability matrix for states
int *vis;
int i,j,l,k;  //loop index varaiables

  if(n<2)return(1);  //failsafe

  //allocate and initialize the reachability matrix and visitation vector
  vis=new int[n];
  adj=new int*[n];
  for(i=0;i<n;i++){
    adj[i]=new int[n];
    for(j=0;j<n;j++)adj[i][j]=0;
  }

  //fill the reachability matrix
  for(i=0;i<n;i++)reachfall(adj[i],i);
  for(i=0;i<n;i++){
    for(j=0;j<n;j++)if((adj[i][j]==0)&&(adj[j][i]==1))adj[j][i]=0;
  }

  //initialize the visitation vector
  for(i=0;i<n;i++)vis[i]=0;
  k=0; //initialize the number of communicating classes

  //mark the CC's
  for(i=0;i<n;i++)for(j=0;j<n;j++)if(adj[i][j]&&(!vis[i])){//found a class
    for(l=0;l<n;l++)if(adj[i][l])vis[l]=1;
    k++;
  }

  //for(i=0;i<n;i++){
  //  for(j=0;j<n;j++)cout << " " << adj[i][j];
  //  cout << endl;
  //}
  
  //deallocate the reachability matrix and visiitation vector
  for(i=0;i<n;i++)delete [] adj[i];
  delete [] adj;
  delete [] vis;

  return(k);

}

/* Simplification Methods */
void aut::ReduceACC(aut &other){//reduce to states accessible 
                                //from the initial state only

int *Q,*vis;        //a queue for tracking accesibility, a visited predicate
int i,j,k,h,t,cnt,nxt;  
int *transtab;      //the translation table
int *backtab;       //backward translation table 

  //create the tracking queue
  Q=new int[2*other.n];
  //create the "visited predicate
  vis=new int[other.n];
  //create the translation and back translation table
  transtab=new int[other.n];
  backtab=new int[other.n];
  //no states counted yet
  cnt=0;
  //initialize the queue with the initial state
  Q[0]=other.inits;
  //initialize head and tail+1 of the queue
  h=0;t=1;
  //mark all states as unvisited
  for(i=0;i<other.n;i++)vis[i]=0;

  while(h<t){//as long as there is anything in the queue

    if(!vis[Q[h]]){//skip states you've visited already

      transtab[cnt]=Q[h];  //record the translation table entry
      backtab[Q[h]]=cnt;   //and its local inverse
      //cout << cnt << "-->" << Q[h] << endl;
      vis[Q[h]]=1;//mark the state as visited
      cnt++;      //increment the number of accissible states
      //place transitions out of the current state into the queue
      //if they have not yet been visited
      nxt=other.states[Q[h]].tran[0];
      if(!vis[nxt])Q[t++]=nxt;
      nxt=other.states[Q[h]].tran[1];
      if(!vis[nxt])Q[t++]=nxt;
    }
    h++; //advance the queue head
  }
  //we've got the translation table, kill off the other dynamic storage
  delete [] vis;
  delete [] Q;

  //don't create a memory leak when creating the reduced machine 
  if(states)clear();   //idiot check...

  //we have cnt states in the reduced machine
  n=cnt;
  states=new state[n];  //allocate states
  inita=other.inita;    //save the initial action
  inits=0;              //we renumbered, initial state is zero
  for(i=0;i<n;i++)for(j=0;j<Vstates;j++){//copy the reduced states
    k=transtab[i];  //find the state i is copied from
    //cout << "k=" << k << endl;
    states[i].resp[j]=other.states[k].resp[j];           //trans-
    states[i].tran[j]=backtab[other.states[k].tran[j]]; //late
  } 
  delete [] transtab;  //give back the storage for the translation table
  delete [] backtab;   //and the back translation table
  //DONE!
}

int aut::SDL(){//State depth length 

int q=2;

  for(int i=0;i<n;i++)q*=2;
  return(q-1);

}

void aut::MarkFall(int **st,    //Recursively derive Markov Equations
                   int *vis,
		   int who
		   ){

  if(vis[who])return;//been here, eh?

int cs=who/4;    //compute the current state
int J,M;         //joss-ann and machine actions

  J=(who%4)/2;   //find the Joss-Anne's current action  bit one
  M=(who%2);     //find the machines current action     bit zero

  //cout << cs << " " << J << " " << M << endl;

  //compute the Markov transitions


  //If JA cooperates:
  st[who][0]=4*states[cs].tran[J]+0+states[cs].resp[J];  //where
  if(M)st[who][1]=0; else st[who][1]=3;  //what
  //irrational a             rational 1-b

  //If JA defects
  st[who][2]=4*states[cs].tran[J]+2+states[cs].resp[J];   //where 
  if(M)st[who][3]=1; else st[who][3]=2;  //what
  //rational b             irrational 1-a

  vis[who]=1;    //mark the state visited

  //fall in the cooperative direction
  MarkFall(st,vis,st[who][0]);
  //fall in the non-cooperative direction
  MarkFall(st,vis,st[who][2]);

} 

void aut::MarkovE(ostream &aus){//print out the Markov Equations

int **mark;  //holds the Markov chain state description
int *vis;    //state visited?
int i,j;     //loop index variables
int first;   //controls printing of +

char *TR="CD";  //translation array

  mark=new int*[4*n];                     //alocate the
  for(i=0;i<4*n;i++)mark[i]=new int[4];   //state space
  vis=new int[4*n];                       //visitation array
                                          //where-what where-what

  for(i=0;i<4*n;i++)vis[i]=0;             //initialize visitation
  
  //Create states init,JAC,inita
  MarkFall(mark,vis,4*inits+inita);
                //init,JAD,inita
  MarkFall(mark,vis,4*inits+2+inita);

  /* Matrix Printer 
  for(i=0;i<4*n;i++)if(vis[i]){
    cout << TR[(i%4)/2] << TR[(i%2)] << i/4 << " : ";
    for(j=0;j<4*n;j++)if(vis[j]){
      if(mark[i][0]==j)switch(mark[i][1]){
      case 0: aus << "  a ";
	break;
      case 1: aus << " 1-a";
	break;
      case 2: aus << "  b ";
	break;
      case 3: aus << " 1-b";
	break;
      } else if(mark[i][2]==j)switch(mark[i][3]){
      case 0: aus << "  a ";
	break;
      case 1: aus << " 1-a";
	break;
      case 2: aus << "  b ";
	break;
      case 3: aus << " 1-b";
	break;
      } else aus << "  0 ";
    }
    aus << endl;
  }
  */
  /* Equation printer */
  cout << "JA:self:state" << endl;
  for(j=0;j<4*n;j++)if(vis[j]){
    first=1;
    aus << TR[(j%4)/2] << TR[j%2] << j/4 << "="; 
    for(i=0;i<4*n;i++)if(vis[i]){
      if(mark[i][0]==j){
        if(first)first=0; else aus << "+";
        switch(mark[i][1]){
        case 0: aus << "a";
	  break;
        case 1: aus << "(1-a)";
  	  break;
        case 2: aus << "b";
	  break;
        case 3: aus << "(1-b)";
	  break;
	}
        aus << TR[(i%4)/2] << TR[i%2] << i/4; 
      }
      if(mark[i][2]==j){
        if(first)first=0; else aus << "+";
        switch(mark[i][3]){
        case 0: aus << "a";
	  break;
        case 1: aus << "(1-a)";
	  break;
        case 2: aus << "b";
	  break;
        case 3: aus << "(1-b)";
	  break;
	}
        aus << TR[(i%4)/2] << TR[i%2] << i/4; 
      }
    }
    if(first)aus << "0";
    aus << endl;
  }

  delete [] vis;
  for(i=0;i<4*n;i++)delete [] mark[i];
  delete [] mark;

}

//estimate the value of the tit-for-tat fingerprint of the machine 
//at point alpha beta with given tolerance.  The tolerance tol is
//the the width of an acceptable confidence interval on the value
//of the fingerprint
double aut::fingerprintTFT(double a,double b,double tol){

const int L=1000;     //this is an internal control for the fingerprint 
                      //assessment, it is the minimum number of rounds of 
                      //play used in assesment even if the tolerance is
                      //acceptable.

const int resetv=150; //since an automata can have different states used 
                      //based on its first few initial plays we reset the
                      //autoamta periodically.  This length 150 is the 
                      //number of plays used in evolution in our current
                      //research.

//this is the score matric used in fingerprinting.  There are default 
//values which we are using in our research, put in your own if you
//want to.
const double sc[2][2]={{3.0,0.0},{5.0,1.0}}; //C=0, D=1  score[you][him]

int act,fin,hold,i;
double r,buf,hr,sq,sg,s,mu,divisor,halt;

  sg=buf=0.0;  //zero tracking variables
  reset();
  if(a+b<=1){//we are in the standard part of the fingerprint
    act=fin=0;
    i=0;
    halt=1;
    while((i<L)||(halt>tol)){
      i++;
      hold=run(fin);
      r=drand48();
      r-=a;
      if(r<0)fin=0; else {
        r-=b;
        if(r<0)fin=1; else fin=act;
      }
      act=hold;
      s=sc[act][fin];
      buf+=s;
      sg+=s*s;
      if(i%L==0){
        divisor=((double)i);
        mu=buf/divisor;
        sq=sqrt(sg/divisor-mu*mu);
        halt=1.96*sq/sqrt(divisor);
      }
      if(L%resetv==0)reset();
    }
  } else {//we are in the extended part of the fingerprint
    act=fin=0; 
    hr=1-b;
    b=1-a;
    a=hr;
    i=0;
    halt=1;
    while((i<L)||(halt>tol)){
      i++;
      hold=run(fin);
      r=drand48();
      r-=a;
      if(r<0)fin=0; else {
        r-=b;
        if(r<0)fin=1; else fin=1-act;
      }
      act=hold;
      s=sc[act][fin];
      buf+=s;
      sg+=s*s;
      if(i%L==0){
        divisor=((double)i);
        mu=buf/divisor;
        sq=sqrt(sg/divisor-mu*mu);
        halt=1.96*sq/sqrt(divisor);
      }
      if(L%resetv==0)reset();
   }
  }
  return(mu);

}

void aut::FallSDL(int *t,         //signature array
                  int val,        //current node value
                  int inc,        //current increment value
                  int cs){        //current state

int ns; //next state
int niv; //new increment value
int nv; //new node value

  //cout << val << " ";
  niv=inc/2;
  nv=val+1;
  t[nv]=states[cs].resp[0];
  if(inc>1)FallSDL(t,nv,niv,states[cs].tran[0]);
  nv=val+inc;
  t[nv]=states[cs].resp[1];
  if(inc>1)FallSDL(t,nv,niv,states[cs].tran[1]);  

}

void aut::FindSDL(int starter,    //Given a starting state, find the (flat) 
                  int *signature){//tree of responses in signature

int z;      //size of signature array
int value;  //value of current node

  z=SDL();
  value=0;
  signature[0]=-1;
  //cout << z << " " << (z+1)/2 << endl;
  FallSDL(signature,value,(z+1)/2,starter);

}

//ER has as many entries as the states of the machine.  It returns the 
//number of equivalence classes of states and the class for each state 
//in ER.  The classes are numbered 1..k where k+1 is the return value.
int aut::StateEQ(int *ER){//give the equivalence relations on the states

int **ssr;      //state space register
int depth;      //entries in a flat signature vector
int i,j,k,l;    //loop indices

  //compute the size of the scratch space and allocate it
  ssr=new int*[n];
  depth=SDL();
  //cerr << "Depth=" << depth-1 << endl;
  for(i=0;i<n;i++)ssr[i]=new int[depth+1];

  //get the equivalence signatures
  for(i=0;i<n;i++){
    FindSDL(i,ssr[i]);
    //for(j=1;j<=depth;j++)cout << ssr[i][j];
    //cout << endl;
  }
  //initialize the equivalence relation to "unclassified"
  for(i=0;i<n;i++)ER[i]=n;
  //form the ER
  j=0;  //next available equivalence class
  for(i=0;i<n;i++){
    if(ER[i]==n){//new equivalent class
      ER[i]=j++;//assign the leading member of the new class
      for(k=i+1;k<n;k++)if(ER[k]==n){//check the other unassigned states
        //this loop terminates with l==depth IF states i,k are equivalent
        for(l=1;(l<depth)&&(ssr[i][l]==ssr[k][l]);l++);
        if(l==depth)ER[k]=ER[i];
      }
    }//if a state is assigned to a class already there 
     //is no need to worry
  }//all the classes should be assigned at this point
  
  //cerr << "About to deallocate" << endl;

  //deallocate the scratch space
  //cerr << "Dealocating signature rows..." << endl; 
  for(i=0;i<n;i++){
    //cerr << i << endl;
    delete [] (ssr[i]);
  }
  //cerr << "Dealocating signature frame." << endl;
  delete [] ssr;

  //cerr << "About to return " << j << endl;

  return(j);

}

//ReduceEQ creates a new automaton by reducing modulo the kind of 
//equivalence relation computed by StateEQ
void aut::ReduceEQ(aut &other){//create a reduced version of the automaton

int *ER;
int i,j,k;                  //loop index variables
int nn;


  //don't create a memory leak when ccreating 
  if(states)clear();   //idiot check...

  // int ER[other.n];            //create the equivalence relation
  ER = new int[other.n];
  nn=other.StateEQ(ER);   //get the equivalence relation

  //Notice that the equivalence relation is a translation table for 
  //the state numbers
  n=nn;
  states=new state[n];
  inita=other.inita;
  inits=ER[other.inits];
  for(i=0;i<n;i++)for(j=0;j<Vstates;j++){//copy the reduced states
    //find the first member of the equivalence class
    k=0;
    while(ER[k]!=i)k++;
    //found it.  Now copy and translate
    states[i].resp[j]=other.states[k].resp[j];
    states[i].tran[j]=ER[other.states[k].tran[j]];
  } 
  //DONE!

  delete[] ER;

} 

int aut::reset(){//reset the automata, return initial action

   cur=inits;
   return(inita);

}

int aut::reset(int &wiis){//returns initial action and state

   wiis=inits;
   return(inita);

}

int aut::run(int inp){//run on input inp, update internal state and 
                      //return your response
int val;

   val=states[cur].resp[inp];
   cur=states[cur].tran[inp];
   return(val);   

}

int aut::run(int inp,int &stt){ //run on input inp with given state stt

int val;

   val=states[stt].resp[inp];
   stt=states[stt].tran[inp];
   return(val);   

}

//these routines assume the fingerprints have the same length
double aut::Dsqr(aut &other){//squared fingerprint distance

double accu,delta;

  accu=0.0;
  for(int i=0;i<fpz;i++){
    delta=fp[i]-other.fp[i];
    accu+=(delta*delta);
  }
  return(accu);
}

double aut::D(aut &other){//fingerprint distance

  return(sqrt(Dsqr(other)));
  
} 

//genetic operations, other than two point crossover
    
void aut::mutateaction(){

  clearfp();
  states[lrand48()%n].resp[lrand48()%Vstates]=lrand48()%VAsize;

}
    
void aut::mutatetransition(){

  clearfp();
  states[lrand48()%n].tran[lrand48()%Vstates]=lrand48()%n;

}

void aut::copystatemutation(){

  clearfp();
  states[lrand48()%n]=states[lrand48()%n];

}

void aut::initialmutation(){

  clearfp();
  if(lrand48()%2)inits=lrand48()%n; else inita=lrand48()%VAsize;

}

//automata printing routine
void aut::printn(ostream &aus){//numerical printing routine

int i,j;
char cv[18]="0123456789";

  aus << "--------------" << endl;
  aus << "States:" << n << "." << endl;
  aus << "Start:" << cv[inita] << "->" << inits << endl;
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

void aut::printFP(ostream &aus){//with fingerprint printing routine

int i,j;
char cv[18]="CDABEFGHIJKLMNOPQ";

  aus << "--------------" << endl;
  aus << "States:" << n << "." << endl;
  aus << "Start:" << cv[inita] << "->" << inits << endl;
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
  aus << fpz;
  for(i=0;i<fpz;i++)aus << " " << fp[i];
  aus << endl;
  for(i=0;i<3+6*Vstates;i++)aus << "-";
  aus << endl;

}

void aut::readFP(istream &inp){//input with fingerprint

int i,j,nv;
char buf[500];

  inp.getline(buf,499);
  inp.getline(buf,499);
  i=0;
  while((!isdigit(buf[i]))&&(i<strlen(buf)))i++;
  if(i>=strlen(buf)){
    return; //hopeless, give up
  }
  nv=atoi(buf+i);
  create(nv);
  inp.getline(buf,499);
  i=0;
  while((buf[i]!=':')&&(i<strlen(buf)))i++;
  if(buf[i]!=':')return; //give up
  i++;
  if(buf[i]=='C')inita=0; else inita=1;
  i+=3;
  inits=atoi(buf+i);
  inp.getline(buf,499);
  inp.getline(buf,499);
  for(j=0;j<nv;j++){
    inp.getline(buf,499);
    i=0;
    while(buf[i]!=')')i++;
    i++;
    if(buf[i]=='C')states[j].resp[0]=0; 
              else states[j].resp[0]=1;
    i+=3;
    states[j].tran[0]=atoi(buf+i);
    while(buf[i]!='|')i++;
    i++;
    if(buf[i]=='C')states[j].resp[1]=0; 
              else states[j].resp[1]=1;
    i+=3;
    states[j].tran[1]=atoi(buf+i);
  }
  inp.getline(buf,499);  //eat the next seperator
  inp.getline(buf,499);  //this line contains the fingerprint
  //read the number of entries in the print
  fpz=atoi(buf);
  //allocate
  fp=new double[fpz];
  //parse the values
  j=0;
  for(i=0;i<fpz;i++){
    while(buf[j]!=' ')j++;
    while(!isdigit(buf[j]))j++;
    fp[i]=atof(buf+j);
  }  
  //fingerprint is in
  inp.getline(buf,499);  //eat the final seperator
 
}

//fitness funtions from chapter 6 on EXOM
//assorted fitness functions
int aut::predict(int *ref,int bits){

int s,a,c,i,z;

  s=inits;
  a=inita;
  c=0;
  for(i=0;i<bits;i++){
    //cout << a << " " << ref[i] << endl;
    if(a==ref[i])c++;
    z=states[s].tran[ref[i]];
    a=states[s].resp[ref[i]];
    s=z;
  }
  return(c);
}


//The self driving length is the number of states and transitions
//used by a machine driven by its own output until it hits a repeated
//state-output pair
int aut::selfdri(){//the self driving length fitness funciton

int *used;
int s,a,c,z,i;

   used=new int[VAsize*n];  //create a state tracking zone

   for(i=0;i<VAsize*n;i++)used[i]=0;
   s=inits;
   a=inita;
   c=0;
   i=VAsize*s+a;
   while(!used[i]){ 
     //cout << s << " " << a << endl;
      c++;   
      used[i]=1;
      z=states[s].tran[a];
      a=states[s].resp[a];
      s=z;
      i=VAsize*s+a;
   }
   
   delete [] used;     //get rid of the state tracking zone

   return(c);

}

//automata printing routine
ostream &operator<<(ostream & aus,aut & m){

int i,j;
char cv[18]="CDABEFGHIJKLMNOPQ";

  aus << "--------------" << endl;
  aus << "States:" << m.n << "." << endl;
  aus << "Start:" << cv[m.inita] << "->" << m.inits << endl;
  aus << "   ";

  for(i=0;i<Vstates;i++){
    aus << "If " << cv[i] << " ";
    if(i<Vstates-1)aus << "|"; else aus << endl;
  }

  for(i=0;i<3+6*Vstates;i++)aus << "-";
  aus << endl;
  for(i=0;i<m.n;i++){ 
    aus.width(2);aus << i;
    aus << ")";
    for(j=0;j<Vstates;j++){
      aus << cv[m.states[i].resp[j]] << "->";
      aus.width(2);aus << m.states[i].tran[j];
      if(j<Vstates-1)aus << "|"; else aus << endl;
    }
  }

  for(i=0;i<3+6*Vstates;i++)aus << "-";
  aus << endl;
  return(aus);

}

//automata reading routine, only good for binary automata so far,
//in the PD metaphore
istream &operator>>(istream & aus,aut & m){

int i,j,nv;
char inp[256];

   aus.getline(inp,255);
   //cerr << inp << endl;
   aus.getline(inp,255);
   //cerr << inp << endl;
   i=0;
   while((!isdigit(inp[i]))&&(i<strlen(inp)))i++;
   if(i>=strlen(inp)){
     return(aus); //hopeless, give up
   }
   nv=atoi(inp+i);
   //cerr << nv << endl;
   m.create(nv);
   aus.getline(inp,255);
   i=0;
   while((inp[i]!=':')&&(i<strlen(inp)))i++;
   if(inp[i]!=':')return(aus); //give up
   i++;
   if(inp[i]=='C')m.inita=0; else m.inita=1;
   i+=3;
   m.inits=atoi(inp+i);
   aus.getline(inp,255);
   aus.getline(inp,255);
   for(j=0;j<nv;j++){
     aus.getline(inp,255);
     i=0;
     while(inp[i]!=')')i++;
     i++;
     if(inp[i]=='C')m.states[j].resp[0]=0; 
               else m.states[j].resp[0]=1;
     i+=3;
     m.states[j].tran[0]=atoi(inp+i);
     while(inp[i]!='|')i++;
     i++;
     if(inp[i]=='C')m.states[j].resp[1]=0; 
               else m.states[j].resp[1]=1;
     i+=3;
     m.states[j].tran[1]=atoi(inp+i);
   }
   aus.getline(inp,255);
   return(aus);
}

//two point crossover
void tpc(aut &a, aut &b){

int i,cp1,cp2,z,sw;
state sws;
   
  a.clearfp(); 
  z=a.n;              //check size for crossover
  if(b.n<z)z=b.n;     //taking the smaller machine length
  cp1=lrand48()%z;    //choose crossover points
  cp2=lrand48()%z;    //and then
  if(cp1>cp2){//put them in order
    sw=cp1;
    cp1=cp2;
    cp2=sw;
  }
  for(i=cp1;i<cp2;i++){//exchange the states from the first crossover
    sws=a.states[i];  //point to just before the second
    a.states[i]=b.states[i];
    b.states[i]=sws;
  }
}

void copy(aut &a,aut &b){  //copy, with allocation if needed

int i,j;

   if(a.states){
     if(a.n!=b.n){
       delete [] a.states;
       a.states=new state[b.n];
       a.n=b.n;
     }
   } else {
       a.states=new state[b.n];
       a.n=b.n;
   }
   a.inita=b.inita;
   a.inits=b.inits;
   for(i=0;i<a.n;i++)for(j=0;j<Vstates;j++){
      a.states[i].resp[j]=b.states[i].resp[j];
      a.states[i].tran[j]=b.states[i].tran[j];
   }
   //copy a fingerprint if one is present
   if(b.fpz){
     if(a.fpz!=b.fpz){
       a.clearfp();
       a.fpz=b.fpz;
       a.fp=new double[a.fpz];
     }
     for(i=0;i<a.fpz;i++)a.fp[i]=b.fp[i];
   } else a.clearfp();
}

void upcopy(aut &a,aut &b){//copy, with allocation if needed, and 
                           //duplicate a state
int i,j,dup;

   a.clearfp(); 
   if(a.states)delete [] a.states;
   a.states=new state[b.n+1];
   a.n=b.n+1;
   a.inita=b.inita;
   a.inits=b.inits;
   for(i=0;i<b.n;i++)for(j=0;j<Vstates;j++){
      a.states[i].resp[j]=b.states[i].resp[j];
      a.states[i].tran[j]=b.states[i].tran[j];
   }

   //now flip a coin for every incoming arrow and reassign 
   //to the duplicated state on `heads'
   dup=lrand48()%b.n;
   a.states[b.n]=a.states[dup];
   for(i=0;i<b.n;i++)
     for(j=0;j<Vstates;j++)
       if(a.states[i].tran[j]==dup){
         if(lrand48()%2)a.states[i].tran[j]=b.n;
       }

}
