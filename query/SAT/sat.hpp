#ifndef SAT_HPP
#define SAT_HPP
#include "./obj.hpp"
#include <stdio.h>
#include <time.h>
#include <iostream>
#include "./basics.hpp"
#include <vector>

#define BLK 5
#define space(ptr,size) \
 { if (((size) % BLK) == 0) \
      { (ptr) = myrealloc ((ptr),((size)+BLK)*sizeof(*(ptr))); } }

#define fi first
#define se second
using namespace std;
typedef pair<double,int> PDI;
typedef double Tdist;
typedef void* Index;

struct qelem
{
	Obj elem;
	Tdist dist;
	int who;
};

struct arrdata
{
	qelem *data;
	int ndata;
};

struct arrnum
{
	int *num;
	int nnum;
};

//node
struct nodo
{
	//int height;
	Obj obj;
	Tdist maxd;
	arrnum vec;
	arrdata queue;
};

struct heapElem
{
	int id;
	Tdist dist;  /* dist to q */
	Tdist lbound;  /* lower bound */
	Tdist mind; /* best distsance */
};

struct grafo
{
	nodo *nodos;
	int nnodos;
	int np;
	double memory_used()
    {
        double res=0;
        for(int i=0;i<nnodos;i++)
            res+=sizeof(int)*(nodos[i].queue.ndata+1);
        return res;
    }
};

struct SAT{
    Index S;
    EuclDB DB;
    int MaxHeight;
    void addHeap (heapElem *heap, int *hsize, int id, Tdist dist, Tdist lbound,Tdist mind);
    heapElem extrHeap (heapElem *heap, int *hsize);
    int newnode(grafo *G, Obj obj);
    void newvec(grafo *G, int node, Obj obj);
    void newqueue(grafo *G, int nodo, Obj obj, Tdist dist);
    void bubble (qelem *r, long lo, long up);
    qelem *sort (qelem *r, long lo, long up);
    void distr (grafo *G, int n,int h);
    void build(vector<Node> &dataset, int n);
    void _search (grafo G, int n, float* obj, Tdist r, Tdist d0, Tdist mind,vector<PDI> *res);
    void rangequery(float* obj, Tdist r, vector<PDI> *res);
    Tdist knn(float* obj, int k, bool show);
    SAT(vector<Node> &dataset,int MaxHeight=100000000):MaxHeight(MaxHeight){
        build(dataset,dataset.size());
    }

    double memory_used()
    {
        return sizeof(S)+DB.memory_used()+12+8+((grafo*)S)->memory_used();
    }

    ~SAT(){
        int n;
        grafo *G = (grafo*)S;
        for (n=0; n<G->nnodos; n++)
        free (G->nodos[n].vec.num);
        free (G->nodos);
        G->nodos = NULL;
        G->nnodos = 0;
        free (G);
    }
    double distanceInter (Obj o1, Obj o2)
    {
        return DB.df (db(o1),db(o2),DB.coords);
    }

    double qdistanceInter(float* o1,Obj o2)
    {
        for (int d = 0; d < DB.coords; ++d) {
            DB.nobj[d] = o1[d];
        }
        return DB.df(DB.nobj, db(o2), DB.coords);
    }
    float* db(int p)
    {
        return DB.nums + DB.coords*(int)p;
    }
};

void SAT::addHeap (heapElem *heap, int *hsize, int id, Tdist dist, Tdist lbound,Tdist mind)
{
    int hs;
	heap--;
	hs = ++*hsize;
	while (hs > 1)
	{
		int hs2 = hs>>1;
		if (heap[hs2].lbound <= lbound) break;
		heap[hs] = heap[hs2];
		hs = hs2;
	}
	heap[hs].id = id;
	heap[hs].dist = dist;
	heap[hs].lbound = lbound;
	heap[hs].mind = mind;
}

heapElem SAT::extrHeap (heapElem *heap, int *hsize)
{
    int i,dosi,newi,hs;
	heapElem ret = heap [0], tmp;
	heap[0] = heap[--*hsize];
	hs = *hsize;
	heap--;
	i = 1;
	dosi = i<<1;
	while (dosi <= hs)
	{
		newi = dosi;
		if ((dosi < hs) && (heap[dosi+1].lbound <heap[dosi].lbound))
			newi++;
		if (heap[newi].lbound <heap[i].lbound)
		{
			tmp = heap[newi];
			heap[newi] = heap[i];
			heap[i] = tmp;
			i = newi;
			dosi = i<<1;
		}
		else break;
	}
	return ret;
}

int SAT::newnode(grafo *G, Obj obj)
{
    if ((G->nnodos % BLK) == 0) {
		G->nodos = (nodo*)myrealloc(G->nodos, ((G->nnodos)+BLK)*sizeof(*(G->nodos)));
	}
	G->nodos[G->nnodos].obj = obj;
	G->nodos[G->nnodos].vec.num = NULL;
	G->nodos[G->nnodos].vec.nnum = 0;
	G->nodos[G->nnodos].queue.data = NULL;
	G->nodos[G->nnodos].queue.ndata = 0;
	return G->nnodos++;
}

void SAT::newvec(grafo *G, int node, Obj obj)
{
    int pos = newnode (G,obj);
	nodo *N = &G->nodos[node];
	if ((N->vec.nnum % BLK) == 0) {
		N->vec.num = (int*)myrealloc(N->vec.num,((N->vec.nnum)+BLK)*sizeof(*(N->vec.num)));
	}
	N->vec.num[N->vec.nnum] = pos;
	N->vec.nnum++;
}

void SAT::newqueue(grafo *G, int nodo, Obj obj, Tdist dist)
{
    qelem *nq;
	if ((G->nodos[nodo].queue.ndata % BLK) == 0) {
		G->nodos[nodo].queue.data = (qelem*)myrealloc(G->nodos[nodo].queue.data, ((G->nodos[nodo].queue.ndata) + BLK)*sizeof(*(G->nodos[nodo].queue.data)));
	}
	nq = &G->nodos[nodo].queue.data[G->nodos[nodo].queue.ndata];
	nq->elem = obj;
	nq->dist = dist;
	nq->who = -1;
	G->nodos[nodo].queue.ndata++;
}

void SAT::bubble (qelem *r, long lo, long up)
{
    long i,j;
	qelem temp;
	for (i=lo+1;i<=up;i++)
	{
		j = i;
		while ((j > lo) && (r[j].dist > r[j - 1].dist))
		{
			temp = r[j];
			r[j] = r[j-1];
			r[--j] = temp;
		}
	}
}

qelem* SAT::sort (qelem *r, long lo, long up)
{
    long i, j;
	qelem tempr;
	while (lo < up)
	{
		i = lo;
		j = up;
		tempr = r[lo];
		while (i < j)
		{

			while (tempr.dist > r[j].dist) j--;
			r[i] = r[j];
			while ((i<j) && (tempr.dist <= r[i].dist)) i++;
			r[j] = r[i];
		}
		r[i] = tempr;
		if (i-lo < up-i)
		{ sort(r,lo,i-1); lo = i+1; }
		else { sort(r,i+1,up); up = i-1; }
	}
	bubble (r,lo,up);
	return r;
}

void SAT::distr (grafo *G, int n,int h)
{
    int i,j,ni;
	nodo *N = &G->nodos[n];
	qelem *q;
	N->queue.data = sort(N->queue.data, 0, N->queue.ndata - 1);
	if (N->queue.ndata != 0) {
		newvec(G, n, N->queue.data[0].elem);
		N = &G->nodos[n];
		N->maxd = N->queue.data[0].dist;
	}
	else {
		N->maxd = 0;
	}
	if (h >= MaxHeight) return;
	ni = 0;
	for (i=1;i<N->queue.ndata;i++)
	{
		q = &N->queue.data[i];
		for (j=0;j<N->vec.nnum;j++)
		{
			Tdist d = mydistance (N->queue.data[i].elem,G->nodos[N->vec.num[j]].obj);
			if (d <=q->dist)
			{
				q->dist = d; q->who = j;
			}
		}
		if (q->who == -1)
		{
			newvec (G,n,N->queue.data[i].elem);
			N = &G->nodos[n];
		}
		else
		{
			N->queue.data[ni++] = N->queue.data[i];
		}
	}

	for (i=0;i<ni;i++)
	{
		q = &N->queue.data[i];
		for (j=q->who+1;j<N->vec.nnum;j++)
		{
			Tdist d = mydistance (N->queue.data[i].elem,G->nodos[N->vec.num[j]].obj);
			if (d <=q->dist) { q->dist = d; q->who = j;}
		}
		newqueue (G,N->vec.num[q->who],q->elem,q->dist);
	}
	free (N->queue.data); N->queue.data = NULL; N->queue.ndata = -1;

	for (j=0;j<G->nodos[n].vec.nnum;j++)
	{

		distr (G,G->nodos[n].vec.num[j],h+1);
	}
}

void SAT::build(vector<Node> &dataset, int n)
{
    grafo *G = (grafo*)malloc (sizeof(grafo));
	int index=0;

	G->nodos = NULL; G->nnodos = 0;
	G->np =  DB.openDB(dataset);
	if (n && (n < G->np)) G->np = n;
	//i = 1;

	int iter = 3;
	int root = 0;
	int firstChild = 0;
	double maxDist = -1;
	vector<double> dists(G->np,0);
	srand((unsigned)time(NULL));

	for (int i = 0; i < iter; i++) {
		vector<double> tmpDists(G->np,0);
		int rootCand = rand()% G->np;
		index = 0;
		double maxDistCand = -1;
		int maxIdCand = -1;
		while (index < G->np){
			if (index == rootCand) {
				index++;
				continue;
			}
			double dist = mydistance(rootCand, index);
			tmpDists[index] = dist;
			if (dist > maxDistCand) {
				maxDistCand = dist;
				maxIdCand = index;
			}
			index++;
		}
		if (maxDistCand > maxDist) {
			maxDist = maxDistCand;
			root = rootCand;
			firstChild = maxIdCand;
			dists.insert(dists.begin(), tmpDists.begin(), tmpDists.end());
		}
		tmpDists=vector<double>();
	}

	newnode(G, root);

	index = 0;
	while (index < G->np)
	{

		if (index == root) {
			index++;
			continue;
		}
		newqueue (G,0, index, dists[index]);
		index++;
	}

	dists=vector<double>();
	distr (G,0,1);
	S=(Index)G;
}

void SAT::_search (grafo G, int n, float* obj, Tdist r, Tdist d0, Tdist mind,vector<PDI> *res)
{
    int j;
	nodo *N = &G.nodos[n];
	Tdist *dd;

	if(N->queue.ndata<0){
		if (N->vec.nnum == 0) {}
		if (d0 - r > N->maxd) return ;
		dd = (Tdist*)mymalloc(N->vec.nnum*sizeof(Tdist));

		for (j = 0; j<N->vec.nnum; j++)
		{
			dd[j] = myqdistance(obj, G.nodos[N->vec.num[j]].obj);
			if (dd[j]<mind) mind = dd[j];
		}
		for (j = 0; j<N->vec.nnum; j++)
		{
			if (dd[j] <= mind + 2 * r) {
				if (dd[j] <= r)
                    res->push_back({dd[j]*dd[j],G.nodos[N->vec.num[j]].obj});
                _search(G, N->vec.num[j], obj, r, dd[j], mind, res);
			}
		}
		free(dd);
		dd = NULL;
	}
	else {
		Tdist dist = 0;
		for (int i = 0; i < N->queue.ndata; i++) {
			dist = myqdistance(obj, N->queue.data[i].elem);
			if (dist <= r)
            {
                res->push_back({dist*dist,N->queue.data[i].elem});
            }
		}
	}
}

void SAT::rangequery(float* obj, Tdist r, vector<PDI> *res)
{
    grafo *G = (grafo*)S;
	Tdist d0 = myqdistance(obj,G->nodos[0].obj);
	if (d0 <= r)
    {
        res->push_back({d0*d0,G->nodos[0].obj});
    }
    _search (*G,0,obj,r,d0,d0,res);
}

Tdist SAT::knn(float* obj, int k, bool show=false)
{
    Tdist ret,lbound,dist;
	int j;
	grafo G = *((grafo*)S);
	nodo *N;
	Tdist *dd;
	heapElem *heap;
	int hsize = 0;
	heapElem hel;
	Tcelem res = createCelem(k);
	heap = (heapElem*)mymalloc (G.nnodos*sizeof(heapElem));
	hsize = 0;
	dd = (Tdist*)mymalloc (G.nnodos*sizeof(Tdist));
	dist = myqdistance(obj,G.nodos[0].obj);
	lbound = dist-G.nodos[0].maxd;
	if (lbound < 0) lbound = 0;
	addHeap (heap,&hsize,0,dist,lbound,dist);
	while (hsize > 0)
	{
		hel = extrHeap (heap, &hsize);
		if (outCelem(&res,hel.lbound)) break; /* pruning distance alcanzada */
		N = &G.nodos[hel.id];
		addCelem (&res,N->obj,hel.dist);

		if(N->queue.ndata<0){
			for (j = 0; j<N->vec.nnum; j++)
			{
				dd[j] = myqdistance(obj, G.nodos[N->vec.num[j]].obj);
				if (dd[j] < hel.mind) hel.mind = dd[j];
			}
			for (j = 0; j<N->vec.nnum; j++)
			{
				lbound = hel.lbound;
				if (lbound < (dd[j] - hel.mind) / 2) lbound = (dd[j] - hel.mind) / 2;
				if (lbound < dd[j] - G.nodos[N->vec.num[j]].maxd)
					lbound = dd[j] - G.nodos[N->vec.num[j]].maxd;
				addHeap(heap, &hsize, N->vec.num[j], dd[j], lbound, hel.mind);
			}
		}
		else {
			Tdist dist;
			for (j = 0; j < N->queue.ndata; j++) {
				dist = myqdistance(obj, N->queue.data[j].elem);
				if (!outCelem(&res, dist)) addCelem(&res, N->queue.data[j].elem, dist);
			}
		}

	}
	if (show) showCelem(&res);
	ret = radCelem(&res);
	free (dd); free (heap); freeCelem (&res);
	return ret;
}

#endif // SAT_HPP
