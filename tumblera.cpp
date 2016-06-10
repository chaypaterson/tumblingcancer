#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <cmath>
#include <set>
#include <algorithm>
#include <random>
#include <ctime>
#include "types.h"

using namespace std;

/* Run-and-tumble simulations for a persistent random walker on a lattice.
 * The random walker should have a position and a velocity/polarity.
 * The polarity should also be a position.
 * This should output the MSD and trajectory of the walker.
 * */

double sqd(posn r)
{   // calculate square displacements
    double sqd = 0;
    for (int i=0; i<3; i++)
    {
        sqd += r[i]*r[i];
    } 
    return sqd;
}

double meansq(vector<posn> r)
{
    double msqd=0;
    for (int i=0; i<(int)r.size(); i++)
    {
        msqd += sqd(r[i]);
    }
    msqd = msqd/r.size();

    return msqd;
}

double corrf(vector<posn> q)
{
    double mean=0;
    for (int i=0; i<(int)q.size(); i++)
    {
        mean += q[i].x;
    }
    mean = mean/q.size();

    return mean;
}

void init_neigh(set<posn>& ns)
{   // intialize von neumann neighbourhoods:
    for (int i=0; i<3; i++)
    {
        for (int j=-1; j<2; j+=2)
        {
            posn foo = (posn){0,0,0};
            foo[i] = j;
            ns.insert(foo);
        }
    } 
    /*
    for (int i=-1; i<=1; i++)
    {   // initialize moore neighbourhoods:
        for (int j=-1; j<=1; j++)
        {
            for (int k=-1; k<=1; k++)
            {
                ns.insert((posn){i,j,k});
            }
        }
    }

    ns.erase((posn){0,0,0}); // do not include origin as neighbour
    */
}

/*
struct walker {
    posn site;  // lattice site
    posn plity; // cell polarity

    bool operator<(const walker& left, const walker& right)
    {
        return index(left.site)<index(right.site);
    }
};
*/ 

int main()
{   // Declarations and initialisations:
    // our walker should have a position and a velocity
    posn location;
    posn polarity;
    
    // initialise "unit vectors"
    set<posn> neighbours;   // set of neighbouring positions
    init_neigh(neighbours); // initialise neighbour set

    double dt = 1;
    double alpha = 1;   // tumbling rate
    double beta = 1; // growth rate
    double rgw = 0;//0.1;  // switching rate
    double rwg = 0.1;//10*rgw;   // reverse switching rate

    mt19937 gen((int)12345);         // seed rng
    uniform_real_distribution<> dis(0,1); // uniform distribution from 0 to 1.
    uniform_int_distribution<> uds(1,neighbours.size());

    // For GROWERS:
    set<posn> growers;  // set of growing cells
    set<posn> newgrow;  // cells to add
    set<posn> oldgrow;  // cells to remove
    set<posn> surface;  // iterator

    // for WALKERS:
    typedef pair<posn,posn> walker;
    vector<walker> walkers;     // iterator
    vector<walker> oldwalk;     // walkers to remove

    // initialise first cell: a walker:
    location = (posn){0,0,0};   // cell at origin
    polarity = (posn){1,0,0};   // cell faces +x

    walkers.push_back(make_pair(location,polarity));

    // Simulation step flow:
    for (double tt=0; tt<1000; tt+=dt)
    {   // dt = 1 for now.

    // Growth:
        for (auto p = surface.begin(); p != surface.end(); p++)
        {
            if (1)// ( dis(gen) < beta*dt )
            {
                int i = 0, ix = uds(gen);
                for (auto q = neighbours.begin(); q != neighbours.end(); q++) 
                {   // iterate over neighbours
                    i++;
                    posn r = *p+*q;
                    if ((i == ix) && (growers.count(r)==0)) 
                    {   // if random neighbour position is unoccupied
                        cout << i << endl;
                        newgrow.insert(r);
                        break;
                    }
                }
            }

            // Type switching:
            if ( dis(gen) < rgw*dt )
            {   // remove a cell from growers and add to walkers
                int i = 0, ix = uds(gen);
                for (auto q = neighbours.begin(); q != neighbours.end(); q++)
                {   // Choose random polarity
                    i++;
                    if (i == ix)
                    {
                        oldgrow.insert(*p); // mark for removal
                        walkers.push_back(make_pair(*p,*q));   // new walker
                        break;
                    }
                }
            }
        }

    // Invasion:
        for (auto w = walkers.begin(); w != walkers.end(); w++)
        {
            if ( dis(gen) < alpha*dt )
            {   // tumbling:
                int i = 0, ix = uds(gen);   // choose random neighbour
                for (auto q = neighbours.begin(); q != neighbours.end(); q++) 
                {   // choose random polarity
                    i++;
                    if (i == ix)
                    {
                        w->second = *q;
                        break;
                    }
                }
            }
            // only move if new location empty
            posn newsite = w->first+w->second;
            bool clear = 1;
            for (auto q = neighbours.begin(); q != neighbours.end(); q++)
            {   // check all polarities of this site.
                int numws = walkers.size();
                if (numws>0) numws = count(walkers.begin(),walkers.end(),make_pair(newsite,*q));
                // also check newsite for non-motile fecund cells:
                if ((numws>0) or (growers.count(newsite)>0) or (newgrow.count(newsite)>0))
                {   // if anything here, not available to move into.
                    clear = 0;
                    //break;
                }
            }
            // otherwise move to new site:
            if (clear) w->first = newsite;

            // Type switching:
            if ( dis(gen) < rwg*dt )
            {   // rwg ~= 100 rgw
                // remove a cell from walkers and add to growers
                newgrow.insert(w->first); //?
                oldwalk.push_back(*w);
            }
        }

    // Bookkeeping:        
        // Sort out growers:
        for (auto p = oldgrow.begin(); p != oldgrow.end(); p++)
        {   //remove oldcells from cells:
            growers.erase(*p);
            surface.erase(*p);    // and surface
        }
        //cout << oldgrow.size() <<","<< oldwalk.size() <<",";
        //cout << newgrow.size() << endl;
        for (auto w = oldwalk.begin(); w != oldwalk.end(); w++)
        {
            walkers.erase(w);
        }
        // Add newcells to cells:
        //growers.insert(newgrow.begin(), newgrow.end());
        for (auto p = newgrow.begin(); p!= newgrow.end(); p++) //!
        {
            growers.insert(*p);
            surface.insert(*p);
        }
        // Then empty these containers.
        newgrow.clear();
        oldgrow.clear();
        oldwalk.clear();

// !
    // Statistics:
        // write means: // TODO populations and positions separately
        cout << tt <<",\t"<< walkers.size() <<",\t"<< growers.size() << endl;
    }

    return 0;
} 
