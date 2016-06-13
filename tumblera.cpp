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

int main()
{   // Declarations and initialisations:
    // our walker should have a position and a velocity
    posn origin;
    posn polarity;
    
    // initialise "unit vectors"
    set<posn> neighbours;   // set of neighbouring positions
    init_neigh(neighbours); // initialise neighbour set

    int dt = 1;
    double alpha = 1;   // tumbling rate
    double beta = 0.1; // growth rate
    double rgw = 0.1;  // switching rate
    double rwg = 0.1;//10*rgw;   // reverse switching rate

    mt19937 gen((int)12345);         // seed rng
    uniform_real_distribution<> dis(0,1); // uniform distribution from 0 to 1.
    uniform_int_distribution<> uds(1,neighbours.size());

    // For GROWERS:

    // NEW: one type of cell ONLY
    typedef pair<posn,posn> walker;
    vector<walker> walkers;     // iterator
    vector<walker> newwalk;     // walkers to add

    // initialise first cell: a walker:
    origin = (posn){0,0,0};   // cell at origin
    polarity = (posn){1,0,0};   // cell faces +x

    walkers.push_back(make_pair(origin,polarity));

    // Simulation step flow:
    for (int tt=0; tt<1000; tt+=dt)
    {   // dt = 1 for now.

    // Invasion:
        for (auto w = walkers.begin(); w != walkers.end(); w++)
        {
            if (w->second!=origin)
            {   // walkers:
                if (dis(gen) < alpha)
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
                // TODO boilerplate!
                int numws = 0;
                for (auto q = neighbours.begin(); q != neighbours.end(); q++)
                {   // check all polarities of this site.
                    numws += count(walkers.begin(),walkers.end(),make_pair(newsite,*q));
                    numws += count(newwalk.begin(),newwalk.end(),make_pair(newsite,*q));
                }
                numws += count(newwalk.begin(),newwalk.end(),make_pair(newsite,origin));
                numws += count(walkers.begin(),walkers.end(),make_pair(newsite,origin));

                // If so, move to new site:
                if (numws==0) w->first = newsite;

                // Type switching:
                if (dis(gen) < rwg)
                {   // rwg ~= 100 rgw
                    // remove cell polarity
                    w->second = origin;
                }
            } else {
                // growers:
                if (dis(gen) < beta)
                {
                    posn newsite;
                    int i = 0, ix = uds(gen);   // choose random neighbour
                    for (auto q = neighbours.begin(); q != neighbours.end(); q++)
                    {   // get trial neighbouring site:
                        i++;
                        if (i == ix)
                        {
                            newsite = *q;
                            break;
                        }
                    }
                    // only divide if trial site free:
                    int numws = 0;
                    for (auto q = neighbours.begin(); q!= neighbours.end(); q++)
                    {
                        numws += count(walkers.begin(),walkers.end(),make_pair(newsite,*q));
                        numws += count(newwalk.begin(),newwalk.end(),make_pair(newsite,*q));
                    }
                    numws += count(newwalk.begin(),newwalk.end(),make_pair(newsite,origin));
                    numws += count(walkers.begin(),walkers.end(),make_pair(newsite,origin));
                    
                    if (numws==0) newwalk.push_back(make_pair(newsite,origin));
                }

                if (dis(gen) < rgw)
                {   // add random polarity
                    int i = 0, ix = uds(gen);
                    for (auto q = neighbours.begin(); q != neighbours.end(); q++)
                    {
                        i++;
                        if (i == ix)
                        {
                            w->second = *q;
                            break;
                        }
                    }
                }
            }
        }

    // Bookkeeping:        

       // NEW
        walkers.insert(walkers.end(),newwalk.begin(), newwalk.end());
        newwalk.clear();

        // Sort out walkers:
            // Statistics:
        // write means: // TODO populations and positions separately
        cout << tt <<",\t"<< walkers.size();
        cout << endl;
    }

    return 0;
} 
