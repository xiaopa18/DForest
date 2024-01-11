#pragma once
#include<vector>
#include<unordered_set>
using namespace std;
//#define lazy
class Loss{
    vector<unsigned char> a;
    #ifdef lazy
    unordered_set<int> lazy_update;
    #endif // lazy
    int num;
public:
    Loss():num(0){}
    bool insert(int t)
    {
        t--;
        if(t>>4) return false;
        if(!(num&1)) a.push_back('\0');
        if(num&1) a[num>>1]|=t;
        else a[num>>1]|=t<<4;
        num++;
        return true;
    }
    int query(int id)
    {
        if(id>=num or id<0) return 0x3f3f3f3f;
        return (int)(id&1?a[id>>1]&(0xf):a[id>>1]>>4)+1;
    }
    double get_size()
    {
        return a.size();
    }
    bool del(int id)
    {
        if(id>=num or id<0) return false;
        #ifdef lazy
        lazy_update.insert(id);
        if(lazy_update.size()*10>=num)
        {
            Loss new_loss;
            for(int i=0;i<num;i++)
            {
                if(lazy_update.count(i)) continue;
                new_loss.insert(query(i));
            }
            a=new_loss.a;
            num=new_loss.num;
            lazy_update.clear();
        }
        #else
        if(id&1) a[id>>1]&=0xf0;
        for(int i=id+1;i<num;i++)
        {
            if(i&1)  a[i>>1]<<=4;
            else a[(i-1)>>1]|=a[i>>1]>>4;
        }
        num--;
        if(!(num&1)) a.pop_back();
        #endif // lazy
        return true;
    }
};
