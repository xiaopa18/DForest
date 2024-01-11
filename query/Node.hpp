#ifndef NODE_H_
#define NODE_H_
#include<vector>
#include<cmath>
using namespace std;

struct Node
{
    constexpr static double eps=1e-5;
    vector<float> data;
    vector<double> loss;
    int idx,dim;
    double rou;
    auto begin() const
    {
        return data.begin();
    }
    auto end() const
    {
        return data.begin() + dim;
    }
    size_t size()
    {
        return data.size();
    }
    Node(vector<float> dt,int id):data(dt),idx(id){
        dim = (int)dt.size();
    }
    void get_dim(double e)
    {
        double loss=0;
        double e2=e*e;
        int idx=(int)data.size();
        while(idx)
        {
            if(loss+data[idx-1]*data[idx-1]>e2) break;
            loss+=data[idx-1]*data[idx-1];
            idx--;
        }
        dim=max(idx,1);
        this->loss.resize(data.size()+1);
        this->loss[data.size()]=0;
        for(int i=data.size()-1;~i;i--)
            this->loss[i]=sqrt(this->loss[i+1]*this->loss[i+1]+data[i]*data[i]);
        rou=0;
        for(int i=dim-2;~i;i--)
        {
            //rou=min(rou,(this->loss[i]-this->loss[dim-1])/(dim-1-i+1));
            rou+=(this->loss[i]-this->loss[dim-1])/(dim-1-i+1);
        }
        //if(fabs(rou-1e9)<eps) rou=0;
        if(rou!=0) rou/=dim-1;
    }
    Node(){}
    bool operator<(const Node &t)const{
        for(int i=0;i<dim;i++)
            if(fabs(data[i]-t.data[i])>eps)
                return data[i]<t.data[i];
        return false;
    }
    bool operator==(const Node &t)const{
        for(int i=0;i<dim;i++)
            if(fabs(data[i]-t.data[i])>eps)
                return false;
        return true;
    }
};

inline void get_dim(vector<Node> &dataset,double e)
{
    for(size_t i=0;i<dataset.size();i++)
        dataset[i].get_dim(e);
}

#endif
