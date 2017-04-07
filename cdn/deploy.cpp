#include "deploy.h"
#include <stdio.h>
#include <string>
#include <iostream>
#include <stdint.h>
#include <cstring>
#include <math.h>

using namespace std;

struct Arc
{
    int node;
    int x;
    int c;
};

// data struct for isap
struct Edge
{
    int from, to;
    int next, cap;
} edge[MAX_ARC_NUM + 3000];
int head[MAX_NODE_NUM];
int cnt, src, des, n;
int originalCap[MAX_ARC_PER_NODE + 3000];

//original network node number
int nodeNum_ = 0;
//uncapacited network node number
int uncapacitedNodeNum_ = 0;
int arcNum_ = 0;
int consumerNum_ = 0;
int serverCost_ = 0;
int totalDemand_ = 0;
int sink_ = 0;
int source_ = 0;
//bigest c
int maxC_ = 0;
//bigest u
int maxU_ = 0;
int capacity_[MAX_NODE_NUM][MAX_NODE_NUM];
int cost_[MAX_NODE_NUM][MAX_NODE_NUM];
//consumerNode corresponds to certain node
int consumerNode_[MAX_NODE_NUM];
int consumerDemand_[MAX_NODE_NUM];
//node corresponds to certain consumerNode
int nodeConsumer_[MAX_CONSUMER_NUM];
//linked list
int arcList_[MAX_NODE_NUM][MAX_ARC_PER_NODE + 1];
//making arcs uncapacited;
int uncapacitedCost_[MAX_UNCAPACITED_NODE_NUM][MAX_ARC_PER_NODE + 1];
int uncapacitedArcList_[MAX_UNCAPACITED_NODE_NUM][MAX_ARC_PER_NODE + 1] = {0};
int uncapacitedExcess_[MAX_UNCAPACITED_NODE_NUM] = {0};
Arc answerArcList_[MAX_UNCAPACITED_NODE_NUM][MAX_ARC_PER_NODE + 1];
int answerServerLocation[MAX_NODE_NUM];
int answer_ = INF;
int answerLine_ = 0;
string res_;

void findClosestConsumer();
void setClosestConsumer(int n);
void kmedoids(int n, unsigned int a[]);
int doubleScaling(int serverLocation[]);
int ISAP(int serverLocation[]);


void addEdge(int u, int v, int f)
{
    edge[cnt].next = head[u];
    edge[cnt].from = u;
    edge[cnt].to = v;
    edge[cnt].cap = f;
    head[u] = cnt++;
    edge[cnt].next = head[v];
    edge[cnt].from = v;
    edge[cnt].to = u;
    edge[cnt].cap = 0;
    head[v] = cnt++;
}


double calcutaleServerNum(double totalDemand, double costAve, double arcAve, double serverPrice)
{
    return (totalDemand * costAve / serverPrice / (log(arcAve) ));
}


void init_input(char * topo[MAX_EDGE_NUM])
{
    char *c;
    int spaceCount = 0;

    c = topo[0];
    while (*c != '\0' && *c != '\n' && *c != '\r')
    {
        if (*c == ' ')
        {
            c++;
            spaceCount++;
            continue;
        }
        if (spaceCount == 0)
        {
            nodeNum_ = *c - '0' + nodeNum_ * 10;
        } else if (spaceCount == 1)
        {
            arcNum_ = *c - '0' + arcNum_ * 10;
        } else if (spaceCount == 2)
        {
            consumerNum_ = *c - '0' + consumerNum_ * 10;
        }
        c++;
    };
    source_ = nodeNum_++;
    uncapacitedNodeNum_ = nodeNum_;
    //des, src, n for isap
    src = source_;
    des = source_ + 1;
    n = des + 1;
    memset(head, -1, sizeof(head));

    c = topo[2];
    while (*c != '\0' && *c != '\n' && *c != '\r')
    {
        serverCost_ = *c - '0' + serverCost_ * 10;
        c++;
    };

    //init cost_
    for(int i=0; i<nodeNum_; i++)
    {
        for(int j=0; j<nodeNum_; j++)
        {
            cost_[i][j] = INF;
        }
    }
    int node1, node2, capacity, cost;
    int totalCapacity = 0, totalCost = 0;
    int temp, temp2;
    for(int i=0; i< arcNum_; i++)
    {
        c = topo[i + 4];
        spaceCount = 0;
        node1 = node2 = capacity = cost = 0;
        while (*c != '\0' && *c != '\n' && *c != '\r')
        {
            if (*c == ' ')
            {
                c++;
                spaceCount++;
                continue;
            }
            if (spaceCount == 0)
            {
                node1 = *c - '0' + node1 * 10;
            } else if (spaceCount == 1)
            {
                node2 = *c - '0' + node2 * 10;
            } else if (spaceCount == 2)
            {
                capacity = *c - '0' + capacity* 10;
            } else if (spaceCount == 3)
            {
                cost = *c - '0' + cost * 10;
            }
            c++;
        }
        //original network node matrix
        capacity_[node1][node2] = capacity_[node2][node1] = capacity;
        cost_[node1][node2] = cost_[node2][node1] = cost;
        //original network node list
        temp = ++arcList_[node1][0];
        arcList_[node1][temp] = node2;
        temp = ++ arcList_[node2][0];
        arcList_[node2][temp] = node1;

        //doubleScaling requires directed network, so we transform the undirected network into directed network
        //uncapacited network node matrix and node list
        temp = uncapacitedNodeNum_++;
        uncapacitedExcess_[temp] = -capacity;
        uncapacitedExcess_[node2] += capacity;
        temp2 = ++uncapacitedArcList_[node1][0];
        uncapacitedArcList_[node1][temp2] = temp;
        uncapacitedCost_[node1][temp2] = cost;
        temp2 = ++uncapacitedArcList_[node2][0];
        uncapacitedArcList_[node2][temp2] = temp;
        uncapacitedCost_[node2][temp2] = 0;

        temp = uncapacitedNodeNum_++;
        uncapacitedExcess_[temp] = -capacity;
        uncapacitedExcess_[node1] += capacity;
        temp2 = ++uncapacitedArcList_[node1][0];
        uncapacitedArcList_[node1][temp2] = temp;
        uncapacitedCost_[node1][temp2] = 0;
        temp2 = ++uncapacitedArcList_[node2][0];
        uncapacitedArcList_[node2][temp2] = temp;
        uncapacitedCost_[node2][temp2] = cost;

        //graph for isap
        addEdge(node1, node2, capacity);
        originalCap[cnt - 2] = capacity;
        addEdge(node2, node1, capacity);
        originalCap[cnt - 2] = capacity;

        maxC_ = maxC_ > capacity ? maxC_ : capacity;
        maxU_ = maxU_ > cost ? maxU_ : cost;
        totalCapacity += capacity;
        totalCost += cost * capacity;
    }


    int consumerNode, consumerDemand;
    int totalConsumerDemand = 0;
    for (int i=0; i<nodeNum_; i++) consumerNode_[i] = -1;
    for (int i=0; i<consumerNum_; i++) nodeConsumer_[i] = -1;
    for (int i=0; i<consumerNum_; i++)
    {
        c = topo[i + 5 + arcNum_];
        spaceCount = 0;
        node1 = consumerNode = consumerDemand = 0;
        while (*c != '\0' && *c != '\n' && *c != '\r')
        {
            if (*c == ' ')
            {
                c++;
                spaceCount++;
                continue;
            }
            if (spaceCount == 0)
            {
                consumerNode = *c - '0' + consumerNode * 10;
            } else if (spaceCount == 1)
            {
                node1 = *c - '0' + node1 * 10;
            } else if (spaceCount == 2)
            {
                consumerDemand = *c - '0' + consumerDemand * 10;
            }
            c++;
        }
        consumerNode_[node1] = consumerNode;
        consumerDemand_[node1] = consumerDemand;
        nodeConsumer_[consumerNode] = node1;
        totalConsumerDemand += consumerDemand;
        uncapacitedExcess_[node1] -= consumerDemand;

        //add consumer to sink for isap
        addEdge(node1, des, consumerDemand);
        originalCap[cnt - 2] = consumerDemand;
    }
    totalDemand_ = totalConsumerDemand;
    uncapacitedExcess_[source_] = totalConsumerDemand;
}

void formalizeOutput()
{
    int path[MAX_NODE_NUM], pathUl[MAX_NODE_NUM], u, l, ul, flow, demand = 0;
    for(int i=1; i<=answerServerLocation[0]; i++)
    {
        int server = answerServerLocation[i];
        if(consumerNode_[server] < 0)
        {
            demand = 0;
        } else {
            demand = consumerDemand_[server];
        }
        for(int j=1; j<=answerArcList_[server][0].node; j++)
        {
            demand += answerArcList_[server][j].x;
        }
        l=1;
        path[0] = server;
        while(demand > 0)
        {
            flow = demand;
            l = 1;
            while(consumerNode_[path[l - 1]] < 0)
            {
                u = path[l-1];
                ul = 1;
                while(answerArcList_[u][ul].x == 0) ul++;
                pathUl[l-1] = ul;
                path[l++] = answerArcList_[u][ul].node;
                flow = flow > answerArcList_[u][ul].x ? answerArcList_[u][ul].x : flow;
            }
            flow = flow > consumerDemand_[path[l-1]] ? consumerDemand_[path[l-1]] : flow;
            for(int i=0; i<l; i++)
            {
                res_ += to_string(path[i]);
                res_ += ' ';
            }
            for(int i=0; i<l-1; i++)
            {
                answerArcList_[path[i]][pathUl[i]].x -= flow;
            }
            demand -= flow;
            res_ += to_string(consumerNode_[path[l-1]]);
            res_ += ' ';
            res_ += to_string(flow);
            res_ += '\n';
            consumerDemand_[path[l-1]] -= flow;
            if(consumerDemand_[path[l-1]] == 0) consumerNode_[path[l-1]] = -1;
            answerLine_++;
        }

    }
}

void setServerLocation(int serverLocation[])
{
    uncapacitedArcList_[source_][0] = 0;
    for(int k=1; k<=serverLocation[0]; k++)
    {
        uncapacitedArcList_[source_][++uncapacitedArcList_[source_][0]] = serverLocation[k];
    }
}

//You need to complete the function
void deploy_server(char * topo[MAX_EDGE_NUM], int line_num,char * filename)
{
    clock_t clockStart = clock();
//    int n = 7;
    init_input(topo);

    //the clustering is deprecated because of poor performance
    //findClosestConsumer() and setClosestConsumer is pretreatment for clustering
    //sort closest consumers
//    findClosestConsumer();

    //set node feature based on consumerNum/n closest consumers
//    setClosestConsumer(n);

    //result of cluster
//    unsigned int b[32] = {0};
//    kmedoids(n, b);

    int serverLocation[nodeNum_] = {0};

    //serverLocation[0] is length of server
//    serverLocation[0] = 7;
//    serverLocation[1] = 7;
//    serverLocation[2] = 22;
//    serverLocation[3] = 43;
//    serverLocation[4] = 15;
//    serverLocation[5] = 37;
//    serverLocation[6] = 13;
//    serverLocation[7] = 38;
    //set source for doubleScaling
    //need to confirm the serverLocation is able to get maxflow
    if(ISAP(serverLocation) == totalDemand_)
    {

        //according to the original problem, we have to choose the source by ourselves
        //after we choose serverLocation, we have to refine the uncapacitedArcList
        setServerLocation(serverLocation);

        doubleScaling(serverLocation);
    }

    formalizeOutput();
    string res1 = to_string(answerLine_);
    res1 += '\n';
    res1 += '\n';
    res1 += res_;
    char* topo_file = (char *)res1.c_str();
    write_result(topo_file, filename);

    clock_t end = clock();
    cout<<(double)(end - clockStart)/CLOCKS_PER_SEC<<endl;

}
