#include "util.h"

CONTEXT_USER* findActiveUser(CONTEXT *app_context, int pidToFind){
    for(int i = 0; i < app_context->numberActiveUsers; i++) {
        if(app_context->activeUsers[i].pid == pidToFind)  {
            return &(app_context->activeUsers[i]);
        }
    }
    return NULL;
}

int isNameAvailable(CONTEXT app_context, char* new_name){
    for (int i = 0; i < app_context.numberActiveUsers; i++) {
        CONTEXT_USER user = app_context.activeUsers[i];

        if (strcmp(user.name, new_name) == 0) {
            return 0;
        }
    }

    return 1;
}

void registerUser(CONTEXT *app_context, REGISTER_REQUEST register_request, HEADER header) {
    int *numberActiveUsers = &(app_context->numberActiveUsers);
    CONTEXT_USER newUser;
    strcpy(newUser.name, register_request.name);
    newUser.pid = header.pid;
    app_context->activeUsers[*numberActiveUsers] = newUser;
    (*numberActiveUsers)++;
}

int isTopicAvailable(CONTEXT app_context, char* newTopic) {
    for (int i = 0; i < app_context.numberActiveTopics; i++) {
        if (strcmp(newTopic, app_context.topicsCreated[i].nameTopic) == 0) { 
            return 0;
        }
    }
    return 1;
}

int isUserSubscribedTopic(CONTEXT app_context, char* topic, int pidToFind){
    CONTEXT_USER *user = findActiveUser(&app_context, pidToFind);
    if (user == NULL) {
        printf("[MANAGER] Erro - Usuário com PID %d não encontrado.\n", pidToFind);
        return 0;
    }

    for(int j = 0; j < app_context.numberActiveTopics; j++) {
        if(strcmp(user->userTopics[j], topic) == 0){
            return 1;
        }
    }  
    return 0;
}

int isTopicLocked(CONTEXT app_context, char* topic){
    for(int i = 0; i < app_context.numberActiveTopics; i++) {
        if(strcmp(app_context.topicsCreated[i].nameTopic, topic) == 0){
            if (app_context.topicsCreated[i].topicLocked == 1){
                return 1;
            } else if(app_context.topicsCreated[i].topicLocked == 0){
                return 0;
            }
        }
    }

    return 0;
}

int createTopicWithUser(CONTEXT *app_context, char* newTopic, HEADER header) {
    int *numberActiveTopics = &(app_context->numberActiveTopics);
    if(*numberActiveTopics == NUM_TOPICS_MAX) {
        printf("[MANAGER] O user '%s' tentou criar o tópico '%s', mas o número tópicos criados já passou do limite.\n", header.user, newTopic);
        return 0;
    }
    strcpy(app_context->topicsCreated[*numberActiveTopics].nameTopic, newTopic); // Copia o nome do Tópico
    (*numberActiveTopics)++;
    
    CONTEXT_USER *user = findActiveUser(app_context, header.pid);
    if (user == NULL) {
        printf("[MANAGER] Erro - Usuário com PID %d não encontrado.\n", header.pid);
        return 0;
    }
    int *numberSubscribedTopics = &(user->numberSubscribedTopics);
    strcpy(user->userTopics[*numberSubscribedTopics], newTopic); // Subscrição do Tópico
    (*numberSubscribedTopics)++;
    return 1;
}

int userSubscribeTopic(CONTEXT *app_context, char* topicToSubs, HEADER header) {
     CONTEXT_USER *user = findActiveUser(app_context, header.pid);
    if (user == NULL) {
        printf("[MANAGER] Erro - Usuário com PID %d não encontrado.\n", header.pid);
        return 0;
    }

    int *numberSubscribedTopics = &(user->numberSubscribedTopics);
    strcpy(user->userTopics[*numberSubscribedTopics], topicToSubs); // Subscrição do Tópico
    (*numberSubscribedTopics)++;

    return 1;
}


int removeTopic(CONTEXT *app_context, char* topicToUnsub, int pidToNotCheck) {
    int iTopicToRemove = -1;
    int *numberActiveTopics = &(app_context->numberActiveTopics);
    for (int i = 0; i < app_context->numberActiveUsers; i++) {
        for (int j = 0; j < app_context->activeUsers[i].numberSubscribedTopics; j++) {
            if (strcmp(app_context->activeUsers[i].userTopics[j], topicToUnsub) == 0 && app_context->activeUsers[i].pid != pidToNotCheck) {
                return 0;
            }
        }
    }

    for (int i = 0; i < *numberActiveTopics; i++) {
        if (strcmp(app_context->topicsCreated[i].nameTopic, topicToUnsub) == 0 && app_context->topicsCreated[i].numberPersistantMessages == 0) {
            iTopicToRemove = i;
            printf("[MANAGER] Encontrou um tópico com 0 mensagens persistentes '%s'.\n", app_context->topicsCreated[i].nameTopic);
            break;
        }
    }

    if (iTopicToRemove == -1) {
        //printf("[MANAGER] Tópico '%s' não pode ser removido, pois ainda tem mensagens persistentes.\n", topicToUnsub);
        return 0;
    }

    printf("[MANAGER] O tópico '%s' foi removido da lista de tópicos criados.\n", topicToUnsub);

    strcpy(app_context->topicsCreated[iTopicToRemove].nameTopic, app_context->topicsCreated[*numberActiveTopics - 1].nameTopic);
    app_context->topicsCreated[iTopicToRemove].topicLocked = app_context->topicsCreated[*numberActiveTopics - 1].topicLocked;
    app_context->topicsCreated[iTopicToRemove].numberPersistantMessages = app_context->topicsCreated[*numberActiveTopics - 1].numberPersistantMessages;
    for(int k = 0; k < NUM_PERSISTANT_MESSAGES; k++) { 
        app_context->topicsCreated[iTopicToRemove].persistantMessages[k] = app_context->topicsCreated[*numberActiveTopics - 1].persistantMessages[k];
    }

    app_context->topicsCreated[*numberActiveTopics - 1].nameTopic[0] = '\0';
    app_context->topicsCreated[*numberActiveTopics - 1].topicLocked = 0;
    app_context->topicsCreated[*numberActiveTopics - 1].numberPersistantMessages = 0;
    memset(&app_context->topicsCreated[*numberActiveTopics - 1], 0, sizeof(CONTEXT_TOPICS));

    (*numberActiveTopics)--;

    return 1;
}


int userUnsubscribeTopic(CONTEXT *app_context, char* topicToUnsub, HEADER header) {
    CONTEXT_USER *user = findActiveUser(app_context, header.pid);
    if (user == NULL) {
        printf("[MANAGER] Erro - Usuário com PID %d não encontrado.\n", header.pid);
        return 0;
    }

    int iTopicRemoved = -1;
    int *numberSubscribedTopics = &(user->numberSubscribedTopics);

    // Localizar índice do tópico a ser removido
    for (int i = 0; i < *numberSubscribedTopics; i++) {
        if (strcmp(user->userTopics[i], topicToUnsub) == 0) {
            iTopicRemoved = i;
            break;
        }
    }

    if (iTopicRemoved == -1) {
        printf("[MANAGER] Erro - O tópico '%s' não foi encontrado para o usuário.\n", topicToUnsub);
        return 0;
    }

    // Substituir o tópico removido pelo último tópico do array
    strcpy(user->userTopics[iTopicRemoved], user->userTopics[*numberSubscribedTopics - 1]);
    user->userTopics[*numberSubscribedTopics - 1][0] = '\0';

    (*numberSubscribedTopics)--;

    // Remover o tópico globalmente, se aplicável
    removeTopic(app_context, topicToUnsub, -1);

    return 1;
}


int copyTopicsListToTopicsCli(TOPICS *topics, CONTEXT app_context){
    if(app_context.numberActiveTopics == 0) {
        topics->topicsSuccess = 0;
        return 0;
    }
    for(int i = 0; i < app_context.numberActiveTopics; i++) {
        strcpy(topics->listTopics[i].nameTopic, app_context.topicsCreated[i].nameTopic);
        topics->listTopics[i].topicLocked = app_context.topicsCreated[i].topicLocked;
        topics->listTopics[i].numberPersistantMessages = app_context.topicsCreated[i].numberPersistantMessages;
    }
    topics->numberTopics = app_context.numberActiveTopics;
    topics->topicsSuccess = 1;

    return 1;
}

void removeActiveUser(CONTEXT *app_context, char* name, int *removeSuccess) {
    int iToRemove = -1;
    for(int i = 0; i < app_context->numberActiveUsers; i++) {
        if(strcmp(app_context->activeUsers[i].name, name) == 0)  {
            iToRemove = i;
        }
    }
    if(iToRemove == -1) {
        printf("[MANAGER] Não existe ninguém na lista dos utilizadores ativos com esse nome\n");
        *removeSuccess = 0;
        return;
    }
    
    for(int k = 0; k < app_context->activeUsers[iToRemove].numberSubscribedTopics; k++) {
        removeTopic(app_context, app_context->activeUsers[iToRemove].userTopics[k], app_context->activeUsers[iToRemove].pid);
    }
    
    int *numberActiveUsers = &(app_context->numberActiveUsers);

    app_context->activeUsers[iToRemove] = app_context->activeUsers[*numberActiveUsers - 1];
    (*numberActiveUsers)--;


    *removeSuccess = 1;
}

void lockTopic(CONTEXT *app_context, char* nameTopicToLock, int *lockSuccess) {
    int iToLock = -1;
    for(int i = 0; i < app_context->numberActiveTopics; i++) {
        if(strcmp(app_context->topicsCreated[i].nameTopic, nameTopicToLock) == 0)  {
            iToLock = i;
        }
    }
    if(iToLock == -1) {
        printf("[MANAGER] Não existe nenhum tópico ativo com esse nome.\n");
        *lockSuccess = 0;
        return;
    }
    
    app_context->topicsCreated[iToLock].topicLocked = 1;
    *lockSuccess = 1;
}

void unlockTopic(CONTEXT *app_context, char* nameTopicToUnlock, int *unlockSuccess) {
    int iToUnlock = -1;
    for(int i = 0; i < app_context->numberActiveTopics; i++) {
        if(strcmp(app_context->topicsCreated[i].nameTopic, nameTopicToUnlock) == 0)  {
            iToUnlock = i;
        }
    }
    if(iToUnlock == -1) {
        printf("[MANAGER] Não existe nenhum tópico ativo com esse nome.\n");
        *unlockSuccess = 0;
        return;
    }
    
    app_context->topicsCreated[iToUnlock].topicLocked = 0;
    *unlockSuccess = 1;
}

void writeMessagePersistant(CONTEXT *app_context, MESSAGE message) {   
    int iToAddPersistantMessage = -1;
    for(int i = 0; i < app_context->numberActiveTopics; i++) {
        if(strcmp(app_context->topicsCreated[i].nameTopic, message.topic) == 0) {
            if(app_context->topicsCreated[i].numberPersistantMessages >= 5) {
                printf("[MANAGER] Este tópico '%s' já chegou ao limite de mensagens persistentes.\n", message.topic);
                return;
            }
            iToAddPersistantMessage = i;
        }
    }

    int *numberPersistantMessages = &(app_context->topicsCreated[iToAddPersistantMessage].numberPersistantMessages);
    strcpy(app_context->topicsCreated[iToAddPersistantMessage].persistantMessages[*numberPersistantMessages].persistantUser, message.user);
    app_context->topicsCreated[iToAddPersistantMessage].persistantMessages[*numberPersistantMessages].persistantTime = message.timePersistent;
    strcpy(app_context->topicsCreated[iToAddPersistantMessage].persistantMessages[*numberPersistantMessages].persistantMessage, message.text);

    (*numberPersistantMessages)++;
}

int createTopic(CONTEXT *app_context, char* newTopic) {
    if(app_context->numberActiveTopics >= NUM_TOPICS_MAX) {
        printf("[MANAGER] O tópico '%s' não foi criado pelas mensagens persistentes do ficheiro, pois o servidor já atingiu o número máximo de tópicos criados.\n", newTopic);
        return 0;
    }
    for(int i = 0; i < app_context->numberActiveTopics; i++) {
        if(strcmp(app_context->topicsCreated[i].nameTopic, newTopic) == 0) {
            return 0;
        }
    }
    
    int *numberActiveTopics = &(app_context->numberActiveTopics);
    strcpy(app_context->topicsCreated[*numberActiveTopics].nameTopic, newTopic); // Copia o nome do Tópico
    (*numberActiveTopics)++;
    printf("[MANAGER] O tópico '%s' foi criado pelas mensagens persistentes do ficheiro.\n", newTopic);
    return 1;
}

void createPersistantMessage(CONTEXT *app_context, char* topic, char* persistantUser, int persistantTime, char* persistantMessage) {
    int iTopicToAddPersistantMessage = -1;
    for(int i = 0; i < app_context->numberActiveTopics; i++) {
        if(strcmp(app_context->topicsCreated[i].nameTopic, topic) == 0) {
            iTopicToAddPersistantMessage = i;
            break;
        }
    }

    int *numberPersistantMessages = &(app_context->topicsCreated[iTopicToAddPersistantMessage].numberPersistantMessages);
    
    if(*numberPersistantMessages < 5) {
        app_context->topicsCreated[iTopicToAddPersistantMessage].persistantMessages[*numberPersistantMessages].persistantTime = persistantTime;
        strcpy(app_context->topicsCreated[iTopicToAddPersistantMessage].persistantMessages[*numberPersistantMessages].persistantUser, persistantUser);
        strcpy(app_context->topicsCreated[iTopicToAddPersistantMessage].persistantMessages[*numberPersistantMessages].persistantMessage, persistantMessage);
        (*numberPersistantMessages)++;
    } else {
        printf("[MANAGER] Não é possível adicionar a mensagem persistente '%s' do user '%s' ao tópico '%s', pois este já atingiu o máximo de mensagens persistentes", persistantMessage, persistantUser, topic);
    }
}

void readDataFromFile(CONTEXT *app_context, FILE* file) {
    char fileLine[330];
    char topic[SIZE_TOPIC], username[SIZE], message[MESSAGE_SIZE];
    int persistantTime;
    while (fgets(fileLine, sizeof(fileLine), file) != NULL) {
        // Separar a frase usando sscanf
        sscanf(fileLine, "%19s %19s %d %[^\n]", topic, username, &persistantTime, message);

        createTopic(app_context, topic);

        createPersistantMessage(app_context, topic, username, persistantTime, message);
    }

    printf("\n");
}

void writeDataInFile(CONTEXT *app_context, FILE* file) {
    for (int i = 0; i < app_context->numberActiveTopics; i++) {
        CONTEXT_TOPICS *currentTopic = &app_context->topicsCreated[i];
        for (int j = 0; j < currentTopic->numberPersistantMessages; j++) {
            PERSISTANT_MESSAGES *message = &currentTopic->persistantMessages[j];
            fprintf(file, "%s %s %d %s\n",
                    currentTopic->nameTopic,
                    message->persistantUser,
                    message->persistantTime,
                    message->persistantMessage);
        }
    }
}

void leave(int signal) { // Função de tratamento de sinal
}

int commandCharToInt(char *commandChar) {
    if(strcmp(commandChar, "users") == 0) {
        return CMD_USERS;
    }
    if(strcmp(commandChar, "remove") == 0) {
        return CMD_REMOVE;
    }
    if(strcmp(commandChar, "topics") == 0) {
        return CMD_TOPICS_SERV;
    }
    if(strcmp(commandChar, "show") == 0) {
        return CMD_SHOW;
    }
    if(strcmp(commandChar, "lock") == 0) {
        return CMD_LOCK;
    }
    if(strcmp(commandChar, "unlock") == 0) {
        return CMD_UNLOCK;
    }
    if(strcmp(commandChar, "close") == 0) {
        return CMD_CLOSE;
    }
    
    return -1;
}

void commandToText(char* str, int* j, char* msgTopic) {
    int k = 0;
    (*j)++; // salta o espaço msg(este espaço)textodamensagem   
    for ( ; str[*j] != ' ' && str[*j] != '\0'; (*j)++) {
        msgTopic[k] = str[*j];
        k++;
    }
    msgTopic[k] = '\0';
}

void *managerAdminCommands(void *pContext_threads) {
    CONTEXT_THREADS *pContextThreads = (CONTEXT_THREADS *) pContext_threads;

    char str[340], commandChar[15], userToRemove[SIZE], fifo_cli[SIZE], topic[SIZE_TOPIC];
    int command, i, j, fd_cli;
    int removeSucess = 0, lockSuccess = 0, unlockSuccess = 0;
    int nBytesSentExit, pidToRemove, flagExistsTopic = 0;
    while(1){
        scanf(" %[^\n]", str);
        for(i = 0; str[i] != ' ' && str[i] != '\0'; i++);
        for(j = 0; j < i; j++) {
            commandChar[j] = str[j];
        }
        commandChar[j] = '\0';
        command = commandCharToInt(commandChar);
        switch (command) {
            case CMD_USERS:
                if(pContextThreads->appContext->numberActiveUsers == 0) {
                    printf("[MANAGER] Não nenhum utilizador ativo de momento.\n");
                    printf("\n");
                    break;
                }

                printf("[MANAGER] Lista de utilizadores ativos:\n");
                pthread_mutex_lock(pContextThreads->pLatch); // MUTEX
                for(int i = 0; i < pContextThreads->appContext->numberActiveUsers; i++) {
                    printf("\t - User[%d]: %s\n", i + 1, pContextThreads->appContext->activeUsers[i].name);
                }
                pthread_mutex_unlock(pContextThreads->pLatch); // MUTEX
                printf("\n");
                break;

            case CMD_REMOVE:
                removeSucess = 0;
                commandToText(str, &i, userToRemove);
                if(pContextThreads->appContext->numberActiveUsers == 0) {
                    printf("[MANAGER] Não existe nenhum utilizador ativo de momento.\n");
                    break;
                }
                for(int i = 0; i < pContextThreads->appContext->numberActiveUsers; i++) {
                    if(strcmp(pContextThreads->appContext->activeUsers[i].name, userToRemove) == 0) {
                        pthread_mutex_lock(pContextThreads->pLatch); // MUTEX
                        removeActiveUser(pContextThreads->appContext, userToRemove, &removeSucess);
                        pthread_mutex_unlock(pContextThreads->pLatch); // MUTEX
                        pidToRemove = pContextThreads->appContext->activeUsers[i].pid;
                        break;
                    }
                }
                if(removeSucess == 1) {
                    printf("[MANAGER] Removeste o utilizador '%s' com sucesso.\n", userToRemove);

                    sprintf(fifo_cli, FIFO_CLI, pidToRemove);
                    fd_cli = open(fifo_cli, O_WRONLY);
                    if (fd_cli == -1) {
                        printf("[MANAGER] ERRO ao abrir o FIFO do cliente\n");
                        break;
                    }

                    int cmdExit = CMD_EXIT;
                    nBytesSentExit = write(fd_cli, &cmdExit, sizeof(cmdExit));
                    if(nBytesSentExit == sizeof(cmdExit)) {
                        printf("[MANAGER] Reenviou o sucesso do remove para '%s'.\n", userToRemove);
                    }
                    close(fd_cli);
                } else{
                    printf("[MANAGER] Não existe ninguém com nome '%s' na lista de utilizadores ativos.\n", userToRemove);
                }
                
                printf("\n");
                break;

            case CMD_TOPICS_SERV:
                if(pContextThreads->appContext->numberActiveTopics == 0) {
                    printf("[MANAGER] Não existe nenhum tópico ativo de momento.\n");
                    printf("\n");
                    break;
                }

                printf("[MANAGER] Lista de tópicos ativos:\n");
                pthread_mutex_lock(pContextThreads->pLatch); // MUTEX
                for(int i = 0; i < pContextThreads->appContext->numberActiveTopics; i++) {
                    if(pContextThreads->appContext->topicsCreated[i].topicLocked == 1) {
                        printf("\t - Tópico[%d]: %s (Bloqueado)\n", i + 1, pContextThreads->appContext->topicsCreated[i].nameTopic);
                    } else if (pContextThreads->appContext->topicsCreated[i].topicLocked == 0){
                        printf("\t - Tópico[%d]: %s\n", i + 1, pContextThreads->appContext->topicsCreated[i].nameTopic);
                    }
                }
                pthread_mutex_unlock(pContextThreads->pLatch); // MUTEX

                printf("\n");
                break;
            
            case CMD_SHOW:
                flagExistsTopic = 0;
                commandToText(str, &i, topic);
                if(pContextThreads->appContext->numberActiveTopics == 0) {
                    printf("[MANAGER] Não existe nenhum tópico criado, logo não podes dar show a nenhum tópico.\n");
                    printf("\n");
                    break;
                }
                
                pthread_mutex_lock(pContextThreads->pLatch);
                for (int i = 0; i < pContextThreads->appContext->numberActiveTopics; i++) {
                    if (strcmp(pContextThreads->appContext->topicsCreated[i].nameTopic, topic) == 0) {
                        flagExistsTopic = 1;
                        if(pContextThreads->appContext->topicsCreated[i].numberPersistantMessages > 0) {
                            printf("[MANAGER] Lista de mensagens persistentes do tópico '%s':\n", topic);
                            flagExistsTopic = 2;
                            PERSISTANT_MESSAGES *messages = pContextThreads->appContext->topicsCreated[i].persistantMessages;

                            for (int j = 0; j < pContextThreads->appContext->topicsCreated[i].numberPersistantMessages; j++) {
                                printf("\t - %s %s %d %s\n", 
                                    pContextThreads->appContext->topicsCreated[i].nameTopic, 
                                    messages[j].persistantUser,
                                    messages[j].persistantTime,
                                    messages[j].persistantMessage);
                            }
                        }
                    }
                }
                pthread_mutex_unlock(pContextThreads->pLatch);

                if(flagExistsTopic == 0) {
                    printf("[MANAGER] O tópico '%s' não existe.\n", topic);
                } else if(flagExistsTopic == 1) {
                    printf("[MANAGER] O tópico '%s' não tem mensagens persistentes, logo não consegue dar show.\n", topic);
                }

                printf("\n");
                break;

            case CMD_LOCK:
                lockSuccess = 0;
                commandToText(str, &i, topic);
                if(pContextThreads->appContext->numberActiveTopics == 0) {
                    printf("[MANAGER] Não existe nenhum tópico criado.\n");
                    break;
                }
                pthread_mutex_lock(pContextThreads->pLatch); // MUTEX
                lockTopic(pContextThreads->appContext, topic, &lockSuccess);
                pthread_mutex_unlock(pContextThreads->pLatch); // MUTEX
                if(lockSuccess == 1) {
                    printf("[MANAGER] Bloqueaste o tópico '%s'.\n", topic);
                } else if (lockSuccess == 0){
                    printf("[MANAGER] O tópico '%s' não existe, logo não consegues bloqueá-lo.\n", topic);
                }
                
                printf("\n");
                break;
            case CMD_UNLOCK:
                unlockSuccess = 0;
                commandToText(str, &i, topic);
                if(pContextThreads->appContext->numberActiveTopics == 0) {
                    printf("[MANAGER] Não existe nenhum tópico criado.\n");
                    break;
                }
                pthread_mutex_lock(pContextThreads->pLatch); // MUTEX
                unlockTopic(pContextThreads->appContext, topic, &unlockSuccess);
                pthread_mutex_unlock(pContextThreads->pLatch); // MUTEX
                if(unlockSuccess == 1) {
                    printf("[MANAGER] Desbloqueaste o tópico '%s'.\n", topic);
                } else if (lockSuccess == 0){
                    printf("[MANAGER] O tópico '%s' não existe, logo não consegues desbloqueá-lo.\n", topic);
                }
                
                printf("\n");
                break;
            
            case CMD_CLOSE:
                if(pContextThreads->appContext->numberActiveUsers == 0) {
                    pContextThreads->timerContinue = 0;
                    pthread_kill(pContextThreads->mainThreadId, SIGUSR1);
                    pid_t lwp = syscall(SYS_gettid);
                    printf("[MANAGER] Thread '%d' dos comando admin a terminar.\n", lwp);
                    pthread_exit(NULL); 
                    printf("\n");
                    break;
                }

                for(int i = 0; i < pContextThreads->appContext->numberActiveUsers; i++) {
                    sprintf(fifo_cli, FIFO_CLI, pContextThreads->appContext->activeUsers[i].pid);
                    fd_cli = open(fifo_cli, O_WRONLY);
                    if (fd_cli == -1) {
                        printf("[MANAGER] ERRO ao abrir o FIFO do cliente\n");
                        break;
                    }

                    int cmdExit = CMD_EXIT;
                    nBytesSentExit = write(fd_cli, &cmdExit, sizeof(cmdExit));
                    if(nBytesSentExit == sizeof(cmdExit)) {
                        printf("[MANAGER] Reenviou o close para o utilizador '%s'.\n", pContextThreads->appContext->activeUsers[i].name);
                    }
                    close(fd_cli);
                }
                pContextThreads->timerContinue = 0;
                pthread_kill(pContextThreads->mainThreadId, SIGUSR1);
                pid_t lwp = syscall(SYS_gettid);
                printf("[MANAGER] Thread '%d' dos comando admin a terminar.\n", lwp);
                pthread_exit(NULL); 
                printf("\n");
                break;

            case -1:
                printf("[MANAGER] Não utilizaste um comando permitido. Usa um destes: \n\t - users \n\t - remove <username>\n\t - topics\n\t - show <Tópico>\n\t - exit\n\t - lock <Tópico>\n\t - unlock <Tópico>\n\t - close\n");
                
                printf("\n");
                break;
        }
    }
}

void *timerForPersistantMessages(void *pContext_threads) {
    CONTEXT_THREADS *pContextThreads = (CONTEXT_THREADS *) pContext_threads;
    int numberPersistantMessages = -1;

    while(pContextThreads->timerContinue == 1) {
        for (int i = 0; i < pContextThreads->appContext->numberActiveTopics; i++) {
            pthread_mutex_lock(pContextThreads->pLatch);
            numberPersistantMessages = pContextThreads->appContext->topicsCreated[i].numberPersistantMessages;
            for(int j = 0; j < numberPersistantMessages; j++) {
                (pContextThreads->appContext->topicsCreated[i].persistantMessages[j].persistantTime)--;

                if(pContextThreads->appContext->topicsCreated[i].persistantMessages[j].persistantTime <= 0) {
                    pContextThreads->appContext->topicsCreated[i].persistantMessages[j].persistantUser[0] = '\0';
                    pContextThreads->appContext->topicsCreated[i].persistantMessages[j].persistantMessage[0] = '\0';

                    if(j != (numberPersistantMessages - 1)) {
                        strcpy(
                            pContextThreads->appContext->topicsCreated[i].persistantMessages[j].persistantMessage, 
                            pContextThreads->appContext->topicsCreated[i].persistantMessages[(pContextThreads->appContext->topicsCreated[i].numberPersistantMessages) - 1].persistantMessage
                        );
                        strcpy(
                            pContextThreads->appContext->topicsCreated[i].persistantMessages[j].persistantMessage, 
                            pContextThreads->appContext->topicsCreated[i].persistantMessages[(pContextThreads->appContext->topicsCreated[i].numberPersistantMessages) - 1].persistantMessage
                        );

                        pContextThreads->appContext->topicsCreated[i].persistantMessages[j].persistantTime = 
                        pContextThreads->appContext->topicsCreated[i].persistantMessages[(pContextThreads->appContext->topicsCreated[i].numberPersistantMessages) - 1].persistantTime;
                    }
                    
                    (pContextThreads->appContext->topicsCreated[i].numberPersistantMessages)--;
                }
                
                removeTopic(pContextThreads->appContext, pContextThreads->appContext->topicsCreated[i].nameTopic, -1);
            }
            pthread_mutex_unlock(pContextThreads->pLatch);                               
        }
        sleep(1);
    }
    pid_t lwp = syscall(SYS_gettid);
    printf("[MANAGER] Thread '%d' do timer a terminar.\n", lwp);

    pthread_exit(NULL);
}

int main() {
    setbuf(stdout, NULL);
    int fd_serv, fd_cli;
    int nBytesCmd, nBytesReceive, nBytesSent;
    char fifo_cli[20];
    FILE *file;

    CONTEXT app_context = {0};

    // Leitura
    HEADER header;
    REGISTER_REQUEST register_request;
    MESSAGE message;
    SUBSCRIBE subscribe;
    UNSUBSCRIBE unsubscribe;
    TOPICS topics;

    // Reenvio
    MESSAGE_SRV message_srv;
    SUBSCRIBE_SRV subscribe_srv;
    UNSUBSCRIBE_SRV unsubscribe_srv;
    TOPICS_SRV topics_srv;
    int RegisterSuccess, exitSuccess;
    
    printf("[MANAGER] INICIO...\n");
    if (access(FIFO_SRV, F_OK ) == 0) {
        printf("[MANAGER] ERRO - Ja existe um servidor a correr!\n");
        pthread_exit(NULL);
    }

    // Sinal SIGUSR1 para comunicar entre threads
    struct sigaction action;
    action.sa_flags = 0;
    action.sa_handler = leave;
    sigaction(SIGUSR1, &action, NULL);


    // Tratamento das threads
    pthread_t threadId[2];
    pthread_mutex_t latch; 
    pthread_mutex_init(&latch, NULL);
    CONTEXT_THREADS context_threads;
    context_threads.appContext = &app_context;
    context_threads.mainThreadId = pthread_self();
    context_threads.timerContinue = 1;
    context_threads.pLatch = &latch;
    pthread_create(&threadId[0], NULL, managerAdminCommands, (void *) &context_threads); 
    pthread_create(&threadId[1], NULL, timerForPersistantMessages, (void *) &context_threads);

    // Abertura e leitura de mensagens persistentes do ficheiro
    char *filename = getenv("MSG_FICH");
    if (filename == NULL) {
        fprintf(stderr, "Erro: Variável de ambiente MSG_FICH não definida.\n");
        exit(EXIT_FAILURE);
    }

    file = fopen(filename, "r");
    if (file == NULL) {
        perror("Erro ao abrir o ficheiro");
        exit(EXIT_FAILURE);
    }
    readDataFromFile(&app_context, file);
    fclose(file);

    // Criação do fifo do server
    mkfifo(FIFO_SRV, 0600);
    fd_serv = open(FIFO_SRV, O_RDWR);

    // Leitura e reenvio para o feed
    do {
        nBytesCmd = read(fd_serv, &header, sizeof(header));
        if (nBytesCmd != sizeof(header)) { // não leu corretamente
            printf("[MANAGER] Não leu corretamente.\n");
            break;
        }

        switch (header.cmd) {
        case CMD_REGISTER:
            nBytesReceive = read(fd_serv, &register_request, sizeof(register_request));
            if (nBytesReceive != sizeof(register_request)) {
                printf("[MANAGER] Não leu corretamente o registo.\n");
                break;
            }

            if (app_context.numberActiveUsers == NUM_USERS) {
                RegisterSuccess = 2;
                printf("[MANAGER] ERRO - Não foi possível registar o User '%s': número máximo de utilizadores atingido.\n", register_request.name);
            } else if (isNameAvailable(app_context, register_request.name) == 1) {
                RegisterSuccess = 1;
                pthread_mutex_lock(&latch); // MUTEX
                registerUser(&app_context, register_request, header);
                pthread_mutex_unlock(&latch); // MUTEX
                printf("[MANAGER] Registo do User '%s' feito.\n",register_request.name);
            } else {
                RegisterSuccess = 0;
                printf("[MANAGER] Registo do User '%s' não feito, já existe alguém com esse nome.\n", register_request.name);
            }

            sprintf(fifo_cli, FIFO_CLI, header.pid);
            fd_cli = open(fifo_cli, O_WRONLY);
            if (fd_cli == -1) {
                printf("[MANAGER] ERRO ao abrir o FIFO do cliente\n");
                break;
            }

    
            nBytesSent = write(fd_cli, &RegisterSuccess, sizeof(RegisterSuccess));
            if (nBytesSent == sizeof(RegisterSuccess)) {
                printf("[MANAGER] Enviou o sucesso do registo.\n");
            }
            close(fd_cli);
            break;
        
        case CMD_MESSAGE:
            nBytesReceive = read(fd_serv, &message, sizeof(message));
            if(nBytesReceive != sizeof(message)){
                printf("Não leu corretamente a message.\n");
                break;
            }


            if(isTopicAvailable(app_context, message.topic) == 0 && isUserSubscribedTopic(app_context, message.topic, header.pid) == 0){
                message.msgSuccess = 0;
                printf("[MANAGER] O Tópico '%s' já foi criado. Tente outro.\n", message.topic);
            }else if(isTopicAvailable(app_context, message.topic) == 1 && isUserSubscribedTopic(app_context, message.topic, header.pid) == 0) {
                pthread_mutex_lock(&latch); // MUTEX
                if (createTopicWithUser(&app_context, message.topic, header) == 1) {
                    message.msgSuccess = 1;
                    printf("[MANAGER] Tópico '%s' criado pelo utilizador '%s'.\n", message.topic, header.user);
                    if(message.timePersistent == 0) {
                        printf("[MANAGER] Recebi mensagem '%s' no tópico '%s' de %s.\n", message.text, message.topic, header.user);
                    } else {
                        printf("[MANAGER] Recebi mensagem persistente '%s' de '%d' segundos no tópico '%s' de %s.\n", message.text, message.timePersistent, message.topic, header.user);
                    }
                } else {
                    message.msgSuccess = 3;
                }
                pthread_mutex_unlock(&latch); // MUTEX
            }else if(isUserSubscribedTopic(app_context, message.topic, header.pid) == 1 && isTopicLocked(app_context, message.topic) == 0){
                message.msgSuccess = 1;
                if(message.timePersistent == 0) {
                    printf("[MANAGER] Recebi mensagem '%s' no tópico '%s' de %s.\n", message.text, message.topic, header.user);
                } else {
                    printf("[MANAGER] Recebi mensagem persistente '%s' de '%d' segundos no tópico '%s' de %s.\n", message.text, message.timePersistent, message.topic, header.user);
                }
            }else if(isUserSubscribedTopic(app_context, message.topic, header.pid) == 1 && isTopicLocked(app_context, message.topic) == 1){
                message.msgSuccess = 2;
                if(message.timePersistent == 0) {
                    printf("[MANAGER] Recebi mensagem '%s' no tópico '%s' de %s, mas o Tópico encontra-se bloqueado, e portanto, a mensagem não foi aceite.\n", message.text, message.topic, header.user);
                } else {
                    printf("[MANAGER] Recebi mensagem persistente '%s' de '%d' segundos no tópico '%s' de %s, mas o Tópico encontra-se bloqueado, e portanto, a mensagem não foi aceite.\n", message.text, message.timePersistent, message.topic, header.user);
                }
            }

            if(message.timePersistent > 0 && message.msgSuccess == 1) {
                writeMessagePersistant(&app_context, message);
            }


            message_srv.message = message;
            message_srv.cmd = header.cmd;
            if(message.msgSuccess == 1) {
                for(int i = 0; i < app_context.numberActiveUsers; i++) {
                    if(isUserSubscribedTopic(app_context, message.topic, app_context.activeUsers[i].pid) == 1) {
                        sprintf(fifo_cli, FIFO_CLI, app_context.activeUsers[i].pid);
                        fd_cli = open(fifo_cli, O_WRONLY);
                        if (fd_cli == -1) {
                            printf("[MANAGER] ERRO ao abrir o FIFO do cliente\n");
                            break;
                        }

                        nBytesSent = write(fd_cli, &message_srv, sizeof(message_srv));
                        if(nBytesSent == sizeof(message_srv)) {
                            printf("[MANAGER] Reenviou a mensagem para '%s'.\n", app_context.activeUsers[i].name);
                        }
                        close(fd_cli);
                    }
                }
                break;
            }

            sprintf(fifo_cli, FIFO_CLI, header.pid);
            fd_cli = open(fifo_cli, O_WRONLY);
            if (fd_cli == -1) {
                printf("[MANAGER] ERRO ao abrir o FIFO do cliente\n");
                break;
            }

            nBytesSent = write(fd_cli, &message_srv, sizeof(message_srv));
            if(nBytesSent == sizeof(message_srv)) {
                printf("[MANAGER] Enviou o sucesso da mensagem.\n");
            }
            close(fd_cli);
            break;

        case CMD_SUBSCRIBE:
            nBytesReceive = read(fd_serv, &subscribe, sizeof(subscribe));
            if(nBytesReceive != sizeof(subscribe)) {
                printf("Não leu corretamente o subscribe.\n");
                break;
            }
              
            if(isTopicAvailable(app_context, subscribe.topic) == 1){
                pthread_mutex_lock(&latch); // MUTEX
                if (createTopicWithUser(&app_context, subscribe.topic, header) == 1) {
                    subscribe.subsSuccess = 0;
                    printf("[MANAGER] Tópico '%s' criado pelo utilizador '%s'.\n", subscribe.topic, header.user);
                } else {
                    subscribe.subsSuccess = 1;
                    printf("[MANAGER] ERRO na criação do Tópico.\n");
                }
                pthread_mutex_unlock(&latch); // MUTEX
            } else if(isUserSubscribedTopic(app_context, subscribe.topic, header.pid) == 1) {
                subscribe.subsSuccess = 2;
                printf ("[MANAGER] O user '%s' tentou subscrever um Tópico '%s' ao qual já está subscrito.\n", header.user, subscribe.topic);
            } else if(isTopicAvailable(app_context, subscribe.topic) == 0 && isUserSubscribedTopic(app_context, subscribe.topic, header.pid) == 0){
                subscribe.subsSuccess = 3;
                userSubscribeTopic(&app_context, subscribe.topic, header);
                printf ("[MANAGER] O user '%s' subscreveu o Tópico '%s', já existente.\n", header.user, subscribe.topic);
                for(int i = 0; i < app_context.numberActiveTopics; i++) {
                    if(strcmp(subscribe.topic, app_context.topicsCreated[i].nameTopic) == 0) {
                        if(app_context.topicsCreated[i].numberPersistantMessages <= 0) {
                            printf("[MANAGER] Não existem mensagens persistentes no tópico '%s' para reenviar.\n", subscribe.topic);
                            break;
                        }
                        subscribe_srv.numberPersistantMessages = app_context.topicsCreated[i].numberPersistantMessages;
                        for(int j = 0; j < app_context.topicsCreated[i].numberPersistantMessages; j++) {
                            subscribe_srv.persistantMessages[j] = app_context.topicsCreated[i].persistantMessages[j];
                        }
                    }
                }
            }

            subscribe_srv.subscribe = subscribe;
            subscribe_srv.cmd = header.cmd;

            sprintf(fifo_cli, FIFO_CLI, header.pid);
            fd_cli = open(fifo_cli, O_WRONLY);
            if (fd_cli == -1) {
                printf("[MANAGER] ERRO ao abrir o FIFO do cliente\n");
                break;
            }

        
            nBytesSent= write(fd_cli, &subscribe_srv, sizeof(subscribe_srv));
            if(nBytesSent == sizeof(subscribe_srv)){
                printf("[MANAGER] Reenviou o subscribe.\n");
            }

            close(fd_cli);
            break;

        case CMD_UNSUBSCRIBE:
            nBytesReceive= read(fd_serv, &unsubscribe, sizeof(unsubscribe));
            if(nBytesReceive != sizeof(unsubscribe)) {
                printf("Não leu corretamente o unsubscribe.\n");
                break;
            }

            if(isTopicAvailable(app_context, unsubscribe.topic) == 1) {
                printf("[MANAGER] O topico '%s' não existe.\n", unsubscribe.topic);
                unsubscribe.unSubsSuccess = 0;
            } else if (isTopicAvailable(app_context, unsubscribe.topic) == 0 && isUserSubscribedTopic(app_context, unsubscribe.topic, header.pid) == 0) {
                printf("[MANAGER] O user '%s' não está subscrito no topico '%s', logo não pode dar unsubscribe.\n", header.user, unsubscribe.topic);
                unsubscribe.unSubsSuccess = 1;
            } else if(isTopicAvailable(app_context, unsubscribe.topic) == 0 && isUserSubscribedTopic(app_context, unsubscribe.topic, header.pid) == 1){
                pthread_mutex_lock(&latch); // MUTEX
                userUnsubscribeTopic(&app_context, unsubscribe.topic, header);
                pthread_mutex_unlock(&latch); // MUTEX
                unsubscribe.unSubsSuccess = 2;
                printf("[MANAGER] O user '%s' deu unsubscribe no topico '%s'.\n", header.user, unsubscribe.topic);
            }


            sprintf(fifo_cli, FIFO_CLI, header.pid);
            fd_cli = open(fifo_cli, O_WRONLY);
            if (fd_cli == -1) {
                printf("[MANAGER] ERRO ao abrir o FIFO do cliente\n");
                break;
            }

            unsubscribe_srv.unsubscribe = unsubscribe;
            unsubscribe_srv.cmd = header.cmd;
            nBytesSent = write(fd_cli, &unsubscribe_srv, sizeof(unsubscribe_srv));
            if(nBytesSent == sizeof(unsubscribe_srv)){
                printf("[MANAGER] Reenviou o subscribe.\n");
            }

            close(fd_cli);
            break;

        case CMD_TOPICS:
            nBytesReceive = read(fd_serv, &topics, sizeof(topics));
            if(nBytesReceive != sizeof(topics)) {
                printf("[MANAGER] Não leu corretamente o unsubscribe.\n");
                break;
            }
            pthread_mutex_lock(&latch); // MUTEX
            copyTopicsListToTopicsCli(&topics, app_context);
            pthread_mutex_unlock(&latch); // MUTEX

            sprintf(fifo_cli, FIFO_CLI, header.pid);
            fd_cli = open(fifo_cli, O_WRONLY);
            if (fd_cli == -1) {
                printf("[MANAGER] ERRO ao abrir o FIFO do cliente\n");
                break;
            }

            topics_srv.topics = topics;
            topics_srv.cmd = header.cmd;
            nBytesSent = write(fd_cli, &topics_srv, sizeof(topics_srv));
            if(nBytesSent == sizeof(topics_srv)){
                printf("[MANAGER] Reenviou a lista dos topicos.\n");
            }
            
            close(fd_cli);
            break;

        case CMD_EXIT:
            nBytesReceive = read(fd_serv, &exitSuccess, sizeof(exitSuccess));
            if(nBytesReceive != sizeof(exitSuccess)) {
                printf("Não leu corretamente o exit.\n");
                break;
            }
            pthread_mutex_lock(&latch); // MUTEX
            removeActiveUser(&app_context, header.user, &exitSuccess);
            pthread_mutex_unlock(&latch); // MUTEX
            printf("[MANAGER] Utilizador '%s' removido da lista dos utilizadores ativos.\n", header.user);

            sprintf(fifo_cli, FIFO_CLI, header.pid);
            fd_cli = open(fifo_cli, O_WRONLY);
            if (fd_cli == -1) {
                printf("[MANAGER] ERRO ao abrir o FIFO do cliente\n");
                break;
            }

            nBytesSent = write(fd_cli, &header.cmd, sizeof(header.cmd));
            if(nBytesSent == sizeof(header.cmd)) {
                printf("[MANAGER] Reenviou o sucesso do exit.\n");
            }
            
            close(fd_cli);
            break;

        default:
            printf("[MANAGER] Ups! O comando cmd NÃO foi enviado corretamente.\n");

            break;
        }
        printf("\n");
    } while(1);
    close(fd_serv);
    unlink(FIFO_SRV);

    pthread_join(threadId[0], NULL);
    pthread_join(threadId[1], NULL);

    // Abertura e escrita de mensagens persistentes do ficheiro
    file = fopen(filename, "w");
    if (file == NULL) {
        perror("Erro ao abrir o ficheiro");
        exit(EXIT_FAILURE);
    }
    writeDataInFile(&app_context, file);
    fclose(file);

    pthread_mutex_destroy(&latch);

    pid_t lwp = syscall(SYS_gettid);
    printf("[MANAGER] Thread '%d' do main a terminar.\n", lwp);

    printf("[MANAGER] FIM!\n");
    pthread_exit(NULL);
}