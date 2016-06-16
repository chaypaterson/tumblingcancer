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

bool isfree(vector<pair<posn,posn>> a, vector<pair<posn,posn>> b, posn site)
{   // check is site is free from entries in a or b
    set<posn> ns;
    init_neigh(ns);
    posn origin = (posn){0,0,0};

    int numws = 0;
    for (auto q = ns.begin(); q != ns.end(); q++)
    {   // check all polarities of this site.
        numws += count(a.begin(),a.end(),make_pair(site,*q));
        numws += count(b.begin(),b.end(),make_pair(site,*q));
    }
    numws += count(a.begin(),a.end(),make_pair(site,origin));
    numws += count(b.begin(),b.end(),make_pair(site,origin));
    
    return (numws==0);
    // TODO this is O(N) and is called in an O(N) loop: bad! Improve speed.
}

int main()
{   // Declarations and initialisations:
    // our walker should have a position and a velocity
    posn origin;
    
    // initialise "unit vectors"
    set<posn> neighbours;   // set of neighbouring positions
    init_neigh(neighbours); // initialise neighbour set

    double dt = 1;
    double alpha = 1;   // tumbling rate
    double beta = 0.1; // growth rate
    double rgw = 0.01;  // grower-->walker switching rate
    double rwg = 0.1;   // reverse switching rate
    int steps = 10;     // speed multiplier (careful!)
    // True speed is = steps/dt

    mt19937 gen((int)12345);         // seed rng
    uniform_real_distribution<> dis(0,1); // uniform distribution from 0 to 1.
    uniform_int_distribution<> uds(1,neighbours.size());

    // NEW: one type of cell ONLY
    typedef pair<posn,posn> walker;
    vector<walker> walkers;     // iterator
    vector<walker> newwalk;     // walkers to add

    // initialise first cell: a grower:
    origin = (posn){0,0,0};   // cell at origin

    walkers.push_back(make_pair(origin,origin));

    // Simulation step flow:
    for (double tt=0; tt<1000; tt+=dt)
    {   // dt = 1 for now.
    // Invasion:
        for (auto w = walkers.begin(); w != walkers.end(); w++)
        {   // This is now the only structure.
            if (w->second!=origin)
            {   // walkers:
                for (int i=0; i<steps; i++)
                {   // migration
                    if (dis(gen) < alpha*dt/(double)steps)
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
                    // If so, move to new site:
                    if (isfree(walkers,newwalk,newsite)) w->first = newsite;
                }

                // Type switching:
                if (dis(gen) < rwg*dt)
                {   // rwg ~= 100 rgw
                    // remove cell polarity
                    w->second = origin;
                }
            } else {
                // growers:
                if (dis(gen) < beta*dt)
                {
                    posn newsite;
                    int i = 0, ix = uds(gen);   // choose random neighbour
                    for (auto q = neighbours.begin(); q != neighbours.end(); q++)
                    {   // get trial neighbouring site:
                        i++;
                        if (i == ix)
                        {
                            newsite = w->first+*q; // neighbour OF CURRENT POSITION
                            break;
                        }
                    }
                    // only divide if trial site free:
                    if (isfree(walkers,newwalk,newsite)) newwalk.push_back(make_pair(newsite,origin));
                }

                if (dis(gen) < rgw*dt)
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
        walkers.insert(walkers.end(),newwalk.begin(), newwalk.end());
        newwalk.clear();

    // Statistics:
        if (fmod(tt,1.00) < 1.5*dt)
        {
            int fecund = 0;
            for (auto w = walkers.begin(); w != walkers.end(); w++)
            {
                if (w->second==origin) fecund++;
            }
            int invasive = walkers.size()-fecund;
            // write means: // TODO populations and positions separately
            cout << tt <<",\t"<< walkers.size();
            cout << ",\t" << fecund <<",\t"<< invasive;
            cout << endl;
        }
    }

    return 0;
}
