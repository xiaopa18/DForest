#ifndef EPT_STORAGE_HPP
#define EPT_STORAGE_HPP
#define fi first
#define se second
#include"../Node.hpp"
#include"../Loss/Loss.hpp"
#include<cmath>
#include<algorithm>
#include<queue>
#include<vector>
#include<random>
#include<set>
#include<fstream>
#include<ctime>
#define MAXREAL 1e20
using namespace std;
typedef pair<double,int> PDI;
extern double IOtime;

#ifdef CALC_PAGE
extern double page;
#endif // CALC_PAGE

#ifdef CALC_DIST_COUNT
extern double dcmp;
#endif

struct EPT_storage
{
    int num_cand;//the number of candidate
    int LGroup;//pivot number
    int num,len;
    int dim1;
    ifstream d1inf,pivinf,disinf;
    string index;
    float* dataset1;//index
    int* pivId;
    double* dist;
    bool*ispivot;
    double** O_P_matrix;
    int* cand;
    int sampleSize;
    float* samples_objs;
    EPT_storage(vector<Node> dataset,string index,int page_size,int LGroup,int num_cand);
    void sample_data(int nums, vector<Node> &dataset);
    double** MaxPrunning(int num);
    int PivotSelect(int objId, int o_num, int q_num, int pivotNum,ofstream &pivouf,ofstream &disouf);
    double distance(float *a,float *b,int len);
    bool insert(Node p);
    bool del(int idx);
    vector<PDI>* rangequery(float *que,double r);
};

vector<PDI>* EPT_storage::rangequery(float *que,double r)
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
	d1inf.seekg(0,ios::beg);
	pivinf.seekg(0,ios::beg);
	disinf.seekg(0,ios::beg);
	bool next = false;
	for(int bk=0;bk<(num+len-1)/len;bk++)
    {
        int leng=min(len,num-bk*len);
        IOtime-=clock();
        d1inf.read((char*)dataset1,leng*dim1*4);
        pivinf.read((char*)pivId,leng*4*LGroup);
        disinf.read((char*)dist,leng*8*LGroup);
        #ifdef CALC_PAGE
        page++;
        #endif // CALC_PAGE
        IOtime+=clock();
        int pos=-LGroup;
        for(int i=0;i<leng;i++)
        {
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
                dis=distance(que,dataset1+i*dim1,dim1);
                if(dis<=r2) ans->push_back({dis,bk*len+i});
            }
        }
    }
    return ans;
}

double EPT_storage::distance(float *a,float *b,int len)
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

EPT_storage::EPT_storage(vector<Node> dataset,string index,int page_size=4096,int LGroup=10,int num_cand=40)//unit:B
{
    dim1=dataset[0].dim;
    this->num=dataset.size();
    this->LGroup=LGroup;
    this->num_cand=num_cand;
    this->len=max(page_size/(dataset[0].data.size()*4+8+LGroup*(8+4)),(size_t)1);
    pivId=new int[len*LGroup];
    dist=new double[len*LGroup];
    dataset1=new float[len*dim1];
    ofstream ouf1(index+".d1",ios::out|ios::binary);
    if(!ouf1.is_open())
    {
        cout<<"open "<<index+".d1"<<" write is failed\n";
        exit(-1);
    }
    for(int i=0;i<dataset.size();i++)
    {
        for(int j=0;j<dim1;j++)
            ouf1.write((char*)(&dataset[i].data[j]),4);
    }
    ouf1.close();
    d1inf.open(index+".d1",ios::in|ios::binary);
    if(!d1inf.is_open())
    {
        cout<<"open "<<index+".d1"<<" read is failed\n";
        exit(-1);
    }
    sampleSize = max((int)dataset.size() / 200,min(num_cand,(int)dataset.size()));
    samples_objs=new float[sampleSize*dim1];
	sample_data(sampleSize, dataset);
    ispivot = new bool[num_cand];
	for (int i = 0; i < num_cand; i++)
		ispivot[i] = false;
	O_P_matrix = MaxPrunning(sampleSize);
	ofstream pivouf(index+".piv",ios::out|ios::binary);
	ofstream disouf(index+".dis",ios::out|ios::binary);
    if(!pivouf.is_open())
    {
        cout<<"open "<<index+".piv"<<" write is failed\n";
        exit(-1);
    }
    if(!disouf.is_open())
    {
        cout<<"open "<<index+".dis"<<" write is failed\n";
        exit(-1);
    }
    for(int bk=0;bk<(num+len-1)/len;bk++)
    {
        int leng=min(len,num-bk*len);
        d1inf.read((char*)dataset1,leng*dim1*4);
        for(int i=0;i<leng;i++){
            PivotSelect(i, sampleSize, 1, LGroup,pivouf,disouf);
        }
    }
    pivouf.close();
    disouf.close();
    pivinf.open(index+".piv",ios::in|ios::binary);
    disinf.open(index+".dis",ios::in|ios::binary);
    if(!pivinf.is_open())
    {
        cout<<"open "<<index+".piv"<<" read is failed\n";
        exit(-1);
    }
    if(!disinf.is_open())
    {
        cout<<"open "<<index+".dis"<<" read is failed\n";
        exit(-1);
    }
}

bool EPT_storage::insert(Node p)
{
    if(p.dim!=dim1) return false;
    pivinf.close();
    disinf.close();
    d1inf.close();
    ofstream ouf1(index+".d1",ios::app|ios::binary);
    if(!ouf1.is_open())
    {
        cout<<"open "<<index+".d1"<<" write is failed\n";
        exit(-1);
    }
    for(int i=0;i<dim1;i++)
    {
        dataset1[i]=p.data[i];
        ouf1.write((char*)&p.data[i],4);
    }
    ouf1.close();
    d1inf.open(index+".d1",ios::in|ios::binary);
    if(!d1inf.is_open())
    {
        cout<<"open "<<index+".d1"<<" read is failed\n";
        exit(-1);
    }
    ofstream pivouf(index+".piv",ios::app|ios::binary);
	ofstream disouf(index+".dis",ios::app|ios::binary);
    if(!pivouf.is_open())
    {
        cout<<"open "<<index+".piv"<<" write is failed\n";
        exit(-1);
    }
    if(!disouf.is_open())
    {
        cout<<"open "<<index+".dis"<<" write is failed\n";
        exit(-1);
    }
    PivotSelect(0, sampleSize, 1, LGroup,pivouf,disouf);
    pivouf.close();
    disouf.close();
    pivinf.open(index+".piv",ios::in|ios::binary);
    disinf.open(index+".dis",ios::in|ios::binary);
    if(!pivinf.is_open())
    {
        cout<<"open "<<index+".piv"<<" read is failed\n";
        exit(-1);
    }
    if(!disinf.is_open())
    {
        cout<<"open "<<index+".dis"<<" read is failed\n";
        exit(-1);
    }
    num++;
    return true;
}

bool EPT_storage::del(int id)
{
    d1inf.seekg(0,ios::beg);
    pivinf.seekg(0,ios::beg);
    disinf.seekg(0,ios::beg);
    float *dataset1=new float[num*dim1];
    int *pivId=new int[num*LGroup];
    double *dist=new double[num*LGroup];
    d1inf.read((char*)dataset1,4*num*dim1);
    pivinf.read((char*)pivId,4*num*LGroup);
    disinf.read((char*)dist,8*num*LGroup);
    d1inf.close();
    pivinf.close();
    disinf.close();
    ofstream ouf1(index+".d1",ios::out|ios::binary);
    if(!ouf1.is_open())
    {
        cout<<"open "<<index+".d1"<<" write is failed\n";
        exit(-1);
    }
    ofstream pivouf(index+".piv",ios::out|ios::binary);
	ofstream disouf(index+".dis",ios::out|ios::binary);
    if(!pivouf.is_open())
    {
        cout<<"open "<<index+".piv"<<" write is failed\n";
        exit(-1);
    }
    if(!disouf.is_open())
    {
        cout<<"open "<<index+".dis"<<" write is failed\n";
        exit(-1);
    }

    for(int i=0;i<num;i++)
    {
        if(i==id) continue;
        for(int j=0;j<dim1;j++) ouf1.write((char*)&dataset1[i*dim1+j],4);
        for(int j=0;j<LGroup;j++)
        {
            pivouf.write((char*)&pivId[i*LGroup+j],4);
            disouf.write((char*)&dist[i*LGroup+j],8);
        }
    }
    ouf1.close();
    d1inf.open(index+".d1",ios::in|ios::binary);
    if(!d1inf.is_open())
    {
        cout<<"open "<<index+".d1"<<" read is failed\n";
        exit(-1);
    }
    pivouf.close();
    disouf.close();
    pivinf.open(index+".piv",ios::in|ios::binary);
    disinf.open(index+".dis",ios::in|ios::binary);
    if(!pivinf.is_open())
    {
        cout<<"open "<<index+".piv"<<" read is failed\n";
        exit(-1);
    }
    if(!disinf.is_open())
    {
        cout<<"open "<<index+".dis"<<" read is failed\n";
        exit(-1);
    }
    num--;
    delete[] dataset1,pivId,dist;
    return true;
}

int EPT_storage::PivotSelect(int objId, int o_num, int q_num, int pivotNum,ofstream &pivouf,ofstream &disouf)
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
			Q_O_matrix[i][j] = sqrt(distance(dataset1+objId*dim1, samples_objs+j*dim1,dim1));
			esti[i][j] = 0;
		}
		for (int j = 0;j<num_cand;j++)
		{
			Q_P_matrix[i][j] = sqrt(distance(dataset1+objId*dim1, samples_objs+cand[j]*dim1,dim1));
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
        pivouf.write((char*)&choose,4);
        disouf.write((char*)&Q_P_matrix[0][choose],8);
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

                pivouf.write((char*)&j,4);
                disouf.write((char*)&Q_P_matrix[0][j],8);

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

double** EPT_storage::MaxPrunning(int num)
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

void EPT_storage::sample_data(int nums, vector<Node> &dataset)
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

#endif // EPT_STORAGE_HPP
