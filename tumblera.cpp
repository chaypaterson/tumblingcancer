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

double msq(posn r)
{   // calculate mean square displacements
    double sqd = 0;
    for (int i=0; i<3; i++)
    {
        sqd += r[i]*r[i];
    } 
    return sqd;
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

    // initialise "unit vectors"
    set<posn> neighbours;
    init_neigh(neighbours);

    double alpha = 0; // tumbling rate

    mt19937 gen((int)12345);         // seed rng
    uniform_real_distribution<> dis(0,1); // uniform distribution from 0 to 1.
    uniform_int_distribution<> uds(1,neighbours.size());

    // initialise first cell:
    location = (posn){0,0,0};   // cell at origin
    polarity = (posn){1,0,0};   // cell faces +x

    for (int tt=0; tt<100000; tt++)
    {   // dt = 1 for now.
        if ( dis(gen) < alpha )
        {   // tumble
            int i = 0, ix = uds(gen);   // choose random neighbour
            for (auto q = neighbours.begin(); q != neighbours.end(); q++) 
            {   // iterate over neighbours
                i++;
                if (i == ix) 
                {
                    polarity = *q;
                    break;
                }
            }
        }

        location = location + polarity;
        cout << tt+1 << ", "<< msq(location) << ", ";
        cout << polarity.x << ", " << location.x << ", ";
        cout << location.y << ", " << location.z << endl;
    }

    return 0;
} 
