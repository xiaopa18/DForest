#ifndef EPT_HPP
#define EPT_HPP
#define fi first
#define se second
#include"../Node.hpp"
#include<cmath>
#include<algorithm>
#include<queue>
#include<vector>
#include<unordered_set>
#include<random>
#include<set>
#include"../Loss/Loss.hpp"
#define MAXREAL 1e20
using namespace std;
typedef pair<double,int> PDI;
#ifdef CALC_DIST_COUNT
extern double dcmp;
#endif // CALC_DIST_COUNT
//#define lazy
struct EPT
{
    int num_cand;//the number of candidate
    int LGroup;//pivot number
    int num;
    int dim1;
    vector<float> dataset1;//index
    vector<int> pivId;
    vector<double> dist;
    bool* ispivot;
    double** O_P_matrix;
    int* cand;
    int sampleSize;
    float* samples_objs;
    #ifdef lazy
    unordered_set<int> lazy_update;
    #endif // lazy
    EPT(vector<Node> &dataset,int LGroup,int num_cand);
    ~EPT();
    void sample_data(int nums, vector<Node> &dataset);
    double** MaxPrunning(int num);
    int PivotSelect(int objId, int o_num, int q_num, int pivotNum);
    double distance(float *a,float *b,int len);
    bool insert(Node p);
    bool del(int idx);
    vector<PDI>* rangequery(float *que,double r);
    priority_queue<PDI> knn(float *que,int k);
    double memory_used()
    {
        return 20+4.0*(dataset1.size()+pivId.size())+8.0*dist.size()
                +4.0*sampleSize*dim1+num_cand*8;
    }
};

vector<PDI>* EPT::rangequery(float *que,double r)
{
    vector<PDI> *ans=new vector<PDI>();
    double dis,r2=r*r,pivotsquery[num_cand];
    for (int i = 0; i < num_cand; ++i)
	{
		if (ispivot[i])
		{
			pivotsquery[i] = sqrt(distance(que, samples_objs+cand[i]*dim1,dim1));
		}
	}
	bool next = false;
	int pos=-LGroup;
    for(int i=0;i<num;i++)
    {
        #ifdef lazy
        if(lazy_update.count(i)) continue;
        #endif // lazy
        pos+=LGroup;
        next = false;
        for(int j=0;j<LGroup;j++)
        {
            if(fabs(pivotsquery[pivId[pos+j]]-dist[pos+j]) > r)
            {
                next=true;
                break;
            }
        }
        if(!next)
        {
            dis=distance(que,dataset1.data()+i*dim1,dim1);
            if(dis<=r2) ans->push_back({dis,i});
        }
    }
    return ans;
}

priority_queue<PDI> EPT::knn(float *que,int k)
{
    priority_queue<PDI> res;
    double dis,r2=1e18,pivotsquery[num_cand],r=1e18;
    for (int i = 0; i < num_cand; ++i)
	{
		if (ispivot[i])
		{
			pivotsquery[i] = sqrt(distance(que, samples_objs+cand[i]*dim1,dim1));
		}
	}
	bool next = false;
	int pos=-LGroup;
    for(int i=0;i<num;i++)
    {
        #ifdef lazy
        if(lazy_update.count(i)) continue;
        #endif // lazy
        pos+=LGroup;
        next = false;
        for(int j=0;j<LGroup;j++)
        {
            if(fabs(pivotsquery[pivId[pos+j]]-dist[pos+j]) > r)
            {
                next=true;
                break;
            }
        }
        if(!next)
        {
            dis=distance(que,dataset1.data()+i*dim1,dim1);
            if(dis>r2) continue;
            res.push({dis,i});
            if(res.size()==k+1)
            {
                res.pop();
                r2=res.top().first;
                r=sqrt(r2);
            }
        }
    }
    return res;
}

double EPT::distance(float *a,float *b,int len)
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

EPT::EPT(vector<Node> &dataset,int LGroup=5,int num_cand=40)
{
    num=dataset.size();
    this->LGroup=LGroup;
    this->num_cand=num_cand;
    pivId.resize(LGroup*num);
    dist.resize(LGroup*num);
    dim1=dataset[0].dim;
    this->dataset1.resize(dataset.size()*dim1);
    for(int i=0,id1=0,id2=0;i<num;i++)
        for(int j=0;j<dim1;j++)
            this->dataset1[id1++]=dataset[i].data[j];
    sampleSize = max((int)dataset.size() / 200,min(num_cand,(int)dataset.size()));
    samples_objs=new float[sampleSize*dim1];
	sample_data(sampleSize, dataset);
    ispivot = new bool[num_cand];
	for (int i = 0; i < num_cand; i++)
		ispivot[i] = false;
	O_P_matrix = MaxPrunning(sampleSize);
    for(int i=0;i<(int)dataset.size();i++){
        PivotSelect(i, sampleSize, 1, LGroup);
    }
}

EPT::~EPT()
{
    delete[] ispivot,cand,samples_objs;
    for(int i=0;i<sampleSize;i++) delete[] O_P_matrix[i];
    delete[] O_P_matrix;
}

bool EPT::insert(Node p)
{
    if(p.dim!=dim1) return false;
    for(int i=0;i<dim1;i++)
        dataset1.push_back(p.data[i]);
    for(int i=0;i<LGroup;i++)
    {
        dist.push_back(0);
        pivId.push_back(0);
    }
    PivotSelect(num, sampleSize, 1, LGroup);
    num++;
    return true;
}

bool EPT::del(int id)
{
    if(id<0 or id>=num) return false;
    #ifdef lazy
    lazy_update.insert(id);
    if(lazy_update.size()*10>=num)
    {
        vector<float> tmp_dataset1((num-lazy_update.size())*dim1,0);
        vector<int> tmp_pivId((num-lazy_update.size())*LGroup,0);
        vector<double> tmp_dist((num-lazy_update.size())*LGroup,0);
        for(int i=0,p=0,k=0;i<num;i++)
        {
            if(lazy_update.count(i)) continue;
            for(int j=0;j<dim1;j++) tmp_dataset1[p++]=dataset1[i*dim1+j];
            for(int j=0;j<LGroup;j++)
            {
                tmp_pivId[k]=pivId[i*LGroup+j];
                tmp_dist[k++]=dist[i*LGroup+j];
            }
        }
        dataset1=tmp_dataset1;
        pivId=tmp_pivId;
        dist=tmp_dist;
        num-=lazy_update.size();
        lazy_update.clear();
    }
    #else
    if(dim1)
    {
        dataset1.erase(dataset1.begin()+id*dim1,dataset1.begin()+(id+1)*dim1);
        pivId.erase(pivId.begin()+id*LGroup,pivId.begin()+(id+1)*LGroup);
        dist.erase(dist.begin()+id*LGroup,dist.begin()+(id+1)*LGroup);
    }
    num--;
    #endif // lazy
    return true;
}

int EPT::PivotSelect(int objId, int o_num, int q_num, int pivotNum)
{

	double ** Q_O_matrix = new double *[q_num];
	double ** Q_P_matrix = new double*[q_num];
	double ** esti = new double*[q_num];
	for (int i = 0;i<q_num;i++)
	{
		Q_O_matrix[i] = new double[o_num];
		Q_P_matrix[i] = new double[num_cand];
		esti[i] = new double[o_num];
	}
	bool* indicator = new bool[num_cand];
	for (int i = 0;i<num_cand;i++)
		indicator[i] = true;


	for (int i = 0;i<q_num;i++)
	{
		for (int j = 0;j<o_num;j++)
		{
			Q_O_matrix[i][j] = sqrt(distance(dataset1.data()+objId*dim1, samples_objs+j*dim1,dim1));
			esti[i][j] = 0;
		}
		for (int j = 0;j<num_cand;j++)
		{
			Q_P_matrix[i][j] = sqrt(distance(dataset1.data()+objId*dim1, samples_objs+cand[j]*dim1,dim1));
		}
	}

	double d = 0;
	double t = 0;
	int choose;

	int i;
	for (i = 0;i<pivotNum; i++)
	{
		choose = -1;
		for (int j = 0;j<num_cand;j++)
		{
			if (indicator[j])
			{
				t = 0;
				for (int m = 0;m < q_num;m++)
				{
					for (int n = 0;n <o_num;n++)
					{
						if (Q_O_matrix[m][n] != 0)
						{

							t += (max(fabs(Q_P_matrix[m][j] - O_P_matrix[n][j]), esti[m][n])) / Q_O_matrix[m][n];
						}
					}
				}
				t = t / (q_num*o_num);
				if (t>d)
				{
					choose = j;
					d = t;
				}
			}
		}

		if (choose == -1)
			break;
		indicator[choose] = false;
		if (!ispivot[choose])
			ispivot[choose] = true;
		pivId[objId*LGroup+i]=choose;
		dist[objId*LGroup+i]=Q_P_matrix[0][choose];
		for (int m = 0;m<q_num;m++)
			for (int n = 0;n<o_num;n++)
				esti[m][n] = max(fabs(Q_P_matrix[m][choose] - O_P_matrix[n][choose]), esti[m][n]);

	}

	if (i < pivotNum)
	{
		for(int j =0;j<num_cand;j++)
			if (indicator[j])
			{
				indicator[j] = false;

				pivId[objId*LGroup+i]=j;
                dist[objId*LGroup+i]=Q_P_matrix[0][j];

				if (!ispivot[j])
					ispivot[j] = true;
				i++;
				if (i == pivotNum)
					break;
			}

	}

	for (i = 0;i<q_num;i++)
	{
		delete[] esti[i];
		delete[] Q_P_matrix[i];
		delete[] Q_O_matrix[i];
	}
	delete[] indicator;
	return pivotNum;
}

double** EPT::MaxPrunning(int num)
{
	cand = new int[num_cand];
	bool * indicator = new bool[num];
	for (int i = 0;i<num;i++)
		indicator[i] = true;
	int * idset = new int[num_cand];

	double d = 0.0;
	double t;
	int choose = 0;

	double** distmatrix = new double*[num];
	for (int i = 0;i<num;i++)
	{
		distmatrix[i] = new double[num_cand];
		for (int j = 0;j<num_cand;j++)
			distmatrix[i][j] = 0;
	}
	if (num_cand> 0)
	{

		for (int i = 1;i<num;i++)
		{
			t = sqrt(distance(samples_objs+i*dim1, samples_objs,dim1));
			if (t>d)
			{
				d = t;
				choose = i;
			}
		}

		idset[0] = choose;
		cand[0] = choose;
		indicator[choose] = false;

	}

	if (num_cand>1)
	{
		d = 0;
		for (int i = 0;i<num;i++)
		{
			if (indicator[i])
			{
				distmatrix[i][0] = sqrt(distance(samples_objs+cand[0]*dim1, samples_objs+i*dim1,dim1));
				if (distmatrix[i][0]>d)
				{
					d = distmatrix[i][0];
					choose = i;
				}
			}
		}

		idset[1] = choose;
		cand[1] = choose;
		indicator[choose] = false;

	}

	double edge = d;
	d = MAXREAL;
	for (int i = 2;i<num_cand;i++)
	{
		d = MAXREAL;
		for (int j = 0;j<num;j++)
		{
			if (indicator[j])
			{
				t = 0;
				for (int k = 0;k<i - 1;k++)
				{
					t += fabs(edge - distmatrix[j][k]);
				}
				distmatrix[j][i - 1] = sqrt(distance(samples_objs+j*dim1, samples_objs+cand[i - 1]*dim1,dim1));
				t += fabs(edge - distmatrix[j][i - 1]);
				if (t<d)
				{
					d = t;
					choose = j;
				}
			}
		}

		idset[i] = choose;

		indicator[choose] = false;
		cand[i] = choose;
	}

	for (int i = 0;i<num;i++)
	{
		if (indicator[i])
		{
			distmatrix[i][num_cand - 1] = sqrt(distance(samples_objs+i*dim1, samples_objs+cand[num_cand - 1]*dim1,dim1));
		}
	}

	for (int i = 0;i<num_cand;i++)
		for (int j = i + 1;j<num_cand;j++)
			distmatrix[idset[i]][j] = sqrt(distance(samples_objs+idset[i]*dim1, samples_objs+cand[j]*dim1,dim1));

	delete[] indicator;
	delete[] idset;

	return distmatrix;
}

void EPT::sample_data(int nums, vector<Node> &dataset)
{
	set<int> samples;
    default_random_engine e;
    uniform_int_distribution<int> rd(0,dataset.size()-1);
	int id;
	while ((int)samples.size() < nums)
	{
		id = rd(e);
		if (samples.find(id) == samples.end())
		{
			samples.insert(id);
		}
	}
	id=0;
	for (set<int>::iterator it = samples.begin(); it != samples.end(); ++it)
	{
	    for(int i=0;i<dim1;i++)
            samples_objs[id++]=dataset[*it].data[i];
	}
}

#endif // EPT_HPP
