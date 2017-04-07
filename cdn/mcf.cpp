//
// Created by antelope on 3/29/17.
//
#include <iostream>
#include <cmath>
#include <cstring>
#include "deploy.h"
#include <set>
#include <stdint.h>

using namespace std;

double e_ = 0;
int k_ = 0;
int l_ = 0;
int residualArcList_[MAX_UNCAPACITED_NODE_NUM][MAX_ARC_PER_NODE + 1];
int reverseIndex_[MAX_UNCAPACITED_NODE_NUM][MAX_ARC_PER_NODE + 1];
//capacity in residual network
int u_[MAX_UNCAPACITED_NODE_NUM][MAX_ARC_PER_NODE + 1];
//cost in residual network
int c_[MAX_UNCAPACITED_NODE_NUM][MAX_ARC_PER_NODE + 1];
//flow in residual network
int x_[MAX_UNCAPACITED_NODE_NUM][MAX_ARC_PER_NODE + 1];
//record path in advance()
int pred_[MAX_UNCAPACITED_NODE_NUM];
//record the index of path's nodes in pred's arcList
int predListIndex_[MAX_UNCAPACITED_NODE_NUM];
double pi_[MAX_UNCAPACITED_NODE_NUM];
int excess_[MAX_UNCAPACITED_NODE_NUM];
set<int> sDelta_;

struct Arc
{
    int node;
    int x;
    int c;
};
extern int maxC_;
extern int maxU_;
extern int source_;
extern int nodeNum_;
extern int arcList_[MAX_NODE_NUM][MAX_ARC_PER_NODE + 1];
extern int uncapacitedNodeNum_;
extern int uncapacitedCost_[MAX_UNCAPACITED_NODE_NUM][MAX_ARC_PER_NODE + 1];
extern int uncapacitedArcList_[MAX_UNCAPACITED_NODE_NUM][MAX_ARC_PER_NODE + 1];
extern int uncapacitedExcess_[MAX_UNCAPACITED_NODE_NUM];
extern Arc answerArcList_[MAX_UNCAPACITED_NODE_NUM][MAX_ARC_PER_NODE + 1];
extern int answerServerLocation[MAX_NODE_NUM];
extern int answer_;
extern int serverCost_;

void initResidualNetwork()
{
    int temp = 0;
    for(int i=0; i<uncapacitedNodeNum_; i++)
    {
        temp = uncapacitedArcList_[i][0];
        residualArcList_[i][0] = temp;
        for(int j=1; j<=temp; j++)
        {
            residualArcList_[i][j] = uncapacitedArcList_[i][j];
            c_[i][j] = uncapacitedCost_[i][j];
        }
    }
    for(int i=0; i<uncapacitedNodeNum_; i++)
    {
        excess_[i] = uncapacitedExcess_[i];
    }
}


bool advance(int k)
{
    if(excess_[k] < 0)
    {
        l_ = k;
        return true;
    };
    int nodeTemp = 0, listLength;
    listLength = residualArcList_[k][0];
    for(int i=1; i<=listLength; i++)
    {
        nodeTemp = residualArcList_[k][i];
        if(c_[k][i] - pi_[k] + pi_[nodeTemp] < 0)
        {
            if(i > uncapacitedArcList_[k][0] && u_[k][i] == 0) continue;
            pred_[nodeTemp] = k;
            predListIndex_[nodeTemp] = i;
            if(advance(nodeTemp))
            {
                return true;
            }
        }
    }
    pi_[k] += e_ / 2;
    return false;
}

void computeNodeImbalance(int delta)
{
    sDelta_.clear();
    for(int i=0; i<uncapacitedNodeNum_; i++)
    {
        if(excess_[i] >= delta) sDelta_.insert(i);
    }
}

void improveApproximation()
{
    memset(x_, 0, SIZE_OF_X);
    memset(reverseIndex_, 0, sizeof(reverseIndex_));
    initResidualNetwork();
    int delta = (int) pow(2, (int)(log(maxC_) / log(2)));
    for(int i=nodeNum_; i<uncapacitedNodeNum_; i++) pi_[i] += e_;
    set<int>::iterator it;
    int temp1, temp2, temp3, predIndex;
    while(delta > 0)
    {
        computeNodeImbalance(delta);
        while(!sDelta_.empty())
        {
                it = sDelta_.begin();
                k_ = *it;
                if(!advance(k_)) continue;
                if(l_ == k_) continue;
                temp1 = pred_[l_];
                temp2 = l_;
                predIndex = predListIndex_[l_];
                while(temp2 != k_)
                {
                    if(predIndex <= uncapacitedArcList_[temp1][0])
                    {

                        if(reverseIndex_[temp1][predIndex] == 0) {
                            temp3 = ++residualArcList_[temp2][0];
                            residualArcList_[temp2][temp3] = temp1;
                            c_[temp2][temp3] = -c_[temp1][predIndex];
                            u_[temp2][temp3] = 0;
                            reverseIndex_[temp1][predIndex] = temp3;
                            reverseIndex_[temp2][temp3] = predIndex;
                        }
                        x_[temp1][predIndex] += delta;
                        u_[temp2][reverseIndex_[temp1][predIndex]] += delta;
                    } else
                    {
                        u_[temp1][predIndex] -= delta;
                        x_[temp2][reverseIndex_[temp1][predIndex]] -= delta;
                    }

                    temp2 = temp1;
                    temp1 = pred_[temp2];
                    predIndex = predListIndex_[temp2];
                }
                temp1 = excess_[k_] -= delta;
                excess_[l_] += delta;
                if(temp1 < delta)
                {
                    sDelta_.erase(k_);
                }
        }
        delta /= 2;
    }
}

void setAnswer(int answer, int serverLocation[])
{
    answer_ = answer;
    int temp, answerArcListLength;
    answerServerLocation[0] = serverLocation[0];
    for(int i=1; i<=serverLocation[0]; i++)
    {
        answerServerLocation[i] = serverLocation[i];
    }
    for(int i=0; i<nodeNum_; i++)
    {
        temp = residualArcList_[i][0];
        answerArcList_[i][0].node = 0;
        for(int j=1; j<=temp; j++)
        {
            if(x_[i][j] > 0 && c_[i][j] > 0)
            {
                answerArcListLength = ++answerArcList_[i][0].node;
                answerArcList_[i][answerArcListLength].node = arcList_[i][(j+1)/2];
                answerArcList_[i][answerArcListLength].x = x_[i][j];
            }
        }
    }
}

int doubleScaling(int serverLocation[])
{
    memset(pi_, 0, sizeof(pi_));
    e_ = maxU_;
    while(e_ >= 1/(uncapacitedNodeNum_*1.0))
    {
        improveApproximation();
        e_ = e_ / 2;
    }
    //compared with answer_
    int temp = 0, temp2;
    for(int i=0; i<uncapacitedNodeNum_; i++) {
        temp2 = residualArcList_[i][0];
        for(int j=1; j<=temp2; j++)
        {
            temp += x_[i][j] * c_[i][j];
        }
    }
    temp += serverLocation[0] * serverCost_;
    if(temp < answer_)
    {
        setAnswer(temp, serverLocation);
    }
    return temp;
}