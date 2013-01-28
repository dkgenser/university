/*
 * 213 Proxy Lab
 * Cathy Li (chli) & Dana Genser (dkg)
 *
 * This proxy only accepts http GET requests
 * Default port is 80, but will attempt a different port if indicated
 *
 * The Cache is comprised of smaller sets, where each set can only be accessed
 * by one thread at a time, for both reading and writing. One must lock the
 * cache for reading in additon to writing because the accessed cacheNode if
 * found is moved to the front of its cache set. This ensures that the LRU
 * cacheNode is the last in a set, therefore creating constant time evictions.
 *
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "csapp.h"

#define MAX_OBJ_SIZE 102400
#define NUM_THREADS 15
#define NUM_CACHE_SETS 5
#define MAX_SET_SIZE ((1024*1024)/(NUM_CACHE_SETS))

/* a structure that stores processed request info */
struct Request {
    int port;
    char* uri; 
    char* urn; 
};

/* a struct that stores cache strings */
struct CacheNode {
    struct CacheNode* prev;
    struct CacheNode* next;
    struct Request * req;
    char * data;
    int size;
};

/* Cache: an array for the heads, tails, and sizes of each cache set*/
static struct CacheNode* (heads[NUM_CACHE_SETS]);
static struct CacheNode* (tails[NUM_CACHE_SETS]);
static int sizes[NUM_CACHE_SETS];

/* semaphores for the cache: one for each set */
static sem_t cacheAccess[NUM_CACHE_SETS];

/* semaphore for the threads */
static sem_t threads;

/* prototypes for all proxy functions */
int parseRequest(char* instruction, struct Request * request);
void freeStruct(struct Request * req);
int parseHdrs(rio_t *rS, int clientfd, struct Request* r);
void readRequestHdrs(rio_t *rp);
void cleanUp(char* responseLine, int connectfd);
void* serveClient(void* arg);
void addToCache(int setNum, struct CacheNode* newInfo);
void evict(int setNum);
struct CacheNode* retrieve(int setNum, struct Request* req);
void freeCacheNode(struct CacheNode* cacheInfo);
int setNumCreator(char* str);

/*
 * main: the main procedure for the proxy: listens at a socket
 * and passes any incoming client connections to a helper thread
 */
int main(int argc, char *argv []) {
    int port, socket, connect;
    unsigned int clientlen;
    struct sockaddr_in clientaddr;
    if (argc != 2) {
        printf("Usage: %s <port> \n", argv[0]);
        return 0;
    }

    port = atoi(argv[1]);
    if (port < 1) {
        printf("Port number needs to be non-zero positive value\n");
        return -1;
    }

    Signal(SIGPIPE, SIG_IGN);

    // open socket
    if ((socket = open_listenfd(port)) == -1) {
        printf("Socket/port cannot be opened\n");
        return -1;
    }

    pthread_t tid;
    // initializes the semaphore for threads
    sem_init(&threads, 0, NUM_THREADS);

    int i;
    // inits cache, including cache semaphores
    for (i = 0; i < NUM_CACHE_SETS; i++) {
        heads[i] = NULL;
        tails[i] = NULL;
        sizes[i] = 0;
        sem_init(&cacheAccess[i], 0, 1);
    }


    while (1) {
        clientlen = sizeof (clientaddr);
        // accepts something from the client.  Otherwise, try again
        if ((connect = accept(socket, &clientaddr, &clientlen)) < 0) {
            printf("Accept failed: %s\n", strerror(errno));
            continue;
        }

        // lock the thread semaphore and starts a new thread to
        // take care of the client trying to connect
        sem_wait(&threads);
        pthread_create(&tid, NULL, serveClient, (void*) ((long) connect));

    }

    return 0;
}

/*
 * serveClient: handles reading from the client pipe, parse
 * the requests that the client sends, connecting to the host
 * the client wants to connect to, and feed the results
 * back to the client
 *
 * serveClient is made to be a thread function, so each thread will
 * be running this function as its main execution
 *
 * return: NULL
 */
void * serveClient(void * arg) {

    pthread_detach(pthread_self());

    int clientfd, serverfd = (int) ((long) arg), num;
    rio_t rioS, rioC;
    struct Request* r = malloc(sizeof (struct Request));
    char buf[MAXLINE];

    int n;
    rio_readinitb(&rioS, serverfd);

    /* read in first line */
    if ((n = rio_readlineb(&rioS, buf, MAXLINE)) != 0) {
        printf("Request: %s\n", buf);
    }

    /* parse out server and port to open connection*/
    if (0 > (n = parseRequest(buf, r))) {
        if(n == -1) cleanUp("HTTP/1.0 400 Bad Request\r\n\r\n", serverfd);
        else cleanUp("HTTP/1.0 405 Method Not Allowed \r\n\r\n", serverfd);
        sem_post(&threads);
        free(r);
        return NULL;
    }

    /*
     * In order to avoid the situation where a node is moved or free'd
     * while we are writing to the client, we create a copy from which
     * we can safetly write, while also free'ing access to the cache
     * so others may read.
     */
    int cacheCode;
    struct CacheNode * res;
    // Obtain correct set number
    cacheCode = setNumCreator(r->uri);
    sem_wait(&cacheAccess[cacheCode]);
    // One must lock the cache while reading for a CacheNode is moved to
    // the beginning of its set upon access
    res = retrieve(cacheCode, r);
    char * page;
    int size;
    if (res) {
        page = malloc(res->size);
        size = res->size;
        memcpy(page, res->data, res->size);
    } else
        page = NULL;
    sem_post(&cacheAccess[cacheCode]);
    
    if (page) {
        rio_writen(serverfd, page, size);
        free(page);
        close(serverfd);
        freeStruct(r);
        sem_post(&threads);
        return NULL;
    }


    if (0 > (clientfd = open_clientfd(r->uri, r->port))) {
        //Could not open connection to server
        cleanUp("HTTP/1.0 418 I'm a teapot\r\n\r\n", serverfd);
        freeStruct(r);
        sem_post(&threads);
        return NULL;
    }


    /* Pass request to server, changing fields as neccessary */
    num = parseHdrs(&rioS, clientfd, r);

    if (0 > num) {
        //Could not pass headers to server
        cleanUp("HTTP/1.0 502 Bad Gateway\r\n\r\n", serverfd);
        freeStruct(r);
        close(clientfd);
        sem_post(&threads);
        return NULL;
    }

    rio_readinitb(&rioC, clientfd);
    char buffer[MAX_OBJ_SIZE];
    buffer[0] = '\0';
    int numRead = 0;
    int flag = 0;

    /* Read response from server, pass onto client */
    while ((n = rio_readlineb(&rioC, buf, MAXLINE)) != 0) {
        num = rio_writen(serverfd, buf, n);

        if (num < 0) {
            printf("Can no longer write to client\n");
            flag = 1;
            break;
        }

        if (num != n) {
            printf("Could not complete write: wrote %d of %d", num, (int) n);
            flag = 1;
            break;
        }

        if (numRead + n < MAX_OBJ_SIZE) {
            memcpy(buffer + numRead, buf, n);
            numRead += n;
        } else
            flag = 1;
    }


    if (!flag) {
        res = malloc(sizeof (struct CacheNode));
        res->size = numRead;
        res->data = (char *) malloc((size_t) numRead);
        memcpy(res->data, buffer, numRead);
        res->req = r;
        sem_wait(&cacheAccess[cacheCode]);
        //Ensure that a duplicate entry has not been added in the mean time
        if(retrieve(cacheCode, r)==NULL)
            addToCache(cacheCode, res);
        sem_post(&cacheAccess[cacheCode]);
    } else {
        freeStruct(r);
    }

    close(clientfd);
    close(serverfd);
    sem_post(&threads);
    return NULL;
}

/* 
 * parseRequest: Parses the main request line sent from the client
 * returns: 0 upon successful completion, -1 otherwise
 */
int parseRequest(char* instruction, struct Request* request) {

    // for use to keep strtok thread-safe
    char * strtokPtr;

    char inst[21], url[MAXLINE], http[10], buf[MAXLINE];
    char *serverPort, *req, *port;
    //int ret = 0;

    /*Parse by whitespace*/
    if (sscanf(instruction, "%s %s %s", inst, url, http) < 3) {
        return -1;
    }

    if (strstr("HTTP/1.", http) < 0) {
        return -1;
    }

    if (strcmp(inst, "GET")) {
        printf("Unsupported proxy instruction: %s\n", inst);
        return -2;
    }

    /* Parse server, port, request out of URL*/
    // remove http or https from request if applicable
    if (strstr(url, "http")) {
        strtok_r(url, "/", &strtokPtr);
        serverPort = strtok_r(NULL, "/", &strtokPtr);
    } else
        serverPort = strtok_r(url, "/", &strtokPtr);

    if (!serverPort)
        return -1;
    req = strtok_r(NULL, "\0", &strtokPtr);

    /* Assign server and port*/
    request->uri = strdup(strtok_r(serverPort, ":", &strtokPtr));
    port = strtok_r(NULL, " ", &strtokPtr);
    if (!port)
        request->port = 80;
    else
        request->port = atoi(port);

    /*combine inst, request, HTTP version: assign request*/
    if (req) {
        //11 accounts for " HTTP/1.0\r\n"
        char* temp = malloc(strlen(req) + 11 + 1);
        strcpy(temp, req);
        req = strcat(temp, " HTTP/1.0\r\n");
    } else
        req = strdup(" HTTP/1.0\r\n");

    strcpy(buf, inst);
    request->urn =
            strdup(strncat(strcat(buf, " /"), req, MAXLINE - (strlen(buf) + 1)));
    free(req);


    //printf("SERVER: %s REQUEST: %s PORT %d\n",
            //request->uri, request->urn, request->port);

    return 0;
}

/* passes client request info and headers to the server */
int parseHdrs(rio_t *rS, int clientfd, struct Request* r) {
    char buf[MAXLINE];
    int num, n;

    num = rio_writen(clientfd, r->urn, strlen(r->urn));
    if (num < 0) return num;

    //6 accounts for "Host: ", 4 accounts for \r\n\r\n
    char *temp = malloc((size_t) (6 + strlen(r->uri) + 4 + 1));
    strcpy(temp, "Host: ");
    temp = strcat(strcat(temp, r->uri), "\r\n\r\n");
    num = rio_writen(clientfd, temp, strlen(temp));
    free(temp);
    if (num < 0) return num;

    n = rio_readlineb(rS, buf, MAXLINE);
    while (n != 0 && strcmp(buf, "\r\n")) {
        if ((!strstr(buf, "Host: ")) && (!strstr(buf, "Keep-Alive: "))) {
            if (strstr(buf, "Proxy-Connection: ")) {
                strcpy(buf, "Proxy-Connection: close\r\n");
            }
            else if (strstr(buf, "Connection: ")) {
                strcpy(buf, "Connection: close\r\n");
            }
            num = rio_writen(clientfd, buf, strlen(buf));
            if (num < 0) return num;
        }
        n = rio_readlineb(rS, buf, MAXLINE);
    }

    return num;
}

/*
 * cleanUp: cleans up after errors
 * passes responseLine to connectfd, closes connectfd, and prints
 * error message to terminal
 */
void cleanUp(char* responseLine, int connectfd) {
    printf("Error: %s\n", responseLine);
    rio_writen(connectfd, responseLine, strlen(responseLine));
    close(connectfd);
}

/* 
 * freeStruct: frees the elements in a request struct 
 */
void freeStruct(struct Request* req) {
    free(req->uri);
    free(req->urn);
    free(req);
}

/* 
 * setNumCreator: acts as a hashCode function.  Returns
 * a number between [0, NUM_CACHE_SETS] 
 *     used to determine in which set a cacheNode belongs
 */
int setNumCreator(char* str) {
    int num = 0, index = 0;
    while (str[index] != '\0' && index < 50) {
        num += 7 * num + (int) str[index];
        index += 1;
    }
    num = num % NUM_CACHE_SETS;
    return num < 0 ? num + NUM_CACHE_SETS : num;
}

/* 
 * retrieve: retrieves the request from a cacheNode in set setNum
 * returns a pointer to the matching cacheNode if found, NULL otherwise
 */
struct CacheNode* retrieve(int setNum, struct Request* req) {

    if (setNum >= NUM_CACHE_SETS) {
        return NULL;
    }

    struct CacheNode* temp = heads[setNum];
    while (temp) {
        if (!strcmp(temp->req->uri, req->uri)
                && !strcmp(temp->req->urn, req->urn))
            break;
        temp = temp->next;
    }

    /*move cacheNode temp to beginning of set*/
    if (temp && temp != heads[setNum]) {
        temp->prev->next = temp->next;
        if (temp->next)
            temp->next->prev = temp->prev;
        else {
            tails[setNum] = temp->prev;
        }
        sizes[setNum] -= temp->size;
        temp->prev = NULL;
        temp->next = NULL;
        addToCache(setNum, temp);
    }

    return temp;
}

/* 
 * addToCache: adds cacheNode newInfo to the head of set setNum 
 */
void addToCache(int setNum, struct CacheNode* newInfo) {
    while (sizes[setNum] + newInfo->size > MAX_SET_SIZE) {
        evict(setNum);
    }
    newInfo->next = heads[setNum];
    newInfo->prev = NULL;
    if (heads[setNum])
        heads[setNum]->prev = newInfo;
    else
        tails[setNum] = newInfo;
    heads[setNum] = newInfo;
    sizes[setNum] += newInfo->size;
}

/* 
 * evict: removes the last cacheNode in set setNum
 */
void evict(int setNum) {
    struct CacheNode* temp = tails[setNum];
    if (temp->prev)
        temp->prev->next = NULL;
    else
        heads[setNum] = NULL;
    tails[setNum] = temp->prev;
    sizes[setNum] -= temp->size;
    freeCacheNode(temp);
}

/* 
 * freeCacheNode: frees the elements in the cache node 
 */
void freeCacheNode(struct CacheNode* cacheInfo) {
    freeStruct(cacheInfo->req);
    free(cacheInfo->data);
    free(cacheInfo);
}






