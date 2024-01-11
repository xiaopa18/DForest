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
    int dim,num;
    vector<float> dataset;
    double memory_used()
    {
        return sizeof(int)*2+sizeof(float)*dataset.size();
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
        //dcmp++;
        #endif // CALC_DIST_COUNT
        return res;
    }
    enumeration(vector<Node> dataset){
        num=dataset.size();
        dim=dataset[0].dim;
        this->dataset.resize(num*dim);
        for(int i=0,cnt=0;i<dataset.size();i++)
        {
           // cout<<i<<"/"<<dataset.size()<<endl;
            for(int j=0;j<dim;j++)
                this->dataset[cnt++]=dataset[i].data[j];
        }
    }
    bool insert(Node p)
    {
        for(int i=0;i<dim;i++)
            dataset.push_back(p.data[i]);
        num++;
        return true;
    }
    bool del(int id)
    {
        dataset.erase(dataset.begin()+id*dim,dataset.begin()+(id+1)*dim);
        num--;
        return true;
    }
    vector<PDI>* range_query(float* q,double r)
    {
        double r2=r*r;
        vector<PDI> *res=new vector<PDI>();
        for(int i=0;i<num;i++)
        {
            double tmp=distance(q,dataset.data()+dim*i,dim);
            if(tmp>r2) continue;
            res->push_back({tmp,i});
        }
        return res;
    }
};
#endif // ENUMERATION_HPP

