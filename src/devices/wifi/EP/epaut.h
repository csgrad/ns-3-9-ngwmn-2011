#ifndef EPAUT_HEADER
#define EPAUT_HEADER

#include <iostream>

using namespace std;

//number of transitions out per state
#define Vstates 2
//size of response alphabet
#define VAsize 2

#define Nmax 10

struct state{//responses are tied to transitions

int resp[Vstates];  //responses
int tran[Vstates];  //transitions

};

class epaut{

public:

    epaut();        //create an empty automata
    epaut(int nv);  //create an automata with nv states
    epaut(const epaut &);   //copy constructor
    ~epaut();       //deallocate the automata

    const epaut &operator=(const epaut &);

    void recreate();   //as the initializer aut(int nv) for populations
    void create(int nv);   //as the initializer aut(int nv) for populations
    void clear();          //clear the automata, deallocate states
    void blank(int nv);    //create a blank automata

    void reset();
    int run(int in);

    void mutout();
    void muttrn();
    void mutfst();
    int mutadd();
    int mutdel();
    void printn(ostream &aus);


private:

    int n; //number of states
    int fst;//initial state
    int cur;//current state
    state *states; //state machine
};

#endif
