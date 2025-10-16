#ifndef UTIL_H
#define UTIL_H


#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>

#include <pthread.h>

#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/stat.h>

#define SIZE 20
#define SIZE_TOPIC 20
#define NUM_PERSISTANT_MESSAGES 5
#define MESSAGE_SIZE 300
#define NUM_USERS 10
#define NUM_TOPICS_MAX 20
#define FIFO_SRV "tubo"
#define FIFO_CLI "f_%d"

// Cliente
#define CMD_REGISTER 1
#define CMD_MESSAGE 2
#define CMD_SUBSCRIBE 3
#define CMD_UNSUBSCRIBE 4
#define CMD_TOPICS 5
#define CMD_EXIT 6

// SERVIDOR
#define CMD_USERS 1
#define CMD_REMOVE 2
#define CMD_TOPICS_SERV 3
#define CMD_SHOW 4
#define CMD_LOCK 5
#define CMD_UNLOCK 6
#define CMD_CLOSE 7

//----------------------------------MANAGER--------------------------------//
typedef struct {
    char persistantUser[SIZE];
    int persistantTime;
    char persistantMessage[MESSAGE_SIZE];
} PERSISTANT_MESSAGES;

typedef struct {
    char nameTopic[SIZE_TOPIC];
    int topicLocked;
    int numberPersistantMessages;
    PERSISTANT_MESSAGES persistantMessages[NUM_PERSISTANT_MESSAGES];
} CONTEXT_TOPICS;

typedef struct {
    int pid;
    char name[SIZE];
    char userTopics[NUM_TOPICS_MAX][SIZE_TOPIC];
    int numberSubscribedTopics;
} CONTEXT_USER; 

typedef struct {
    CONTEXT_USER activeUsers[NUM_USERS];
    CONTEXT_TOPICS topicsCreated[NUM_TOPICS_MAX];
    int numberActiveUsers; 
    int numberActiveTopics;
} CONTEXT;

typedef struct {
    CONTEXT* appContext;
    pthread_t mainThreadId;
    pthread_mutex_t *pLatch;
    int timerContinue;
} CONTEXT_THREADS;

//--------------------------------------FEED-----------------------------------//
typedef struct {
    int cmd;
    int pid;
    char user[SIZE];
} HEADER;

//----------ReceiveCommands---------//
typedef struct{
    int fdReceiveCommand;
    pthread_t tid;
} RECEIVE_COMMANDS_CLI;


//-------------Registo-----------//
typedef struct {
    char name[SIZE];
} REGISTER_REQUEST;

typedef struct {
    HEADER header;   
    REGISTER_REQUEST register_request;
} REGISTER_REQUEST_CLI;

//------------Message------------//
typedef struct {
    char text[MESSAGE_SIZE];
    char topic[SIZE];
    int msgSuccess;
    char user[SIZE];
    int timePersistent;
} MESSAGE;

typedef struct {
    HEADER header;   
    MESSAGE message;
} MESSAGE_CLI;

typedef struct {
    int cmd; 
    MESSAGE message;
} MESSAGE_SRV;

//-----------Subscribe---------//
typedef struct{
    char topic[SIZE_TOPIC];
    int subsSuccess;
} SUBSCRIBE;

typedef struct{
    HEADER header;
    SUBSCRIBE subscribe;
} SUBSCRIBE_CLI;

typedef struct{
    int cmd;
    SUBSCRIBE subscribe;
    int numberPersistantMessages;
    PERSISTANT_MESSAGES persistantMessages[NUM_PERSISTANT_MESSAGES];
} SUBSCRIBE_SRV;

//----------Unsubscribe--------//
typedef struct{
    char topic[SIZE_TOPIC];
    int unSubsSuccess;
} UNSUBSCRIBE;

typedef struct{
    HEADER header;
    UNSUBSCRIBE unsubscribe;
} UNSUBSCRIBE_CLI;

typedef struct{
    int cmd;
    UNSUBSCRIBE unsubscribe;
} UNSUBSCRIBE_SRV;

//----------TopicsList--------//
typedef struct{
    CONTEXT_TOPICS listTopics[NUM_TOPICS_MAX];
    int numberTopics;
    int topicsSuccess;
} TOPICS;

typedef struct{
    HEADER header;
    TOPICS topics;
} TOPICS_CLI;

typedef struct{
    int cmd;
    TOPICS topics;
} TOPICS_SRV;

//----------Exit---------//
typedef struct{
    HEADER header;
    int exitSuccess;
} EXIT_CLI;



#endif //UTIL_H

