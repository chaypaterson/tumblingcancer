#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <cmath>
#include <set>
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
{   // intialize von neumann neighbourhoods
    for (int i=0; i<3; i++)
    {
        for (int j=-1; j<2; j+=2)
        {
            posn foo = (posn){0,0,0};
            foo[i] = j;
            ns.insert(foo);
        }
    } 
}

int main()
{
    // our walker should have a position and a velocity
    posn location;
    posn polarity;

    vector<posn> cellposns; // set of replicates
    vector<posn> cellpolys; // set of replicates

    // initialise "unit vectors"
    set<posn> neighbours;
    init_neigh(neighbours);

    double alpha = 0.5; // tumbling rate

    mt19937 gen((int)12345);         // seed rng
    uniform_real_distribution<> dis(0,1); // uniform distribution from 0 to 1.
    uniform_int_distribution<> uds(1,neighbours.size());

    // initialise first cell:
    location = (posn){0,0,0};   // cell at origin
    polarity = (posn){1,0,0};   // cell faces +x

    // TODO implement growth!

    int repls = 100000; // number of replicates

    for (int i=0; i<repls; i++)
    {   // initialise replicates:
        cellposns.push_back(location);
        cellpolys.push_back(polarity);
    } 

    for (int tt=0; tt<1000; tt++)
    {   // dt = 1 for now.
        for (int ii=0; ii<repls; ii++)
        {   // for each replicate
            if ( dis(gen) < alpha )
            {   // tumble
                int i = 0, ix = uds(gen);   // choose random neighbour
                for (auto q = neighbours.begin(); q != neighbours.end(); q++) 
                {   // iterate over neighbours
                    i++;
                    if (i == ix) 
                    {
//                        polarity = *q;    // single cell only
                        cellpolys[ii] = *q;
                        break;
                    }
                }
            }

//            location = location + polarity;   // single cell only
            cellposns[ii] = cellposns[ii] + cellpolys[ii];
        }

        // write means:
        cout << tt+1 << ", "<< meansq(cellposns) << ", ";
        cout << corrf(cellpolys) << ", ";
        // write "typical cell" path:
        cout << cellposns[0].x <<", "<< cellposns[0].y <<", "<< cellposns[0].z<< endl;
    }

    return 0;
} 
