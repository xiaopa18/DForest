#ifndef ENUMERATION_STORAGE_HPP
#define ENUMERATION_STORAGE_HPP
#include<fstream>
#include<ctime>
#include"../Node.hpp"
using namespace std;
extern double IOtime;

#ifdef CALC_PAGE
extern double page;
#endif // CALC_PAGE

#ifdef CALC_DIST_COUNT
extern double dcmp;
#endif

struct enumeration_storage
{
    ifstream inf1,inf2,inf3,inf4;
    int dim1,dim2,len,num;
    float *dataset1,*dataset2;
    double *loss;
    int *idmp;
    string index;
    enumeration_storage(vector<Node> dataset,string index,int page_size=4096)//unit:B
    {
        dim1=dataset[0].dim,dim2=dataset[0].data.size()-dim1;
        this->len=max(page_size/((dim1+dim2)*4+8+4),1);num=dataset.size();this->index=index;
        dataset1=new float[dim1*len];
        dataset2=new float[dim2*len];
        loss=new double[len];
        idmp=new int[len];
        ofstream ouf1(index+".d1",ios::out|ios::binary);
        ofstream ouf2(index+".d2",ios::out|ios::binary);
        ofstream ouf3(index+".mp",ios::out|ios::binary);
        ofstream ouf4(index+".ls",ios::out|ios::binary);
        if(!ouf1.is_open())
        {
            cout<<"open "<<index+".d1"<<" write is failed\n";
            exit(-1);
        }
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
        if(!ouf4.is_open())
        {
            cout<<"open "<<index+".ls"<<" write is failed\n";
            exit(-1);
        }
        for(int i=0;i<dataset.size();i++)
        {
            ouf3.write((char*)(&dataset[i].idx),4);
            ouf4.write((char*)(&dataset[i].loss[dim1]),8);
            for(int j=0;j<dataset[i].data.size();j++)
                if(j<dim1) ouf1.write((char*)(&dataset[i].data[j]),4);
                else ouf2.write((char*)(&dataset[i].data[j]),4);
        }
        ouf1.close();
        ouf2.close();
        ouf3.close();
        ouf4.close();
        inf1.open(index+".d1",ios::in|ios::binary);
        inf2.open(index+".d2",ios::in|ios::binary);
        inf3.open(index+".mp",ios::in|ios::binary);
        inf4.open(index+".ls",ios::in|ios::binary);
        if(!inf1.is_open())
        {
            cout<<"open "<<index+".d1"<<" read is failed\n";
            exit(-1);
        }
        if(!inf2.is_open())
        {
            cout<<"open "<<index+".d2"<<" read is failed\n";
            exit(-1);
        }
        if(!inf3.is_open())
        {
            cout<<"open "<<index+".mp"<<" read is failed\n";
            exit(-1);
        }
        if(!inf4.is_open())
        {
            cout<<"open "<<index+".ls"<<" read is failed\n";
            exit(-1);
        }
    }
    ~enumeration_storage()
    {
        delete[] dataset1;
        delete[] dataset2;
        delete[] idmp;
    }
    bool insert(Node p)
    {
        if(p.data.size()!=dim1+dim2) return false;
        inf1.close();
        inf2.close();
        inf3.close();
        ofstream ouf1(index+".d1",ios::app|ios::binary);
        ofstream ouf2(index+".d2",ios::app|ios::binary);
        ofstream ouf3(index+".mp",ios::app|ios::binary);
        ofstream ouf4(index+".ls",ios::out|ios::binary);
        if(!ouf1.is_open())
        {
            cout<<"open "<<index+".d1"<<" write is failed\n";
            exit(-1);
        }
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
        if(!ouf4.is_open())
        {
            cout<<"open "<<index+".ls"<<" write is failed\n";
            exit(-1);
        }
        ouf3.write((char*)&p.idx,4);
        ouf4.write((char*)(&p.loss[dim1]),8);
        for(int i=0;i<p.data.size();i++)
            if(i<dim1) ouf1.write((char*)&p.data[i],4);
            else ouf2.write((char*)&p.data[i],4);
        ouf1.close();
        ouf2.close();
        ouf3.close();
        ouf4.close();
        inf1.open(index+".d1",ios::in|ios::binary);
        inf2.open(index+".d2",ios::in|ios::binary);
        inf3.open(index+".mp",ios::in|ios::binary);
        inf4.open(index+".ls",ios::in|ios::binary);
        if(!inf1.is_open())
        {
            cout<<"open "<<index+".d1"<<" read is failed\n";
            exit(-1);
        }
        if(!inf2.is_open())
        {
            cout<<"open "<<index+".d2"<<" read is failed\n";
            exit(-1);
        }
        if(!inf3.is_open())
        {
            cout<<"open "<<index+".mp"<<" read is failed\n";
            exit(-1);
        }
        if(!inf4.is_open())
        {
            cout<<"open "<<index+".ls"<<" read is failed\n";
            exit(-1);
        }
        num++;
        return true;
    }

    bool del(int idx)
    {
        int idmp[num];
        inf3.seekg(0,ios::beg);
        inf3.read((char*)idmp,4*num);
        int id=0;
        while(id<num and idmp[id]!=idx) id++;
        if(id==num) return false;
        inf1.seekg(0,ios::beg);
        inf2.seekg(0,ios::beg);
        float dataset1[num*dim1];
        float dataset2[num*dim2];
        double loss[num];
        inf1.read((char*)dataset1,4*num*dim1);
        inf2.read((char*)dataset2,4*num*dim2);
        inf4.read((char*)loss,8*num);
        inf1.close();
        inf2.close();
        inf3.close();
        inf4.close();
        ofstream ouf1(index+".d1",ios::out|ios::binary);
        ofstream ouf2(index+".d2",ios::out|ios::binary);
        ofstream ouf3(index+".mp",ios::out|ios::binary);
        ofstream ouf4(index+".ls",ios::out|ios::binary);
        if(!ouf1.is_open())
        {
            cout<<"open "<<index+".d1"<<" write is failed\n";
            exit(-1);
        }
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
        if(!ouf4.is_open())
        {
            cout<<"open "<<index+".ls"<<" write is failed\n";
            exit(-1);
        }
        for(int i=0;i<num;i++)
        {
            if(i==id) continue;
            ouf3.write((char*)&idmp[i],4);
            ouf4.write((char*)&loss[i],8);
            for(int j=0;j<dim1;j++)
                ouf1.write((char*)&dataset1[i*dim1+j],4);
            for(int j=0;j<dim2;j++)
                ouf2.write((char*)&dataset2[i*dim1+j],4);
        }
        ouf1.close();
        ouf2.close();
        ouf3.close();
        ouf4.close();
        inf1.open(index+".d1",ios::in|ios::binary);
        inf2.open(index+".d2",ios::in|ios::binary);
        inf3.open(index+".mp",ios::in|ios::binary);
        inf4.open(index+".ls",ios::in|ios::binary);
        if(!inf1.is_open())
        {
            cout<<"open "<<index+".d1"<<" read is failed\n";
            exit(-1);
        }
        if(!inf2.is_open())
        {
            cout<<"open "<<index+".d2"<<" read is failed\n";
            exit(-1);
        }
        if(!inf3.is_open())
        {
            cout<<"open "<<index+".mp"<<" read is failed\n";
            exit(-1);
        }
        if(!inf4.is_open())
        {
            cout<<"open "<<index+".ls"<<" read is failed\n";
            exit(-1);
        }
        num--;
        return true;
    }

    double distance(float *a,float *b,int len)
    {
        double res=0,dx;
        #ifdef CALC_DIST_COUNT
        dcmp++;
        #endif
        for(int i=0;i<len;i++)
        {
            dx=a[i]-b[i];
            res+=dx*dx;
        }
        return res;
    }
    vector<int>* rangequery(float *q,double r,double qloss)
    {
        vector<int> *res=new vector<int>();
        double r2=r*r;
        inf1.seekg(0,ios::beg);
        inf2.seekg(0,ios::beg);
        inf3.seekg(0,ios::beg);
        inf4.seekg(0,ios::beg);
        for(int bk=0;bk<(num+len-1)/len;bk++)
        {
            int leng=min(len,num-bk*len);
            IOtime-=clock();
            inf1.read((char*)dataset1,leng*dim1*4);
            inf2.read((char*)dataset2,leng*dim2*4);
            inf3.read((char*)idmp,leng*4);
            inf4.read((char*)loss,leng*8);
            #ifdef CALC_PAGE
            page++;
            #endif // CALC_PAGE
            IOtime+=clock();
            for(int i=0;i<leng;i++)
            {
                double dis=distance(q,dataset1+i*dim1,dim1);
                if(dis>r2) continue;
                if(dis+loss[i]+qloss<=r2)
                {
                    res->push_back(idmp[i]);
                    continue;
                }
                dis+=distance(q+dim1,dataset2+i*dim2,dim2);
                if(dis>r2) continue;
                res->push_back(idmp[i]);
            }
        }
        return res;
    }
    double knn(float *q,int k)
    {
        if(k>num) return 1e20;
        priority_queue<double> res;
        inf1.seekg(0,ios::beg);
        inf2.seekg(0,ios::beg);
        inf3.seekg(0,ios::beg);
        inf4.seekg(0,ios::beg);
        for(int bk=0;bk<(num+len-1)/len;bk++)
        {
            int leng=min(len,num-bk*len);
            IOtime-=clock();
            inf1.read((char*)dataset1,leng*dim1*4);
            inf2.read((char*)dataset2,leng*dim2*4);
            inf3.read((char*)idmp,leng*4);
            inf4.read((char*)loss,leng*8);
            #ifdef CALC_PAGE
            page++;
            #endif // CALC_PAGE
            IOtime+=clock();
            for(int i=0;i<leng;i++)
            {
                double dis=distance(q,dataset1+i*dim1,dim1)+distance(q+dim1,dataset2+i*dim2,dim2);
                res.push(dis);
                if(res.size()>k) res.pop();
            }
        }
        return res.top();
    }
    vector<PDI>* rangequery_knn(float *q,double r)
    {
        vector<PDI> *res=new vector<PDI>();
        double r2=r*r;
        inf1.seekg(0,ios::beg);
        inf2.seekg(0,ios::beg);
        inf3.seekg(0,ios::beg);
        inf4.seekg(0,ios::beg);
        for(int bk=0;bk<(num+len-1)/len;bk++)
        {
            int leng=min(len,num-bk*len);
            IOtime-=clock();
            inf1.read((char*)dataset1,leng*dim1*4);
            inf2.read((char*)dataset2,leng*dim2*4);
            inf3.read((char*)idmp,leng*4);
            inf4.read((char*)loss,leng*8);
            #ifdef CALC_PAGE
            page++;
            #endif // CALC_PAGE
            IOtime+=clock();
            for(int i=0;i<leng;i++)
            {
                double dis=distance(q,dataset1+i*dim1,dim1);
                if(dis>r2) continue;
                dis+=distance(q+dim1,dataset2+i*dim2,dim2);
                if(dis>r2) continue;
                res->push_back({dis,idmp[i]});
            }
        }
        return res;
    }
    void knn(float *que,int k,priority_queue<PDI> &res,double &r)
    {
        double dis,r2=r*r;
        inf1.seekg(0,ios::beg);
        inf2.seekg(0,ios::beg);
        inf3.seekg(0,ios::beg);
        for(int bk=0;bk<(num+len-1)/len;bk++)
        {
            int leng=min(len,num-bk*len);
            IOtime-=clock();
            inf1.read((char*)dataset1,leng*dim1*4);
            inf2.read((char*)dataset2,leng*dim2*4);
            inf3.read((char*)idmp,leng*4);
            #ifdef CALC_PAGE
            page++;
            #endif // CALC_PAGE
            IOtime+=clock();
            for(int i=0;i<leng;i++)
            {
                double dis=distance(que,dataset1+i*dim1,dim1);
                if(dis>r2) continue;
                dis+=distance(que+dim1,dataset2+i*dim2,dim2);
                if(dis>r2) continue;
                if(res.size()==k)
                {
                    res.pop();
                    res.push({dis,idmp[i]});
                    r2=res.top().fi;
                    r=sqrt(r2);
                }
                else
                {
                    res.push({dis,idmp[i]});
                    if(res.size()==k)
                    {
                        r2=res.top().fi;
                        r=sqrt(r2);
                    }
                }
            }
        }
    }
};

#endif
