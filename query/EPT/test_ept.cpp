#include<iostream>
#include"./EPT.hpp"
#include"../Node.hpp"
#include<time.h>
#include<fstream>
using namespace std;
string dataid="audio";
string dir = "../../data_set/"+dataid+"/";
string dataset_file_name = dataid+"_afterpca.csv";
string queryset_file_name = dataid+"_uniform1000_afterpca.csv";
string k_out_file_name="audio_uniform1000_ept_myknn.csv";
vector<Node> dataset;
vector<Node> queryset;

vector<Node> read_dataset(string file_name)
{
    ifstream inf(file_name, ios::in);
    if(!inf.is_open())
    {
        cout<<file_name<<"文件不存在\n";
        exit(-1);
    }
    string tmp;
    vector<Node> res;
    int line = 0;
    while (getline(inf, tmp))
    {
        line++;
        vector<float> data;
        string tp = "";
        for (size_t i = 0; i < tmp.size(); i++)
        {
            if (tmp[i] == ',')
            {
                if (tp != "") data.push_back(stod(tp));
                tp = "";
            }
            else tp += tmp[i];
        }
        if (tp != "") data.push_back(stod(tp));
        if (res.size() && data.size() != res.back().size())
        {
            cout << file_name << "文件第" << line << "行与上一行列数不一样" << "\n";
            exit(-1);
        }
        res.push_back(Node(data,line-1));
    }
    inf.close();
    return res;
}

inline void test_rangequery(EPT &ept,double r)
{
    int sz=0;
    clock_t sum_time=0;
    #ifdef write
    ofstream ouf(dir+r_out_file_name,ios::out);
    #endif
    sum_time-=clock();
    for(size_t i=0;i<queryset.size();i++)
    {
        vector<int> *res=ept.rangequery(queryset[i].data.data(),r,0);
        sz+=res->size();
        #ifdef write
        sort(res->begin(),res->end(),[&](PDI a,PDI b)->bool{return a.se<b.se;});
        ouf<<res->size();
        for(PDI &idx:(*res)) ouf<<","<<idx.second;
        ouf<<"\n";
        #endif
    }
    sum_time+=clock();
    cout<<"r:"<<r<<" size:"<<sz<<"\n";
    cout<<"ave time="<<1.0*sum_time/queryset.size()<<"ms\n";
}

inline void test_knn(EPT &ept,int k)
{
    int sz=0;
    clock_t sum_time=0;
    ofstream ouf(dir+k_out_file_name,ios::out);
    sum_time-=clock();
    double radii=0;
    for(size_t i=0;i<queryset.size();i++)
    {
        priority_queue<PDI> q;
        double r=1e20;
        ept.knn(queryset[i].data.data(),k,q,r);
        //if(q.size()!=k) cout<<"error "<<q.size()<<" "<<k<<endl;
        radii+=r;
        sz+=q.size();
        while(q.size())
        {
            ouf<<q.top().second<<",";
            q.pop();
        }
        ouf<<"\n";
    }
    sum_time+=clock();
    cout<<"k:"<<k<<" ave radii:"<<radii/queryset.size()<<"\n";
    cout<<"ave time="<<1.0*sum_time/queryset.size()<<"ms\n";
    //system("pause");
}

int main()
{
    dataset=read_dataset(dir+dataset_file_name);
    queryset=read_dataset(dir+queryset_file_name);
    printf("read dataset queryset over\n");
    get_dim(dataset,0);
    EPT ept(dataset);
    cout<<"build over"<<"\n";
    vector<int> ks({20});
    for(int k:ks)
    {
        test_knn(ept,k);
        printf("\n\n");
    }
    vector<double> rs({33055,39128,42210,44364,46798});
    //vector<double> rs({0.564587,0.626831,0.656971,0.677176,0.69897});
    for(double r:rs)
    {
        test_rangequery(ept,r);
        printf("\n\n");
    }
    system("pause");
    return 0;
}
