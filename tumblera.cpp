#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <cmath>
#include <set>
#include <deque>
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

// Arithmetic functions:

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

int index(int x)
{
    int index;
    if (x>=0) {
        index = 2*x;
    } else {
        index = -2*x + 1;
    }

    return index;
}

// Custom types:

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

posn randnb(int ix, set<posn> neighbours)
{
    int i = 0;   // choose random neighbour
    posn randn;
    for (auto q = neighbours.begin(); q != neighbours.end(); q++) 
    {   // choose random polarity
        i++;
        if (i == ix)
        {
            randn = *q;
            break;
        }
    }

    return randn;
}

int main(int argc, char *argv[])
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
    
    // Initialise RNG:
    int seed;

    if (argc == 1)
    {   // preserve previous behaviour:
        seed = 12345;
    } else {
        seed = atoi(argv[1]);
    }

    mt19937 gen(seed);  // seed rng

    uniform_real_distribution<> dis(0,1); // uniform distribution from 0 to 1.
    uniform_int_distribution<> uds(1,neighbours.size());

    // Container for cells
    typedef pair<posn,posn> walker; // make tuple, with last entry a set of neighbour posns?
    vector<walker> walkers;     // iterator
    vector<walker> newwalk;     // walkers to add

    // Lattice full marker:
    int HLF = 0;
    int LF = HLF*2+1;
    int HLFx = HLF, HLFy = HLF, HLFz = HLF;
    vector<int> vx(LF,0);
    vector<vector<int>> vy(LF,vx);
    vector<vector<vector<int>>> isfull(LF,vy);

    // initialise first cell: a grower:
    origin = (posn){0,0,0};   // cell at origin
    walkers.push_back(make_pair(origin,origin));

    isfull[HLFx+origin.x][HLFy+origin.y][HLFz+origin.z] = 1;
 
    // Simulation step flow:
    for (double tt=0; tt<1000; tt+=dt)
    {
        for (auto w = walkers.begin(); w != walkers.end(); w++)
        {   // This is now the only structure.
            if (w->second!=origin)
            {   // walkers:
                for (int i=0; i<steps; i++)
                {   // migration
                    if (dis(gen) < alpha*dt/(double)steps)
                    {   // tumbling:
                        w->second = randnb(uds(gen),neighbours);
                    }
                    // calculate new position:
                    posn newsite = w->first+w->second;

                    // if out of bounds in isfull, extend isfull
                    bool oob = false;
                    if (index(newsite.x)>=(HLFx*2+1)) HLFx+=2; oob=true;
                    if (index(newsite.y)>=(HLFy*2+1)) HLFy+=2; oob=true;
                    if (index(newsite.z)>=(HLFz*2+1)) HLFz+=2; oob=true;
                    if (oob)
                    {
                        isfull.resize(HLFx*2+1,vector<vector<int>>(HLFy*2+1,vector<int>(HLFz*2+1)));
                    }

                    // only move if new location empty
                    if (!isfull[index(newsite.x)][index(newsite.y)][index(newsite.z)])
                    {   // If so, move to new site:
                        isfull[index((w->first).x)][index((w->first).y)][index((w->first).z)] = 0;
                        w->first = newsite;
                        isfull[index((w->first).x)][index((w->first).y)][index((w->first).z)] = 1;
                    }
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
                    posn newsite = w->first+randnb(uds(gen),neighbours);

                    // if out of bounds in isfull, extend isfull
                    bool oob = false;
                    if (index(newsite.x)>=(HLFx*2+1)) HLFx+=2; oob=true;
                    if (index(newsite.y)>=(HLFy*2+1)) HLFy+=2; oob=true;
                    if (index(newsite.z)>=(HLFz*2+1)) HLFz+=2; oob=true;
                    if (oob)
                    {
                        isfull.resize(HLFx*2+1,vector<vector<int>>(HLFy*2+1,vector<int>(HLFz*2+1)));
                    }

                    cout << HLFx*2+1 <<","<<HLFy*2+1<<","<<HLFz*2+1<<endl;
                    cout <<newsite.x<<","<<newsite.y<<","<<newsite.z<<endl;
                    cout <<index(newsite.x)<<","<<index(newsite.y)<<","<<index(newsite.z)<<endl;
                    cout << isfull[index(0)][index(0)][index(0)]<<endl;
                    cout << isfull.size()<<","<<isfull[1].size()<<","<<isfull[1].size()<<endl;
                    cout <<isfull[index(newsite.x)][index(newsite.y)][index(newsite.z)]<<endl;

                    // only divide if trial site free:
                    if (!isfull[index(newsite.x)][index(newsite.y)][index(newsite.z)])
                    {
                        newwalk.push_back(make_pair(newsite,origin));
                        isfull[index((newsite).x)][index((newsite).y)][index((newsite).z)] = 1;
                    }
                }

                if (dis(gen) < rgw*dt)
                {   // add random polarity
                    w->second = randnb(uds(gen),neighbours);
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
