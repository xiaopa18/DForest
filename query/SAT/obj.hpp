#ifndef OBJINCLUDED
#define OBJINCLUDED

	/* object database. for now, only one active database at a time will
	   be permitted. all the operations fail if there is no loaded DB */

typedef int Obj;	/* object id */

#define NullObj (-1)   /* null object */

#define NewObj 0      /* new object */
void *_free(void *p);
void *verif(void *p);
#define mymalloc(n) ((n)==0?NULL:verif((void*)malloc(n)))
#define myrealloc(p,n) ((p)==NULL?malloc(n):((n)==0?_free(p):verif((void*)realloc(p,n))))
#define myfree(p) { if ((p)!=NULL) free(p); }

#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
using namespace std;

#ifdef CALC_DIST_COUNT
extern double dcmp;
#endif

class Objvector
{
	vector<float> value;
public:
	Objvector() {}
	Objvector(const Objvector &obj);
	Objvector(vector<float> v);
	Objvector & operator = (const Objvector & obj);
	vector<float> & getValue();
	void setValue(vector<float> v);
	~Objvector() {}
};
	/* loads a DB given its name, and returns its size. if there is
	   another one already open, it will be closed first */
int openDB (char *name);

	/* frees the currently open DB, if any */
void closeDB (void);

	/*(internal) computes the distance between two objects */
double distanceInter (Obj o1, Obj o2);
double qdistanceInter(float* o1, Obj o2);

      /* exported, computes distance and does the accounting */
#define mydistance(o1,o2)(distanceInter(o1,o2))
#define myqdistance(o1,o2)(qdistanceInter(o1,o2))

	/* returns an object identifier from description s. if the object
	   does not belong to the DB, it must be identified as NewObj */
Obj parseobj (char *str);

void printDBelements();

Objvector::Objvector(const Objvector & obj)
{
	value = obj.value;
}

Objvector::Objvector(vector<float> v)
{
	value = v;
}

Objvector & Objvector::operator=(const Objvector & obj)
{

	if (this == &obj) {
		return *this;
	}
	else {
		value = obj.value;
		return *this;
	}
}

vector<float> & Objvector::getValue()
{
	return value;
}



void Objvector::setValue(vector<float> v)
{
	value = v;
}

struct EuclDB
{
	float *nums;  /* coords all together */
    int nnums;	  /* number of vectors (with space for one more) */
    int coords;  /* coordinates */
    float *nobj;
    double df(float *p1, float *p2, int k) /* distance to use */
    {
        #ifdef CALC_DIST_COUNT
        dcmp++;
        #endif
        register int i;
        double tot = 0,dif;
        for (i=0;i<k;i++)
        {
            dif=p1[i]-p2[i];
            tot += dif*dif;
        }
        return sqrt(tot);
    }
    int openDB(vector<Node> &dataset)
    {
        coords = dataset[0].dim;
        nnums = dataset.size();
        nums = (float *)mymalloc((nnums + 1) * sizeof(float)* coords);
        nobj = (float *)mymalloc(coords * sizeof(float));

        for (int i = 0; i < nnums; ++i)
        {
            for (int j = 0; j < coords; ++j)
            {
                nums[i*coords + j]=dataset[i].data[j];
            }
        }
        return nnums;
    }
    double memory_used()
    {
        return sizeof(float)*coords*nnums;
    }
};

#endif
