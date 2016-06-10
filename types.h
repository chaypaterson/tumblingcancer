#include <cstdlib>
#include <cstdio>

using namespace std;

const long int L = 10001;   // lattice side length (must be odd)
const long int hl = 5000;   // half length -1

struct posn { // position type: cells have this
    long int x;
    long int y;
    long int z;

    posn& operator=(const posn& a)
    {
        x = a.x;
        y = a.y;
        z = a.z;
        return *this;
    }

    posn operator+(const posn& a) const
    {
        return (posn){a.x+x, a.y+y, a.z+z};
    }

    long int operator[](int i) const
    {
        switch (i)
        {
            case 0:
                return this->x;
            break;
            case 1:
                return this->y;
            break;
            case 2:
                return this->z;
            break;
            default:
                cout << "BAD INDEX: " << i << endl;
                exit(1);
            break;
        }
    }

    long int &operator[](int i)
    {
        switch (i)
        {
            case 0:
                return this->x;
            break;
            case 1:
                return this->y;
            break;
            case 2:
                return this->z;
            break;
            default:
                cout << "BAD INDEX: " << i << endl;
                exit(1);
            break;
        }
    }
};

long int index(posn xyz)    // make function converting posn to index
{
    long int IX;
    IX = hl + xyz.x; // xyz[0]
    IX += L*(hl +xyz.y);
    IX += L*L*(hl+xyz.z);
/*  long double IX = 3<<(xyz.x); // failed
    IX *= 5<<(xyz.y);
    IX *= 7<<(xyz.z);
*/

    return IX;
}

bool operator==(const posn& a, const posn& b)
{
    bool isit = (a.x)==(b.x);
    isit = isit && ((a.y)==(b.y));
    isit = isit && ((a.z)==(b.z));
    return isit;
}

bool operator<(const posn& left, const posn& right) 
{
    return index(left) < index(right);
}
