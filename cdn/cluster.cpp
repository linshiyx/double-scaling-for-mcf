//
// Created by antelope on 3/19/17.
//
#include "deploy.h"
#include <queue>
#include <algorithm>
#include <cstring>
#include <iostream>

using namespace std;

struct ConsumerDistance
{
    int id;
    int distance;

    bool operator<(const ConsumerDistance &c)
    {
        return distance < c.distance;
    }
};

ConsumerDistance consumerDistance_[MAX_NODE_NUM][MAX_CONSUMER_NUM];
int dist_[MAX_NODE_NUM];
bool vis_[MAX_NODE_NUM];
const int CONSUMER_INT_NUM = 16;
int usedConsumerIntNum_;
unsigned int closestConsumer_[MAX_NODE_NUM][CONSUMER_INT_NUM];
int clusterId_[MAX_NODE_NUM];
int clusterConsumerCount_[MAX_CONSUMER_NUM][MAX_CONSUMER_NUM];
int clusterCenter_[MAX_CONSUMER_NUM];
queue<int> que_;

extern int nodeNum_;
extern int consumerNum_;
extern int arcList_[MAX_NODE_NUM][MAX_ARC_PER_NODE + 1];
extern int cost_[MAX_NODE_NUM][MAX_NODE_NUM];
extern int consumerNode_[MAX_NODE_NUM];


void spfa(int s)
{
    for (int i=0; i<nodeNum_; i++) dist_[i] = INF;
    que_.push(s);
    vis_[s] = true;
    dist_[s] = 0;
    int node;
    int temp;
    while (!que_.empty())
    {
        int cur = que_.front();
        que_.pop();
        vis_[cur] = false;
        for (int i=1; i<=arcList_[cur][0]; i++)
        {
            node = arcList_[cur][i];
            temp = dist_[cur] + cost_[cur][node];
            if (dist_[node] > temp)
            {
                dist_[node] = temp;
                if (!vis_[node])
                {
                    vis_[node] = true;
                    que_.push(node);
                }
            }
        }
    }

}

void setConsumerDistance(int s)
{
    int temp;
    for (int i=0; i<nodeNum_; i++)
    {
        temp = consumerNode_[i];
        if (temp == -1) continue;
        consumerDistance_[s][temp].id = i;
        consumerDistance_[s][temp].distance = dist_[i];
    }
}

void findClosestConsumer()
{
    for (int i=0; i<nodeNum_; i++)
    {
        spfa(i);
        setConsumerDistance(i);
        sort(consumerDistance_[i], consumerDistance_[i] + consumerNum_);
    }

}


int randomN(int n)
{
    return rand() % n;
}

int getConsumerListDist(unsigned a[], unsigned b[])
{
    int distance = 0, temp = 0;
    for(int i=0; i<usedConsumerIntNum_; i++)
    {
        temp = a[i] ^ b[i];
        temp=(temp&0x55555555)+((temp>>1)&0x55555555);
        temp=(temp&0x33333333)+((temp>>2)&0x33333333);
        temp=(temp&0x0F0F0F0F)+((temp>>4)&0x0F0F0F0F);
        temp=(temp&0x00FF00FF)+((temp>>8)&0x00FF00FF);
        temp=(temp&0x0000FFFF)+((temp>>16)&0x0000FFFF);
        distance += temp;
    }
    return distance;
}

int findKth(int count, int a[])
{
    int b[nodeNum_] = {0};
    for(int i=0; i<consumerNum_; i++) {
        b[a[i]]++;
    }
    for(int i=nodeNum_-1; i>=0; i--)
    {
        count -= b[i];
        if(count <= 0) {
            return i;
        }
    }
    return 0;
}

void arrayToByte(int n, int a[], unsigned int b[])
{
    for(int i=0; i<n; i++) {
        b[a[i] / 32] |= 1 << (a[i] % 32);
    }

}

void kmedoids(int n, unsigned int a[])
{
    int comparedConsumerLength = consumerNum_ / n;
    bool changed = true;
    int minCenter = 0, temp, count = 0, minDist = INF;
    bool flag[1000];
    for(int i=0; i<nodeNum_; i++) flag[i] = false;
    while(count < n)
    {
        temp = randomN(nodeNum_);
        if(flag[temp]) continue;
        flag[temp] = true;
        clusterCenter_[count] = temp;
        count++;
    }
    while (changed)
    {
        changed = false;
        //初始化
        memset(clusterConsumerCount_, 0, sizeof(clusterConsumerCount_));
        //找到最近的类中心
        for(int i=0; i<nodeNum_; i++) {
            minDist = INF;
            for(int j=0; j<n; j++)
            {
                temp = getConsumerListDist(closestConsumer_[i], closestConsumer_[clusterCenter_[j]]);
                if(temp < minDist)
                {
                    minDist = temp;
                    minCenter = j;
                }
            }

            //更新clusterConsumerCount
            unsigned temp1 = 1;
            int temp2 = -1;
            for(int j=0; j<consumerNum_; j++)
            {
                if(j % 32 == 0)
                {
                    temp2++;
                    temp1 = 1;
                }
                if((temp1 & closestConsumer_[i][temp2]) > 0) clusterConsumerCount_[minCenter][j]++;
                temp1 = temp1 << 1;
            }
            if(clusterId_[i] != minCenter)
            {
                changed = true;
                clusterId_[i] = minCenter;
            }
        }
        int kth;
        unsigned temp1[CONSUMER_INT_NUM] = {0};
        for(int i=0; i<n; i++)
        {
            kth = findKth(comparedConsumerLength ,clusterConsumerCount_[i]);
            for(int j=0; j<consumerNum_; j++) {
                if(kth > clusterConsumerCount_[i][j]) continue;
                temp1[j / 32] |= 1 << (j % 32);
            }

            minDist = INF;
            for(int j=0; j<nodeNum_; j++)
            {
                temp = getConsumerListDist(closestConsumer_[j], temp1);
                if(temp < minDist)
                {
                    int flag = false;
                    for(int k=0; k<i; k++) {
                        if(clusterCenter_[k] == j) flag = true;
                    }
                    if(flag) continue;
                    minDist = temp;
                    minCenter = j;
                }
            }
            clusterCenter_[i] = minCenter;
        }
    }

    arrayToByte(n, clusterCenter_, a);
}



void printByte(unsigned int b[])
{
    unsigned temp = 1;
    string res;
    for(int j=0; j<nodeNum_; j++)
    {
        if(j % 32 == 0) temp = 1;
        if((temp & b[j / 32]) > 0)
        {
            res += '1';
        } else {
            res += '0';
        }
        temp = temp << 1;
    }
    res += '\n';
    cout<<res;
}


void setClosestConsumer(int n)
{
    int comparedConsumerLength = consumerNum_ / n;
    usedConsumerIntNum_ = comparedConsumerLength / 32 + 1;
    int id;
    for(int i=0; i<nodeNum_; i++)
    {
        for(int j=0; j<usedConsumerIntNum_; j++) closestConsumer_[i][j] = 0;
        for(int j=0; j<comparedConsumerLength; j++)
        {
            id = consumerDistance_[i][j].id;
            closestConsumer_[i][id / 32] |= 1 << (id % 32);
        }
    }
    for(int i=0; i<nodeNum_; i++)
    {
//        printByte(closestConsumer_[i]);
    }

}

