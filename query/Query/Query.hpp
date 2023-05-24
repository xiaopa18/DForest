#ifndef QUERY_H_
#define QUERY_H_
#include"../Node.hpp"
#include "../EPT/EPT.hpp"
#include "../SAT/sat.hpp"
#include "../GNAT/GNAT.hpp"
#include"../enumeration/enumeration.hpp"
#include"../myfunctions.hpp"
#include"../monitoring.hpp"
#include<vector>
#include<time.h>
#include<queue>
#include<cstdlib>
#include<chrono>
#include<mutex>
#define fi first
#define se second
extern int mtree_mn,gnat_arity;
using namespace std;
using namespace std::chrono;
typedef pair<double,int> PDI;
#ifdef CALC_DIST_COUNT
extern double dcmp;
#endif
#ifdef jianzh
extern int jianzhi;
#endif
union Tree
{
    EPT *ept;
    SAT *sat;
    enumeration *enu;
    GNAT_t *gnat;
};

struct Query
{
    Tree tree;
    string type;
    int dim1,dim2;
    vector<float> dataset2;//reduce dim
    Loss loss;
    double e2_16;
    vector<int> idmp;//map real id
    Query(const Query &qe):tree(qe.tree),type(qe.type){}

    Query(vector<Node> dataset,double e):type("null")
    {
        dim1=dataset[0].dim,dim2=dataset[0].data.size()-dim1;
        e2_16=e/16;
        this->dataset2.resize(dataset.size()*dim2);
        for(int i=0,id2=0;i<dataset.size();i++)
        {
            idmp.push_back(dataset[i].idx);
            for(int j=1;j<=16;j++)
                if(dataset[i].loss[dim1]<=e2_16*j)
                {
                    loss.insert(j);
                    break;
                }
            for(int j=dim1;j<dim1+dim2;j++)
                this->dataset2[id2++]=dataset[i].data[j];
        }
        if(dataset.size()<10 or dataset[0].dim==0)
        {
            enumeration *enu=new enumeration(dataset);
            tree.enu=enu;
            type="enum";
        }
        #if defined(USE_SAT)
        else
        {
            tree.sat=new SAT(dataset);
            type="sat";
        }
        #elif defined(USE_GNAT)
        else
        {
            tree.gnat=new GNAT_t(dataset,10);
            type="gnat";
        }
        #else
        else
        {
            tree.ept=new EPT(dataset);
            type="ept";
        }
        #endif // USE_SAT
    }

    double memory_used()
    {
        double mem=sizeof(void*)+type.size()+16+dataset2.size()*4.0+
                loss.get_size()+idmp.size()*4.0;
        if(type=="ept") mem+=tree.ept->memory_used();
        else if(type=="sat") mem+=tree.sat->memory_used();
        else if(type=="gnat") mem+=tree.gnat->memory_used();
        else mem+=tree.enu->memory_used();
        return mem;
    }

    bool insert(Node p)
    {
        if(p.data.size()!=dim1+dim2) return false;
        for(int i=0;i<dim2;i++) dataset2.push_back(p.data[dim1+i]);
        for(int i=1;i<=16;i++)
            if(i*e2_16>=p.loss[dim1])
                loss.insert(i);
        idmp.push_back(p.idx);
        if(type=="ept") return tree.ept->insert(p);
        return tree.enu->insert(p);
    }

    bool del(int idx)
    {
        int id=0;
        while(id<get_num() and idmp[id]!=idx) id++;
        if(id==get_num()) return false;
        if(dim2) dataset2.erase(dataset2.begin()+id*dim2,dataset2.begin()+(id+1)*dim2);
        idmp.erase(idmp.begin()+id);
        loss.del(id);
        if(type=="ept") return tree.ept->del(idx);
        return tree.enu->del(idx);
    }

    void rangequery_knn(float *q,int k,priority_queue<PDI> &res,double r)
    {
        vector<PDI> *ans;
        double r2=r*r;
        if(type=="ept") ans=tree.ept->rangequery(q,r);
        else if(type=="sat")
        {
            ans=new vector<PDI>();
            tree.sat->rangequery(q,r,ans);
        }
        else if(type=="gnat") ans=tree.gnat->rangequery(q,r);
        for(PDI &tmp:(*ans))
        {
            if(tmp.fi>r2) continue;
            tmp.fi+=distance(q+dim1,dataset2.data()+tmp.se*dim2);
            if(tmp.fi>r2) continue;
            tmp.se=idmp[tmp.se];
            res.push(tmp);
            if(res.size()>k)
            {
                res.pop();
                r2=res.top().fi;
            }
        }
        delete ans;
    }

    vector<int>* rangequery(float *q,double r,double qloss)
    {
        vector<PDI> *ans;
        vector<int> *res=new vector<int>();
        double r2=r*r;
        if(type=="ept") ans=tree.ept->rangequery(q,r);
        else if(type=="sat")
        {
            ans=new vector<PDI>();
            tree.sat->rangequery(q,r,ans);
        }
        else if(type=="gnat") ans=tree.gnat->rangequery(q,r);
        for(PDI &tmp:(*ans))
        {
            #ifndef not_pruning
            if(tmp.fi+(loss.query(tmp.se)*e2_16+qloss)*(loss.query(tmp.se)*e2_16+qloss)<=r2)
            {
                #ifdef jianzh
                jianzhi++;
                #endif
                res->push_back(idmp[tmp.se]);
                continue;
            }
            #endif // not_pruning
            tmp.fi+=distance(q+dim1,dataset2.data()+tmp.se*dim2);
            if(tmp.fi<=r2) res->push_back(idmp[tmp.se]);
        }
        delete ans;
        return res;
    }

    double distance(float *a,float *b)
    {
        double res=0,dx;
        for(int i=0;i<dim2;i++)
        {
            dx=a[i]-b[i];
            res+=dx*dx;
        }
        #ifdef CALC_DIST_COUNT
        dcmp++;
        #endif // CALC_DIST_COUNT
        return res;
    }

    int get_num()
    {
        if(type=="ept") return tree.ept->num;
        return tree.enu->num;
    }

    ~Query()
    {
        if(type=="ept") delete tree.ept;
        else if(type=="enum") delete tree.enu;
        else if(type=="sat") delete tree.sat;
        else if(type=="gnat") delete tree.gnat;
    }


};

void thread_rangequery_function(Query *qp,float *q,double r,double qloss,vector<int> **res)
{
    *res=qp->rangequery(q,r,qloss);
}

void delete_forest(vector<Query*> &qs)
{
    for(size_t i=0;i<qs.size();i++)
        if(qs[i]!=nullptr)
            delete qs[i];
}

int select_query_r(int dim,vector<Query*> &qs)
{
    int l=0,r=qs.size()-1;
    while(l<r)
    {
        int mid=l+r>>1;
        if(qs[mid]->dim1>=dim) r=mid;
        else l=mid+1;
    }
    return r;
}

int select_query_l(int dim,vector<Query*> &qs)
{
    int l=0,r=qs.size()-1;
    while(l<r)
    {
        int mid=l+r+1>>1;
        if(qs[mid]->dim1<=dim) l=mid;
        else r=mid-1;
    }
    return l;
}
#endif
