/*************************************************************/
/*                                                           */
/*               Interface for variable automata             */
/*                                                           */
/*************************************************************/

/*

                Finite state machines
                Variable size
                Dan Ashlock 12/20/00

                Cellular encoding method added 
                1/6/05

Cellular encoding commands:

   0 - increment the initial action
   1 - increment the "c" response from the current state
   2 - increment the "d" response from the current state
   3 - move the current state pointer along the "c" transition 
   4 - move the current state pointer along the "d" transition 
   5 - Duplicate the current state and make it the destination
       of the "c" transition
   6 - Duplicate the current state and make it the destination
       of the "d" transition
   7 - Move the "c" transition from the current state to the 
       current state and pin it to the current state pointer
   8 - Move the "d" transition from the current state to the 
       current state and pin it to the current state pointer
   9 - Release the transition arrow if it is pinned
  10 - Move the "c" transition arrow to the destination of 
       the "c" transition arrow out of the state it points to
  11 - Move the "d" transition arrow to the destination of 
       the "d" transition arrow out of the state it points to
  12 - Move the current state pointer back along the creation link
*/

/*
        Markov derivator added 2/18/05

    Encoding:  States are 4x(state number)+2*(JA-Def)+(M-Def)
    Recall that Def=1  Coop=0
    States are derived from CIi and DIi using only accesible states
    Encoding for variables types:
     0 a
     1 1-a
     2 b
     3 1-b

 */


#ifndef	_VAUT_H
#define	_VAUT_H

using namespace std;

//number of transitions out per state
#define Vstates 2
//size of response alphabet
#define VAsize 2

struct state{//responses are tied to transitions

int resp[Vstates];  //responses      0=C   1=D
int tran[Vstates];  //transitions    if C  if D

};

class aut {  //mealey machine class

friend ostream &operator<<(ostream &,aut &);
friend istream &operator>>(istream &,aut &);

//two point crossover operator, non-aligned
friend void tpc(aut &,aut &);

//  a <=== b
friend void copy(aut &,aut &);  //copy, with allocation if needed
friend void upcopy(aut&,aut&);  //as above but duplicates a state

public:
    aut();        //create an empty automata
    aut(int nv);  //create an automata with nv states
    aut(const aut &);   //copy constructor
    ~aut();       //deallocate the automata

    //management
    void clearfp();        //clear the fingerprint, if any
    void clear();          //clear the automata, deallocate states
    void blank(int nv);    //create a blank automata

    //cellular encoding creation method
    void ccreate(int *encoding,int el);

    //secondary initialization
    void create(int nv);   //as the initializer aut(int nv) for populations
    void recreate();       //fill in a new automata
    void initTFT();        //reinitializes to stretched tit-for-tat
    void initPSYCO();      //reinitializes to stretched tit-for-tat
    void initAllD();       //reinitializes to stretched allD
    void initAllC();       //reinitializes to stretched allC
    void initPavlov();     //reinitializes to Pavlov
    void initTF2T();        //reinitializes to tit-for-two-tats
    void init2TFT();         //reinitializes to two-tits-for-tat
    void initRipoff();        //reinitialize to Ripoff
    void initFortress(int ns); //reinitialize to Fortress-n
    void initAppoxMajority(int k); //reinitalize to Majority-k
    void initTrifecta();
    void initTrifectaM();

    //overloaded operators
    const aut &operator=(const aut &);

    //Self description
    int size();                  //returns number of states in the automata
    int print();                 //returns fingerprint size (0=none)
    void makeFP25(double tol);   //create the 25-point TFT fingerprint with
                                 //accuracy tol
    void makeFP25();             //create the 25-point TFT fingerprint
                                 //from the Markov equations
    void tellfp(double *pb);      //returns the fingerprint values

    void sps(char *s);           //returns the self play string

    int nas();                   //reports states reachable from initial state

    void reachfall(int *row,     //recursive helper for ncc
                   int curs);
    int ncc();                   //number of communicating classes
    int SDL();                   //State depth length
    void MarkFall(int **st,      //Recursively derive Markov Equations
                  int *vis,
		  int who
                 ); 
    void MarkovE(ostream &aus);  //print out the Markov Equations

    //estimate the value of the tit-for-tat fingerprint of the machine 
    //at point alpha beta with given tolerance
    double fingerprintTFT(double alpha,double beta,double tol);

    /* Simplification Methods */
    void ReduceACC(aut &other); //reduce to states accessible from the initial 
                                //state only

    //The next several routines build up to ReduceEQ
    //FallSDL is a helper for FindSDL 
    void FallSDL(int *t,         //signature array
                 int val,       //current node value
                 int inc,        //current increment value
                 int cs);        //current state

    void FindSDL(int starter,    //Given a starting state, find the (flat) 
                 int *signature);//tree of responses in signature

    //ER has as many entries as the states of the machine.  It returns the 
    //number of equivalence classes of states and the class for each state 
    //in ER.  The classes are numbered 1..k where k+1 is the return value.
    int StateEQ(int *ER);       //give the equivalence relations on the states

    //reduce EQ creates a new automaton by reducing modulo the kind of 
    //equivalence relation computed by StateEQ
    void ReduceEQ(aut &);  //create a reduced version of the automaton
              
    //usage
    int reset();               //returns initial action
    int reset(int &wiis);      //returns initial action and state   
    int run(int inp);          //run on input inp
    int run(int inp,int &stt); //run on input inp with given state

    //these routines assume the fingerprints have the same length
    double Dsqr(aut &other);   //squared fingerprint distance
    double D(aut &other);      //fingerprint distance

    //genetic operations, other than two point crossover
    void mutateaction();        //mutate an action
    void mutatetransition();    //mutate a transition
    void copystatemutation();   //copy a state, dealing out incoming arrows
    void initialmutation();     //mutate the initial state

    //I.O routines
    void printn(ostream &aus);   //numerical (rather than PD metaphore) print
    void printFP(ostream &aus);  //output with fingerprint
    void readFP(istream &inp);   //input with fingerprint

    //assorted fitness functions
    int predict(int *ref,int bits);  //string of bits and its length
    int selfdri();                   //self driving length fitness funciton

private:

   int n;      //number of states
   int inits;  //initial state
   int inita;  //initial action
   int cur;    //current state   

   int fpz;     //fingerprint size    0 indicates none
   double *fp;  //fingerprint
 
   state *states;

};

#endif /* _VAUT_H */
