#ifndef BASICSINCLUDED
#define BASICSINCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "obj.hpp"
/* data types */
#if defined(__BUILD_FOR_INTEL_DARWIN)
#else
  typedef unsigned long ulong;
#endif

typedef unsigned char byte;

/* distance --- depends on whether it is discrete or continuous */
#define CONT
#ifdef CONT
typedef double Tdist;
#else
typedef int Tdist;
#endif

#include "obj.hpp"

/* random generators --- se usage examples in genqueries.c */
#define faleat (((unsigned)random())/((double)(unsigned)(1<<31)))
#define aleat(n) ((unsigned)((faleat)*(n)))


/* finer memory management */
void *_free(void *p);
void *verif(void *p);
#define mymalloc(n) ((n)==0?NULL:verif((void*)malloc(n)))
#define myrealloc(p,n) ((p)==NULL?malloc(n):((n)==0?_free(p):verif((void*)realloc(p,n))))
#define myfree(p) { if ((p)!=NULL) free(p); }


/* management of the list of candidates for NN queries */
struct celem
{
	Obj id;
    Tdist dist;
}; // list object

struct Tcelem
{
	celem *elems;
    int csize;
    int k;
}; // list

// list of answers for range queries
struct opair
{
	Tdist   d; // distance to the query
	Obj     o; // object
}; // object

#define uint unsigned int

struct Tret
{
	uint chunk;
	opair *ret;
	uint  iret;
}; // answer

/* creates a celem of size k */
Tcelem createCelem (int k);

/* adds element if (distance dist) to celems */
void addCelem (Tcelem *celems, Obj id, Tdist dist);

/* tells whether dist is out of the range of celem */
bool outCelem (Tcelem *celems, Tdist dist);

/* prints the objects of celem */
void showCelem (Tcelem *celems);

/* gives the radius of the farthest element in celem, -1 if empty */
Tdist radCelem (Tcelem *celems);

/* frees celem */
void freeCelem (Tcelem *celems);


/* inserts Or in ret if it is at distance at most r from query o */
bool insRet(Tret *ret,Obj Or, Obj o, Tdist r);

/* inserts Or in ret */
void FinsRet(Tret *ret,Obj Or);

/* inits ret with realloc step 3 */
void iniciaRet(Tret *ret);

void *_free(void *p) { myfree(p); return NULL; }
void *verif(void *p) {
	if (p == NULL) {
		printf ("Error -- out of memory\n");
		exit(1);
	}
	return p;
}


Tcelem createCelem (int k)
{
	Tcelem celems;
    celems.elems = (celem*)mymalloc (k*sizeof(celem));
    celems.k = k;
    celems.csize = 0;
    return celems;
}


void addCelem (Tcelem *celems, Obj id, Tdist dist)
{
	int i,pos = celems->csize,pos2;
    while ((pos>0) && (celems->elems[pos-1].dist > dist)) pos--;
    if (pos == celems->k) return;  /* no entra entre los elegidos */
    pos2=pos;

    while ((pos2>0) && (celems->elems[pos2-1].dist == dist))
	{
		if (id == celems->elems[pos2].id) return; /* repetido */
 	    pos2--;
    }

    if (celems->csize < celems->k) celems->csize++;
    for (i=celems->csize-1;i>pos;i--) celems->elems[i] = celems->elems[i-1];
    celems->elems[pos].id = id; celems->elems[pos].dist = dist;
}


bool outCelem (Tcelem *celems, Tdist dist)
{
	return ((celems->csize == celems->k) &&
	    (dist > celems->elems[celems->k-1].dist));
}


void showCelem (Tcelem *celems){}
//{
//	int j;
//    for (j=0;j<celems->csize;j++)
//    {
//		printobj(celems->elems[j].id);
//    }
//}

void freeCelem (Tcelem *celems)
{
	myfree (celems->elems);
}


Tdist radCelem (Tcelem *celems)
{
	return (celems->csize > 0) ? celems->elems[celems->csize-1].dist : -1;
}


bool insRet(Tret *ret,Obj Or, Obj o, Tdist d)
{
	bool rt=false;
	if(!(ret->iret%ret->chunk))
	{
		ret->ret=(opair *)myrealloc(ret->ret,((ret->iret/ret->chunk)+1)*ret->chunk*sizeof(*(ret->ret)));
	}
	//     if(((ret->ret)[ret->iret].d=distance(Or,o))<=r)
	{
		(ret->ret)[ret->iret].o=Or;
		(ret->ret)[ret->iret].d=d;
		(ret->iret)++;
		rt=true;
	}
	return rt;
}


void FinsRet(Tret *ret,Obj Or)
{
	if(!(ret->iret%ret->chunk))
	{
		ret->ret=(opair *)myrealloc(ret->ret,((ret->iret/ret->chunk)+1)*ret->chunk*sizeof(*(ret->ret)));
	}
	(ret->ret)[ret->iret].o=Or;
	(ret->iret)++;
}


void iniciaRet(Tret *ret)
{
  ret->chunk = 3;
  ret->iret = 0;
  ret->ret = NULL;
}

#endif
