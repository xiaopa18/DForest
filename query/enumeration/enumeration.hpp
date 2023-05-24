#ifndef ENUMERATION_HPP
#define ENUMERATION_HPP

#include<iostream>
#include"../Node.hpp"
#include"../myfunctions.hpp"
#include<vector>
#include<cmath>
using namespace std;
#ifdef CALC_DIST_COUNT
extern double dcmp;
#endif
#ifdef jianzh
extern int jianzhi;
#endif
struct enumeration
{
    int dim1,dim2,num;
    vector<float> dataset1,dataset2;
    vector<double> loss;
    vector<int> idmp;// map real id
    double memory_used()
    {
        return 12+4.0*(dataset1.size()+dataset2.size()+idmp.size())+8.0*loss.size();
    }
    double distance(float *a,float *b,int len)
    {
        double res=0,dx;
        for(int i=0;i<len;i++)
        {
            dx=a[i]-b[i];
            res+=dx*dx;
        }
        #ifdef CALC_DIST_COUNT
        dcmp++;
        #endif // CALC_DIST_COUNT
        return res;
    }
    enumeration(vector<Node> dataset){
        num=dataset.size();
        dim1=dataset[0].dim,dim2=dataset[0].data.size()-dim1;
        for(int i=0;i<dataset.size();i++)
        {
            idmp.push_back(dataset[i].idx);
            loss.push_back(dataset[i].loss[dim1]);
        }
        for(int i=0;i<dataset.size();i++)
            for(int j=0;j<dataset[i].data.size();j++)
                if(j<dim1) dataset1.push_back(dataset[i].data[j]);
                else dataset2.push_back(dataset[i].data[j]);
    }
    bool insert(Node p)
    {
        if(p.data.size()!=dim1+dim2) return false;
        idmp.push_back(p.idx);
        loss.push_back(p.loss[dim1]);
        for(int i=0;i<p.data.size();i++)
            if(i<dim1) dataset1.push_back(p.data[i]);
            else dataset2.push_back(p.data[i]);
        num++;
        return true;
    }
    bool del(int idx)
    {
        int id=0;
        while(id<num and idmp[id]!=idx) id++;
        if(id==num) return false;
        idmp.erase(idmp.begin()+id);
        loss.erase(loss.begin()+id);
        if(dim1) dataset1.erase(dataset1.begin()+id*dim1,dataset1.begin()+(id+1)*dim1);
        if(dim2) dataset2.erase(dataset2.begin()+id*dim2,dataset2.begin()+(id+1)*dim2);
        num--;
        return true;
    }
    vector<int>* range_query(float* q,double r,double qloss)
    {
        double r2=r*r;
        vector<int> *res=new vector<int>();
        for(int i=0;i<num;i++)
        {
            double tmp=distance(q,dataset1.data()+dim1*i,dim1);
            if(tmp>r2) continue;
            #ifndef not_pruning
            if(tmp+loss[i]+qloss<=r2)
            {
                #ifdef jianzh
                jianzhi++;
                #endif
                res->push_back(idmp[i]);
                continue;
            }
            #endif // not_pruning
            tmp+=distance(q+dim1,dataset2.data()+dim2*i,dim2);
            if(tmp<=r2) res->push_back(idmp[i]);
        }
        return res;
    }
    vector<PDI>* rangequery_knn(float* q,double r)
    {
        double r2=r*r;
        vector<PDI> *res=new vector<PDI>();
        for(int i=0;i<num;i++)
        {
            double tmp=distance(q,dataset1.data()+dim1*i,dim1);
            if(tmp>r2) continue;
            tmp+=distance(q+dim1,dataset2.data()+dim2*i,dim2);
            if(tmp<=r2) res->push_back({tmp,idmp[i]});
        }
        return res;
    }
    double knn(float *q,int k)
    {
        if(k>num) return 1e20;
        priority_queue<double> res;
        for(int i=0;i<num;i++)
        {
            double tmp=distance(q,dataset1.data()+dim1*i,dim1)+distance(q+dim1,dataset2.data()+dim2*i,dim2);
            res.push(tmp);
            if(res.size()>k) res.pop();
        }
        return res.top();
    }
    void knn(float *que,int k,priority_queue<PDI> &res,double &r)
    {
        double dis,r2=r*r;
        for(int i=0;i<num;i++)
        {
            dis=distance(que,dataset1.data()+i*dim1,dim1);
            if(dis>r2) continue;
            dis+=distance(que+dim1,dataset2.data()+i*dim2,dim2);
            if(dis>r2) continue;
            if(res.size()==k)
            {
                res.pop();
                res.push({dis,idmp[i]});
                r2=res.top().fi;
            }
            else
            {
                res.push({dis,idmp[i]});
                if(res.size()==k)
                {
                    r2=res.top().fi;
                }
            }
        }
        r=sqrt(r2);
    }
};
#endif // ENUMERATION_HPP
