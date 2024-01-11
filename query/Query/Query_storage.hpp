#ifndef QUERY_STORAGE_HPP
#define QUERY_STORAGE_HPP
#include"../Node.hpp"
#include"../enumeration/enumeration.hpp"
#include"../enumeration_storage/enumeration_storage.hpp"
#include"../EPT_storage/EPT_storage.hpp"
#include<chrono>
#include<vector>
#include<ctime>
#define fi first
#define se second
using namespace std;
using namespace std::chrono;
typedef pair<double,int> PDI;

#ifdef CALC_DIST_COUNT
extern double dcmp;
#endif

union Tree_storage
{
    EPT_storage *ept;
    enumeration_storage *enu;
};

struct Query_storage
{
    Tree_storage tree;
    string type;
    int dim1,dim2,len;
    double e2_16;
    string index;
    ifstream d2inf,idinf;
    float* dataset2;//reduce dim
    int* idmp;//map real id
    Loss loss;
    Query_storage(vector<Node> dataset,double e,string index,int page_size=4096):type("null")
    {
        e2_16=e/16;
        this->dim1=dataset[0].dim,dim2=dataset[0].data.size()-dim1;
        this->index=index;

        {
            EPT_storage *ept;
            ept=new EPT_storage(dataset,index,page_size);
            tree.ept=ept;
            len=ept->len;
            type="ept";
        }
        dataset2=new float[len*dim2];
        idmp=new int[len];
        ofstream ouf2(index+".d2",ios::out|ios::binary);
        ofstream ouf3(index+".mp",ios::out|ios::binary);
        if(!ouf2.is_open())
        {
            cout<<"open "<<index+".d2"<<" write is failed\n";
            exit(-1);
        }
        if(!ouf3.is_open())
        {
            cout<<"open "<<index+".mp"<<" write is failed\n";
            exit(-1);
        }
        for(int i=0;i<dataset.size();i++)
        {
            ouf3.write((char*)(&dataset[i].idx),4);
            for(int j=1;j<=16;j++)
                if(dataset[i].loss[dim1]<=e2_16*j)
                {
                    loss.insert(j);
                    break;
                }
            for(int j=dim1;j<dataset[i].data.size();j++)
                ouf2.write((char*)(&dataset[i].data[j]),4);
        }
        ouf2.close();
        ouf3.close();
        d2inf.open(index+".d2",ios::in|ios::binary);
        idinf.open(index+".mp",ios::in|ios::binary);
        if(!d2inf.is_open())
        {
            cout<<"open "<<index+".d2"<<" read is failed\n";
            exit(-1);
        }
        if(!idinf.is_open())
        {
            cout<<"open "<<index+".mp"<<" read is failed\n";
            exit(-1);
        }
    }
    bool insert(Node p)
    {
        if(p.data.size()!=dim1+dim2) return false;
        d2inf.close();
        idinf.close();
        ofstream ouf2(index+".d2",ios::app|ios::binary);
        ofstream ouf3(index+".mp",ios::app|ios::binary);
        if(!ouf2.is_open())
        {
            cout<<"open "<<index+".d2"<<" write is failed\n";
            exit(-1);
        }
        if(!ouf3.is_open())
        {
            cout<<"open "<<index+".mp"<<" write is failed\n";
            exit(-1);
        }
        ouf3.write((char*)&p.idx,4);
        for(int i=1;i<=16;i++)
            if(i*e2_16>=p.loss[dim1])
                loss.insert(i);
        for(int i=dim1;i<dim1+dim2;i++)
            ouf2.write((char*)&p.data[i],4);
        ouf2.close();
        ouf3.close();
        d2inf.open(index+".d2",ios::in|ios::binary);
        idinf.open(index+".mp",ios::in|ios::binary);
        if(!d2inf.is_open())
        {
            cout<<"open "<<index+".d2"<<" read is failed\n";
            exit(-1);
        }
        if(!idinf.is_open())
        {
            cout<<"open "<<index+".mp"<<" read is failed\n";
            exit(-1);
        }
        if(type=="ept") return tree.ept->insert(p);
        return tree.enu->insert(p);
    }
    bool del(int idx)
    {
        int num=get_num();
        int idmp[num];
        idinf.seekg(0,ios::beg);
        idinf.read((char*)idmp,num*4);
        int id=0;
        while(id<num and idmp[id]!=idx) id++;
        if(id==num) return false;
        d2inf.seekg(0,ios::beg);
        float *dataset2=new float[num*dim2];
        d2inf.read((char*)dataset2,4*num*dim2);
        idinf.close();
        d2inf.close();
        ofstream ouf2(index+".d2",ios::out|ios::binary);
        ofstream ouf3(index+".mp",ios::out|ios::binary);
        if(!ouf2.is_open())
        {
            cout<<"open "<<index+".d2"<<" write is failed\n";
            exit(-1);
        }
        if(!ouf3.is_open())
        {
            cout<<"open "<<index+".mp"<<" write is failed\n";
            exit(-1);
        }
        loss.del(id);
        ouf2.close();
        ouf3.close();
        d2inf.open(index+".d2",ios::in|ios::binary);
        idinf.open(index+".mp",ios::in|ios::binary);
        if(!d2inf.is_open())
        {
            cout<<"open "<<index+".d2"<<" read is failed\n";
            exit(-1);
        }
        if(!idinf.is_open())
        {
            cout<<"open "<<index+".mp"<<" read is failed\n";
            exit(-1);
        }
        delete[] dataset2;
        if(type=="ept") return tree.ept->del(id);
        return tree.enu->del(id);
    }
    vector<int>* rangequery(float *q,double r, double qloss)
    {
        int num=get_num();
        double r2=r*r;
        vector<PDI>* ans;
        vector<int>* res=new vector<int>();
        if(type=="ept") ans = tree.ept->rangequery(q,r);
        if(dim2) d2inf.seekg(0,ios::beg);
        idinf.seekg(0,ios::beg);
        for(int bk=0,now=0;now<ans->size() and bk<(num+len-1)/len;bk++)
        {
            int leng=min(len,num-bk*len);
            IOtime-=clock();
            if(dim2) d2inf.read((char*)dataset2,leng*dim2*4);
            idinf.read((char*)idmp,leng*4);
            IOtime+=clock();
            while(now<ans->size() and (*ans)[now].se<len*bk+leng)
            {
                if((*ans)[now].fi+(loss.query((*ans)[now].se)*e2_16+qloss)*(loss.query((*ans)[now].fi)*e2_16+qloss)<=r2)
                {
                    res->push_back(idmp[(*ans)[now].se-bk*len]);
                    now++;
                    continue;
                }
                (*ans)[now].fi+=distance(q+dim1,dataset2+((*ans)[now].se-bk*len)*dim2);
                if((*ans)[now].fi<=r2) res->push_back(idmp[(*ans)[now].se-bk*len]);
                now++;
            }
        }
        delete ans;
        return res;
    }
    void rangequery_knn(float *q,int k,priority_queue<PDI> &res,double r)
    {
        vector<PDI> *ans;
        int num=get_num();
        double r2=r*r;
        //cout<<r<<endl;
        if(type=="ept") ans=tree.ept->rangequery(q,r);
        if(dim2) d2inf.seekg(0,ios::beg);
        idinf.seekg(0,ios::beg);
       // cout<<"query ok"<<endl;
        for(int bk=0,now=0;now<ans->size() and bk<(num+len-1)/len;bk++)
        {
            //cout<<bk<<endl;
            int leng=min(len,num-bk*len);
            IOtime-=clock();
            if(dim2) d2inf.read((char*)dataset2,leng*dim2*4);
            idinf.read((char*)idmp,leng*4);
            IOtime+=clock();
            //cout<<"read ok"<<"\n";
            while(now<ans->size() and (*ans)[now].se<len*bk+leng)
            {
               // cout<<now<<"/"<<ans->size()<<endl;
                if((*ans)[now].fi>r2)
                {
                    now++;
                    continue;
                }
                (*ans)[now].fi+=distance(q+dim1,dataset2+((*ans)[now].se-bk*len)*dim2);
                if((*ans)[now].fi>r2)
                {
                    now++;
                    continue;
                }
                (*ans)[now].se=idmp[(*ans)[now].se-bk*len];
                res.push((*ans)[now]);
                if(res.size()>k)
                {
                    res.pop();
                    r2=res.top().fi;
                }
                now++;
            }
        }
        delete ans;
    }
    int get_num()
    {
        if(type=="ept") return tree.ept->num;
    }
    double distance(float *a,float *b)
    {
        double res=0,dx;
        #ifdef CALC_DIST_COUNT
        dcmp++;
        #endif
        for(int i=0;i<dim2;i++)
        {
            dx=a[i]-b[i];
            res+=dx*dx;
        }
        return res;
    }
};

void thread_storage_query_function(Query_storage *qp,float *q,double r,double qloss,vector<int> **res)
{
    *res=qp->rangequery(q,r,qloss);
}

#endif // QUERY_STORAGE_HPP
