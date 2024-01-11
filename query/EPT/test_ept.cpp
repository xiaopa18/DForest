#include<iostream>
#include"./EPT.hpp"
#include"../Node.hpp"
#include<time.h>
#include<fstream>
#include<chrono>
using namespace std;
using namespace std::chrono;
string dataid="notre_ins";
string dir = "../../../data_set/"+dataid+"/";
string dataset_file_name = dataid+"_afterpca.csv";
string queryset_file_name = dataid+"_uniform1000_afterpca.csv";
string insert_file="insert10000_afterpca.csv";
vector<Node> dataset;
vector<Node> queryset,insertset;
double r=263;
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

int main()
{
    dataset=read_dataset(dir+dataset_file_name);
    queryset=read_dataset(dir+queryset_file_name);
    insertset=read_dataset(dir+insert_file);
    printf("read dataset queryset over\n");
    get_dim(dataset,0);
    EPT ept(dataset);
    cout<<"build over"<<"\n";
    auto tist=steady_clock::now(),tied=steady_clock::now();
    ofstream ouf("./insert.csv",ios::out|ios::app);
    ouf.setf(ios::fixed);
    for(int j=0;j<2000;j++) ept.insert(insertset[j]);
    for(int i=0;i<5;i++)
    {
        tist=steady_clock::now();
        for(int j=i*2000;j<(i+1)*2000;j++) ept.insert(insertset[j]);
        tied=steady_clock::now();
        double instim=duration_cast<microseconds>(tied - tist).count()/1000.0/10000;
        double quetim=0;
        for(Node &q:queryset)
        {
            tist=steady_clock::now();
            auto res=ept.knn(q.data.data(),10);
            tied=steady_clock::now();
            quetim+=duration_cast<microseconds>(tied - tist).count()/1000.0;
        }

        quetim/=queryset.size();

        ouf<<dataid<<",EPT,"<<(i+1)*2000<<","<<instim<<","<<quetim<<endl;
    }
    return 0;
}
