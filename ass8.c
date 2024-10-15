#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_THREADS 35
#define NUM_NODES 100

// Graph data structure
typedef struct Graph {
  int numNodes;
  int** adjMatrix;
} Graph;

// Global files  
FILE* pathToFile;
FILE* pathOutFile;
FILE* stitchOutFile;

Graph* createGraph(int numNodes) {
  Graph* g = (Graph*) malloc(sizeof(Graph));
  g->numNodes = numNodes;

  g->adjMatrix = (int**) malloc(numNodes * sizeof(int*));
  for(int i=0; i<numNodes; i++) {
    g->adjMatrix[i] = (int*) malloc(numNodes * sizeof(int));
    for(int j=0; j<numNodes; j++) {
      g->adjMatrix[i][j] = 0;
    }
  }

  return g;
}

// Load graph from file
Graph* loadGraph(char* filename) {
  
  FILE* f = fopen(filename, "r");
  
  int numNodes;
  fscanf(f, "%d", &numNodes);
  
  Graph* g = createGraph(numNodes);
  
  while(!feof(f)) {
    int u, v;
    fscanf(f, "%d %d", &u, &v);
    
    g->adjMatrix[u][v] = 1;
    g->adjMatrix[v][u] = 1;
  }

  fclose(f);

  return g;
}

// Graph globals
Graph* g;
int numLandmarks;
int* landmarks;
int* nodePartitions;

// Mutex
pthread_mutex_t graphLock;

// Struct to pass data to threads
typedef struct {
  int threadId;
} ThreadData;

// Function prototypes
void* graphUpdateThread(void* arg);
void* pathFinderThread(void* arg);  
void* pathStitcherThread(void* arg);
int* shortestPath(Graph* g, int src, int dest);
void printPath(int path[], int len);

// Shortest path code
int* shortestPath(Graph* g, int src, int dest) {

  int* dist = malloc(g->numNodes * sizeof(int));
  
  // Initialize 
  for(int i = 0; i < g->numNodes; i++) {
    dist[i] = INT_MAX;
  }

  dist[src] = 0;

  // Relax edges
  for(int i = 0; i < g->numNodes; i++) {
    for(int u = 0; u < g->numNodes; u++) {
      for(int v = 0; v < g->numNodes; v++) {
        if(g->adjMatrix[u][v] && dist[u] != INT_MAX 
           && dist[u] + g->adjMatrix[u][v] < dist[v]) {
          dist[v] = dist[u] + g->adjMatrix[u][v]; 
        }
      }
    }
  }

  // Construct path 
  int* path = malloc(g->numNodes * sizeof(int));
  int current = dest;

  while(current != src) {
    // Find next hop
    // Add to path
  }

  return path;

}


// Print path
void printPath(int path[], int len) {
  
  for(int i = 0; i < len; i++) {
    fprintf(pathOutFile, "%d ", path[i]); 
  }
}

// Main driver function
int main() {

  // Load graph
  g = loadGraph("graph.txt");
  pathToFile = fopen("paths.txt", "r");
  pathOutFile = fopen("path_out.txt", "w");
  stitchOutFile = fopen("stitched.txt", "w");
  
  // Create landmarks
  numLandmarks = 100;
  landmarks = (int*) malloc(numLandmarks * sizeof(int));

  // Partition nodes
  nodePartitions = (int*) malloc(g->numNodes * sizeof(int));

  // Assign partitions
  for(int i=0; i<g->numNodes; i++) {
    nodePartitions[i] = i % numLandmarks; 
  }

  // Initialize mutex
  pthread_mutex_init(&graphLock, NULL);

  // Start threads
  pthread_t threads[NUM_THREADS];
  ThreadData threadData[NUM_THREADS];

  for(int i=0; i<NUM_THREADS; i++) {
    threadData[i].threadId = i;
    
    if(i < 5) {
      pthread_create(&threads[i], NULL, graphUpdateThread, &threadData[i]);
    }
    else if(i < 25) {
      pthread_create(&threads[i], NULL, pathFinderThread, &threadData[i]);
    }
    else {
      pthread_create(&threads[i], NULL, pathStitcherThread, &threadData[i]); 
    }
  }

  // Join all threads 
  for(int i=0; i<NUM_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }

  // Destroy mutex
  pthread_mutex_destroy(&graphLock);

  return 0;
}

// Graph update thread
void* graphUpdateThread(void* arg) {

  ThreadData* data = (ThreadData*) arg;
  int threadId = data->threadId;

  while(1) {
    
    // Pick random edge
    int u = rand() % g->numNodes; 
    int v = rand() % g->numNodes;

    pthread_mutex_lock(&graphLock);

    // Add or remove edge
    if(g->adjMatrix[u][v] == 0) {
      g->adjMatrix[u][v] = 1;
      g->adjMatrix[v][u] = 1; 
    }
    else {
      g->adjMatrix[u][v] = 0;
      g->adjMatrix[v][u] = 0;
    }

    pthread_mutex_unlock(&graphLock);

    sleep(1); 
  }
}

// Path finder thread  
void* pathFinderThread(void* arg) {

  ThreadData* data = (ThreadData*) arg;
  int threadId = data->threadId;

  while(1) {

    // Get next node pair
    int u, v;
    fscanf(pathToFile, "%d %d", &u, &v);

    // Find shortest path from u to its landmark
    int landmarkU = nodePartitions[u];  
    int* pathU = shortestPath(g, u, landmarkU);

    // Find shortest path from v to its landmark 
    int landmarkV = nodePartitions[v];
    int* pathV = shortestPath(g, v, landmarkV);

    // Write paths to file
    fprintf(pathOutFile, "%d ", u);
    printPath(pathU, sizeof(pathU)/sizeof(pathU[0]));  
    fprintf(pathOutFile, "%d ", landmarkU);

    fprintf(pathOutFile, "%d ", v);
    printPath(pathV, sizeof(pathV)/sizeof(pathV[0]));
    fprintf(pathOutFile, "%d\n", landmarkV);

  }

  return NULL;

}

// Path stitcher thread
void* pathStitcherThread(void* arg) {

  ThreadData* data = (ThreadData*) arg;
  int threadId = data->threadId;

  while(1) {

    // Read next two paths 
    int u, lu, v, lv;
    fscanf(pathOutFile, "%d %d %d %d", &u, &lu, &v, &lv);

    // Stitch landmark paths
    int* stitchedPath = shortestPath(g, lu, lv);

    // Write stitched path 
    fprintf(stitchOutFile, "%d ", u);
    printPath(stitchedPath, sizeof(stitchedPath)/sizeof(stitchedPath[0]));
    fprintf(stitchOutFile, "%d\n", v);

  }

  return NULL;

}