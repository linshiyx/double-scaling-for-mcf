#ifndef __ROUTE_H__
#define __ROUTE_H__

#include <ctime>
#include "lib_io.h"

const int MAX_NODE_NUM = 1002;
const int MAX_CONSUMER_NUM = 500;
const int MAX_ARC_PER_NODE = 500;
const int MAX_ARC_NUM = 20000;
const int MAX_UNCAPACITED_NODE_NUM = MAX_NODE_NUM + MAX_ARC_NUM + 1;
const int SIZE_OF_X = MAX_UNCAPACITED_NODE_NUM * (MAX_ARC_PER_NODE + 1) * 4;
const int INF = 1000000000;

void deploy_server(char * graph[MAX_EDGE_NUM], int edge_num, char * filename);

	

#endif
