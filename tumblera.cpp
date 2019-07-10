#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <cmath>
#include <set>
#include <map>
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
    // our first walker should have a position and a velocity
    posn origin;
    
    // initialise "unit vectors"
    set<posn> neighbours;   // set of neighbouring positions
    init_neigh(neighbours); // initialise neighbour set

    // declare constants
    double dt = 1;
    double alpha = 0;   // DEFAULT tumbling rate
    double beta = 0; // growth rate
    double rgw = 0.0001;  // grower-->walker switching rate
    double rwg = 0;   // reverse switching rate
    int steps = 1;     // "speed" (careful! this is an INTEGER)
    // True speed is = steps/dt
    
    int seed;

    if (argc == 1)
    {   // preserve previous behaviour:
        seed = 12345;
    } else {
        seed = atoi(argv[1]);
    }

    if (argc >= 3) alpha = atof(argv[2]);   // set alpha from command line
    ofstream outfile;
    ofstream visfile;

    if (argc >= 4) // read file names
    {
        outfile.open(argv[3]);
        if (argc >= 5) visfile.open(argv[4]);
    } else {
        cout << "Error: no output file." << endl;
        outfile.close();
        visfile.close();
        exit(1);
    }

    // Declare data write mode:
    int WritePop = 0;
    int WriteSQD = 1;
    int WriteShape = 0;
    // TODO check that only one of these is 1.

    // Initialise RNG
    mt19937 gen(seed);  // seed RNG

    uniform_real_distribution<> dis(0,1); // uniform distribution from 0 to 1.
    uniform_int_distribution<> uds(1,neighbours.size());

    // Container for cells
    map<posn, posn> walkers;
    map<posn, posn> newwalk;
    vector<posn> oldwalk;

    // initialise first cell: a walker:
    origin = (posn){0,0,0};   // cell at origin
    walkers[origin] = (posn){1,0,0};

    int population = 1; // NEW: track simulation progress.
    double tt = 0;

    // Calculate probabilities:
    double ptumb = 1-exp(-alpha*dt/(double)steps);
    if (alpha<0) ptumb = 1;
    double pgrow = 1-exp(-beta*dt);
    double pwg = 1-exp(-rwg*dt);
    double pgw = 1-exp(-rgw*dt);

    // Simulation step flow:
    while (tt<1000)//(population<1.5E8)
    {
        for (auto w = walkers.begin(); w != walkers.end(); w++)
        {   // This is now the only structure.
            if (w->second!=origin)
            {   // walkers:
                posn newsite = w->first; // position before migration:

                for (int i=0; i<steps; i++)
                {   // migration
                    if (dis(gen) < ptumb)
                    {   // tumbling:
                        walkers[newsite] = randnb(uds(gen),neighbours);
                    }

                    // only move if new location empty
                    // NB also check newwalk, otherwise collisions between
                    // walkers may result
                    if ((walkers.count(newsite+w->second)==0)&&(newwalk.count(newsite+w->second)==0))
                    {   // If so, move to new site:
                        newwalk[newsite+w->second] = w->second; // keeping polarity
                        oldwalk.push_back(newsite);     // mark for removal
                        // NB: this may result in multiple entries in oldwalk
                        // for the same site, but checking beforehand is slow, O(N)
                        newsite = newsite+w->second;    // update site
                    }
                }

                // Type switching:
                if (dis(gen) < pwg)
                {   // remove cell polarity
                    walkers[newsite] = origin;
                } 
            } else {
                // growers:
                if (dis(gen) < pgrow)
                {
                    posn newsite = w->first+randnb(uds(gen),neighbours);

                    // only divide if trial site free:
                    if (walkers.count(newsite)==0)
                    {
                        newwalk[newsite]=origin;
                    }
                }

                if (dis(gen) < pgw)
                {   // add random polarity
                    walkers[w->first] = randnb(uds(gen),neighbours);
                }
            }
        }

        // Bookkeeping:
        double growthrate = 0;
        growthrate -= walkers.size();
        walkers.insert(newwalk.begin(), newwalk.end());
        newwalk.clear();

        growthrate += walkers.size();
        growthrate /= dt;

        for (auto q = oldwalk.begin(); q != oldwalk.end(); q++)
        {   // should be O(log(walkers.size()))
            // WARNING: test if cell to be deleted is already in walkers
            if (walkers.count(*q)) walkers.erase(walkers.find(*q));
        }
        oldwalk.clear();

        // Statistics:
        if (fmod(tt,1.00) < 1.5*dt)
        {
            /*
            // Store all POPULATION and SUBPOPULATION data
            if (WritePop) {
                population = walkers.size();

                int fecund = 0;
                for (auto w = walkers.begin(); w != walkers.end(); w++)
                {
                    if (w->second==origin) fecund++;
                }
                int invasive = population-fecund;
                // write means: // TODO populations and positions separately
                outfile << tt <<",\t"<< population;
                outfile << ",\t" << fecund <<",\t"<< invasive;
                outfile << ",\t" << growthrate << endl;
            }
            */
            //if (WriteSQD) {
            // TODO test output is the same
            // Store ALL position data and square displacements
            for (auto cell = walkers.begin(); cell != walkers.end(); cell++)
            {
                outfile << (cell->first).x <<", ";
                outfile << (cell->first).y <<", ";
                outfile << (cell->first).z <<", ";
                outfile << tt <<", "<< sqd(cell->first) << endl;// SQDISP
            }
            //}
        }

        tt += dt;
    }

    // TODO output shape/coordinates.
    /* not currently used
    if (WriteShape) {
        for (auto w = walkers.begin(); w != walkers.end(); w++)
        {
            visfile << (w->first).x <<", ";
            visfile << (w->first).y <<", ";
            visfile << (w->first).z <<endl;
        }
    }
    */

    outfile.close();

    return 0;
}
