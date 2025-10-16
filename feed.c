#include "util.h"

int commandCharToInt(char *commandChar) {
    if(strcmp(commandChar, "msg") == 0) {
        return CMD_MESSAGE;
    }
    if(strcmp(commandChar, "subscribe") == 0) {
        return CMD_SUBSCRIBE;
    }
    if(strcmp(commandChar, "unsubscribe") == 0) {
        return CMD_UNSUBSCRIBE;
    }
    if(strcmp(commandChar, "topics") == 0) {
        return CMD_TOPICS;
    }
    if(strcmp(commandChar, "exit") == 0) {
        return CMD_EXIT;
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

void commandToMessageText(char* str, int* j, char* msgTopic) {
    int k = 0;
    (*j)++; // salta o espaço msg(este espaço)textodamensagem   
    for ( ; str[*j] != '\0'; (*j)++) {
        msgTopic[k] = str[*j];
        k++;
    }
    msgTopic[k] = '\0';
}

void leave(int signal) { // Função de tratamento de sinal

}

void *receiveCommandsData(void *pReceiveCommandsCli) {
    RECEIVE_COMMANDS_CLI *pReceiveCommands = (RECEIVE_COMMANDS_CLI *) pReceiveCommandsCli; 

    MESSAGE message;
    SUBSCRIBE subscribe;
    UNSUBSCRIBE unsubscribe;
    TOPICS topics;
    PERSISTANT_MESSAGES persistant_messages[NUM_PERSISTANT_MESSAGES];

    int nBytesReceive, nBytesCmd;
    int cmd, numberPersistantMessages;

    while(1){ 
        nBytesCmd = read(pReceiveCommands->fdReceiveCommand, &cmd, sizeof(int));
        if (nBytesCmd != sizeof(int)) { // não leu corretamente
            printf("[MANAGER] Não leu corretamente.\n");
            continue;
        }
        switch (cmd) {
            case CMD_MESSAGE:
                nBytesReceive = read(pReceiveCommands->fdReceiveCommand, &message, sizeof(message));
                if(nBytesReceive == sizeof(message)){
                    if(message.msgSuccess == 1) {
                        printf("[FEED] %s: \nTópico: %s   \nMensagem: %s\n", message.user, message.topic, message.text);
                    } else if(message.msgSuccess == 0) {
                        printf("[FEED] Não estás subscrito no tópico '%s', subscreve/cria para puderes mandar mensagem.\n", message.topic);
                    } else if(message.msgSuccess == 2) {
                        printf("[FEED] Não consegues mandar mensagem para o tópico '%s', pois está bloqueado.\n", message.topic);
                    } else if(message.msgSuccess == 3) {
                        printf("[FEED] Não consegues mandar mensagem e criar o tópico '%s', pois o servidor já alcançou o número máximo de tópicos que podem ser criados.\n", message.topic);
                    } 
                }

                printf("\n");
                break;

            case CMD_SUBSCRIBE:               
                nBytesReceive = read(pReceiveCommands->fdReceiveCommand , &subscribe, sizeof(subscribe));
                if(nBytesReceive == sizeof(subscribe)){
                    if(subscribe.subsSuccess == 0) {
                        printf("[FEED] Como não existia nenhum tópico '%s', criaste-o.\n", subscribe.topic);
                    }else if(subscribe.subsSuccess == 1) {
                        printf("[FEED] O servidor já alcançou o número máximo de tópicos criado, como o tópico não existe, logo não podes criar o tópico '%s'.\n", subscribe.topic);
                    }else if(subscribe.subsSuccess == 2) {
                        printf("[FEED] Já te encontras subscrito no tópico '%s'.\n", subscribe.topic);
                    }else if(subscribe.subsSuccess == 3) {
                        printf("[FEED] O subscrição do tópico '%s' foi feita com sucesso.\n", subscribe.topic);
                    }
                }

                nBytesReceive = read(pReceiveCommands->fdReceiveCommand , &numberPersistantMessages, sizeof(numberPersistantMessages));
                if(nBytesReceive == sizeof(numberPersistantMessages)){
                    printf("[FEED] O tópico '%s' que subscreveste tem %d mensagens persistentes.\n", subscribe.topic, numberPersistantMessages);
                }

                for(int i = 0; i < NUM_PERSISTANT_MESSAGES; i++) {
                    nBytesReceive = read(pReceiveCommands->fdReceiveCommand , &persistant_messages[i], sizeof(persistant_messages[i]));
                    if(nBytesReceive == sizeof(persistant_messages[i])){
                        //printf("[FEED] Leu a mensagem %d\n", i);
                    } 
                }
                for(int i = 0; i < numberPersistantMessages; i++) {
                    printf("[FEED] %s: \nTópico: %s   \nMensagem: %s\n\n", persistant_messages[i].persistantUser, subscribe.topic, persistant_messages[i].persistantMessage);
                }
                printf("\n");
                break;

            case CMD_UNSUBSCRIBE:
                nBytesReceive = read(pReceiveCommands->fdReceiveCommand, &unsubscribe, sizeof(unsubscribe));
                if(nBytesReceive == sizeof(unsubscribe)){
                    if(unsubscribe.unSubsSuccess == 0) {
                        printf("[FEED] O tópico '%s' não existe, logo não podes dar unsubscribe.\n", unsubscribe.topic);
                    }else if(unsubscribe.unSubsSuccess == 1) {
                        printf("[FEED] Não te encontravas subscrito neste tópico, logo não podes dar unsubscribe neste tópico '%s'.\n", unsubscribe.topic);
                    }else if(unsubscribe.unSubsSuccess == 2) {
                        printf("[FEED] A tua unsubscribe do tópico '%s' foi feita com sucesso.\n", unsubscribe.topic);
                    }  
                }

                printf("\n");
                break;
            
            case CMD_TOPICS:
                nBytesReceive= read(pReceiveCommands->fdReceiveCommand, &topics, sizeof(topics));
                if(nBytesReceive == sizeof(topics)){
                    if(topics.topicsSuccess == 0){
                        printf("[FEED] Não existe nenhum tópico criado.\n");                  
                    }else if(topics.topicsSuccess == 1){
                        printf("[FEED] Aqui tem a sua lista de tópicos:\n");
                        for(int i = 0; i < topics.numberTopics; i++) {
                            if(topics.listTopics[i].topicLocked == 1) {
                                printf("\t- Tópico[%d]: %s (mensagens persistentes: %d) (BLOQUEADO)\n", i + 1, topics.listTopics[i].nameTopic, topics.listTopics[i].numberPersistantMessages);
                            }else if(topics.listTopics[i].topicLocked == 0){
                                printf("\t- Tópico[%d]: %s (mensagens persistentes: %d)\n", i + 1, topics.listTopics[i].nameTopic, topics.listTopics[i].numberPersistantMessages);
                            }
                        }
                    }
                }

                printf("\n");
                break;

            case CMD_EXIT:
                pthread_kill(pReceiveCommands->tid, SIGUSR1);
                pid_t lwp = syscall(SYS_gettid);
                printf("[FEED] Thread '%d' que recebe os comandos a terminar.\n", lwp);
                pthread_exit(NULL); 
                break;

        }
    }
}

int main(int argc, char *argv[]) {
    setbuf(stdout, NULL);
    char fifo_cli[SIZE];
    char str[340];
    int nBytesReceive, nBytesSent; 
    int subscribeSuccess, exitClient = 0;
    int fd_receive_command, fd_send_command;
    char commandChar[15], messageText[MESSAGE_SIZE], topic[SIZE_TOPIC], messagePersistantTime[10];
    int command, i, j;

    REGISTER_REQUEST_CLI register_request_cli;
    MESSAGE_CLI message_cli;
    SUBSCRIBE_CLI subscribe_cli;
    UNSUBSCRIBE_CLI unsubscribe_cli;
    TOPICS_CLI topics_cli;
    EXIT_CLI exit_cli;

    printf("[FEED] INICIO...\n");
    if(argc != 2) {
        printf("[FEED] ERRO - corre o feed de forma correta: './feed (NomeDoUtilizador)'.\n");
    }
    if(access(FIFO_SRV, F_OK ) != 0) {
        printf("[FEED] ERRO - O servidor nao esta a correr.\n");
        exit(3);
    }

    // Criação do fifo e conexão ao server
    sprintf(fifo_cli, FIFO_CLI, getpid());
    mkfifo(fifo_cli, 0600);

    fd_receive_command = open(fifo_cli, O_RDWR);
    fd_send_command = open(FIFO_SRV, O_WRONLY);

    // Registo
    strcpy(register_request_cli.register_request.name, argv[1]);
    register_request_cli.header.pid = getpid();
    register_request_cli.header.cmd = CMD_REGISTER; 
    
    nBytesSent = write(fd_send_command, &register_request_cli, sizeof(register_request_cli));
    if(nBytesSent == sizeof(register_request_cli)) {
        printf("[FEED] Enviou o registo.\n");
    }

    nBytesReceive = read(fd_receive_command, &subscribeSuccess, sizeof(subscribeSuccess));
    if(nBytesReceive == sizeof(subscribeSuccess)) {
        if(subscribeSuccess == 1) {
            printf("[FEED] Registo feito com sucesso.\n\n");
        } else if(subscribeSuccess == 0) {
            printf("[FEED] Registo NÃO feito com sucesso.\n");
            printf("[FEED] FIM!\n");
            pthread_exit(NULL);
        } else if(subscribeSuccess == 2) {
            printf("[FEED] Registo NÃO feito com sucesso, pois o servidor já atingiu o número máximo de utilizadores ativos permitidos.\n");
            printf("[FEED] FIM!\n");
            pthread_exit(NULL);
        }
    }

    // Sinal SIGUSR1 para comunicar entre threads
    struct sigaction action;
    action.sa_flags = 0;
    action.sa_handler = leave;
    sigaction(SIGUSR1, &action, NULL);
        
    // Tratamento das threads
    pthread_t threadId;
    RECEIVE_COMMANDS_CLI receive_commands_cli;
    receive_commands_cli.fdReceiveCommand = fd_receive_command;
    receive_commands_cli.tid = pthread_self();
    pthread_create(&threadId, NULL, receiveCommandsData, (void *) &receive_commands_cli); //mostra(4, 'A');;

    // Comandos
    do{
        if(scanf(" %[^\n]", str) != 1) {
            break; // Sinal recebido da thread
        }
        
        for(i = 0; str[i] != ' ' && str[i] != '\0'; i++);
        for(j = 0; j < i; j++) {
            commandChar[j] = str[j];
        }
        commandChar[j] = '\0';
        command = commandCharToInt(commandChar);
        switch (command) {
            case CMD_MESSAGE: //msg 
                message_cli.header.cmd = CMD_MESSAGE;
                strcpy(message_cli.header.user, argv[1]);
                message_cli.header.pid = getpid();

                commandToText(str, &i, topic);
                strcpy(message_cli.message.topic, topic);
                commandToText(str, &i, messagePersistantTime);
                message_cli.message.timePersistent = atoi(messagePersistantTime);
                commandToMessageText(str, &i, messageText);
                strcpy(message_cli.message.text, messageText);
                message_cli.message.msgSuccess = 0;
                strcpy(message_cli.message.user, argv[1]);
                
                nBytesSent = write(fd_send_command, &message_cli, sizeof(message_cli));
                if(nBytesSent != sizeof(message_cli)) {
                    continue;
                }
        

                break;

            case CMD_SUBSCRIBE: //subscribe
                subscribe_cli.header.cmd = CMD_SUBSCRIBE;
                strcpy(subscribe_cli.header.user, argv[1]);
                subscribe_cli.header.pid = getpid();

                commandToText(str, &i, topic);
                strcpy(subscribe_cli.subscribe.topic, topic);
                subscribe_cli.subscribe.subsSuccess = 0;

                nBytesSent = write(fd_send_command, &subscribe_cli, sizeof(subscribe_cli));
                if(nBytesSent != sizeof(subscribe_cli)) {
                    continue;
                }
               

                break;

            case CMD_UNSUBSCRIBE: //unsubscribe
                unsubscribe_cli.header.cmd = CMD_UNSUBSCRIBE;
                strcpy(unsubscribe_cli.header.user, argv[1]);
                unsubscribe_cli.header.pid = getpid();

                commandToText(str, &i, topic);
                strcpy(unsubscribe_cli.unsubscribe.topic, topic);
                unsubscribe_cli.unsubscribe.unSubsSuccess = 0;
                
                nBytesSent = write(fd_send_command, &unsubscribe_cli, sizeof(unsubscribe_cli));
                if(nBytesSent != sizeof(unsubscribe_cli)) {
                    continue;
                }
               
                
                break;
            
            case CMD_TOPICS: //listar os topicos (topics)
                topics_cli.header.cmd = CMD_TOPICS;
                strcpy(topics_cli.header.user, argv[1]);
                topics_cli.header.pid = getpid();

                topics_cli.topics.topicsSuccess = 0;
                
                nBytesSent = write(fd_send_command, &topics_cli, sizeof(topics_cli));
                if(nBytesSent != sizeof(topics_cli)){
                    continue;
                }
        

                break;

            case CMD_EXIT:
                exit_cli.header.cmd = CMD_EXIT;
                strcpy(exit_cli.header.user, argv[1]);
                exit_cli.header.pid = getpid();

                nBytesSent = write(fd_send_command, &exit_cli, sizeof(exit_cli));
                if(nBytesSent != sizeof(exit_cli)){
                    continue;
                }
                
                exitClient = 1;
                
                break;

            case -1:
                printf("[FEED] Não utilizaste um comando permitido. Usa um destes: \n\t - topics \n\t - msg <Tópico> <duração> <mensagem>\n\t - subcribe <Tópico>\n\t - unsubcribe <Tópico>\n\t - exit\n");
                
                break;
        } 
        //printf("\n");
    }while(exitClient == 0);
    
    close(fd_receive_command);
    close(fd_send_command);

    pthread_join(threadId, NULL);
    
    unlink(fifo_cli);

    pid_t lwp = syscall(SYS_gettid);
    printf("[FEED] Thread '%d' do main a terminar.\n", lwp);

    printf("[FEED] FIM!\n");
    
    pthread_exit(NULL);
}