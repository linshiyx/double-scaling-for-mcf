//
// Created by antelope on 3/31/17.
//

#include <cstring>
#include "deploy.h"
#include <queue>
#include <iostream>

using namespace std;

// data struct for isap
struct Edge
{
    int from, to;
    int next, cap;
};
extern Edge edge[MAX_ARC_NUM + 3000];
extern int head[MAX_NODE_NUM];
extern int cnt;
extern int src;
extern int des;
extern int n;
extern int originalCap[MAX_ARC_PER_NODE + 3000];
int tmp,  m;
int gap[MAX_NODE_NUM],dep[MAX_NODE_NUM],cur[MAX_NODE_NUM], stack[MAX_NODE_NUM], top;

extern int sink_;
// equals to nodeNum_++ because of one more sink_
int nodeNumWithSink_;
// node of a augment path
int p_[MAX_NODE_NUM];
// number of node whose distance to t equals to i
int num_[MAX_NODE_NUM];
// index of current arc
int cur_[MAX_NODE_NUM];
// distance between i and sink
int d_[MAX_NODE_NUM];
bool visited_[MAX_NODE_NUM];


extern int source_;
extern int nodeNum_;
extern int consumerNum_;
extern int arcList_[MAX_NODE_NUM][MAX_ARC_PER_NODE + 1];
extern int capacity_[MAX_NODE_NUM][MAX_NODE_NUM];
extern int nodeConsumer_[MAX_CONSUMER_NUM];
extern int consumerDemand_[MAX_NODE_NUM];

void addEdge(int u, int v, int f);

void initCap()
{
    for(int i=0; i<cnt; i+=2)
    {
        edge[i].cap = originalCap[i];
    }
    for(int i=1; i<cnt; i+=2)
    {
        edge[i].cap = 0;
    }
}

void addSource(int serverLocation[])
{
    int listLength, node;
    listLength = serverLocation[0];
    for(int i=1; i<=listLength; i++)
    {
        node = serverLocation[i];
        addEdge(src, node, INF);
    }
}

void removeSource(int serverLocation[])
{
    int listLength, node;
    listLength = serverLocation[0];
    for(int i=1; i<=listLength; i++)
    {
        node = serverLocation[i];
        head[node] = edge[head[node]].next;
        cnt--;
        cnt--;
    }
    head[src] = -1;
}


int ISAP(int serverLocation[])
{
    initCap();
    addSource(serverLocation);
    int cur_fLow,max_fLow,u,insert,i;
    memset(dep,0,sizeof(dep));
    memset(gap,0,sizeof(gap));
    memcpy(cur, head, n * sizeof(int));
    max_fLow = 0;
    u = src;
    top = 0;
    while(dep[src] < n)
    {
        if(u == des)
        {
            cur_fLow = INF;
            for(i = 0; i < top; ++i)
            {
                if(cur_fLow > edge[stack[i]].cap)
                {
                    insert = i;
                    cur_fLow = edge[stack[i]].cap;
                }
            }
            for(i = 0; i < top; ++i)
            {
                edge[stack[i]].cap -= cur_fLow;
                edge[stack[i]^1].cap += cur_fLow;
            }
            max_fLow += cur_fLow;
            u = edge[stack[top = insert]].from;
        }
        for(i = cur[u]; i != -1; i = edge[i].next)
            if((edge[i].cap > 0) && (dep[u] == dep[edge[i].to]+1))
                break;
        if(i != -1)
        {
            stack[top++] = i;
            u = edge[i].to;
        }
        else
        {
            if(0 == --gap[dep[u]]) break;
            int minn = n;
            for(i = head[u]; i != -1; i = edge[i].next)
            {
                if(edge[i].cap > 0)
                    minn = (minn > dep[edge[i].to]) ? (cur[u]= i, dep[edge[i].to]) : minn;
            }
            ++gap[dep[u] = minn + 1];
            if(u != src) u = edge[stack[--top]].from;
        }
    }
    removeSource(serverLocation);
    return max_fLow;
}