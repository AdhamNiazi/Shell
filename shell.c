#include <ctype.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define ARG_COUNT_MAX 16
#define ARG_LENGTH_MAX 32
#define CMD_LINE_MAX 512
#define DELIMITERS " \t\r\n\a"
#define FILE_PATH_MAX 100
#define ZERO_ASCII 48

/* Linked List Data Structure including functions
   Slight modification of Linked List for integers from Stack Overflow:
   https://stackoverflow.com/questions/21662332/using-a-for-loop-to-create-a-linked-list */
typedef struct CommandNode {
        char **args;
        char *fileName;
        char *cmdLine;
        bool isAppended;
        bool isTruncated;
        bool isBackgroundTask;
        bool error;
        int status;
        int pid;
        struct CommandNode *next;
} CmdNode;

typedef struct LinkedList {
        CmdNode *head;
        CmdNode *tail;
        bool hasBgTask;
        int length;
} CmdList;

typedef struct BackgroundNode {
        char *status;
        char *commandLine;
        int pid;
        struct BackgroundNode *next;
        struct BackgroundNode *prev;
} BgNode;

typedef struct BackgroundList {
        BgNode *head;
        BgNode *tail;
        int length;
} BgList;

/* initializes attributes for Command Linked List */
void initList(CmdList *cmdList) {
        cmdList->tail = cmdList->head = NULL;
        cmdList->hasBgTask = false;
        cmdList->length = 0;
}

/* initializes attributes for Background Linked List */
void initBgList(BgList *bgList) {
        bgList->tail = bgList->head = NULL;
        bgList->length = 0;
}

/* appends node to tail of Linked List */
void appendList(CmdList *cmdList, CmdNode *cmdNode) {
        if (cmdList->tail != NULL) {
                cmdList->tail->next = cmdNode;
                cmdList->tail = cmdNode;
        } else {
                cmdList->tail = cmdList->head = cmdNode;
        }

        cmdList->length++;
}

void appendBgList(BgList *bgList, BgNode *task) {
        if (bgList->tail != NULL) {
                task->prev = bgList->tail;
                bgList->tail->next = task;
                bgList->tail = task;
        } else {
                bgList->tail = bgList->head = task;
        }

        bgList->length++;
}

/* frees up allocated memory for Command Linked List */
void freeCmdList(CmdList *cmdList) {
        CmdNode *cmdList = cmdList->head;
        int length = cmdList->length;
        
        for (int i = 0; i < length; i++) {
                list->head = list->head->next;
                free(node);
                list->length--;
                node = list->head;
        }
}

/* prints out Linked List node values */
void printList(List *list) {
    Node *command = list->head;
    printf("Command List: \n");
    while(command != NULL) {
        char ** args = command->args;
        for (int i = 0; i < ARG_COUNT_MAX; i++) {
                if (args[i] != NULL)
                        printf("%s ", args[i]);
                else
                        printf("NULL ");
        }
        command = command->next;
        printf("\n");
    }
}

/* checks if all characters if any are whitespace 
Source: https://stackoverflow.com/questions/3981510/getline-check-if-line-is-whitespace */
bool isEmptyString(char *str) {
        while (*str != '\0') {
                if (!isspace((unsigned char)*str))
                        return false;
                str++;
        }
        return true;
}

/* trims trailing and leading spaces from string 
Source: https://stackoverflow.com/questions/122616/how-do-i-trim-leading-trailing-whitespace-in-a-standard-way */
char *trimString(char *str) {
        char *end;

        // Trim leading space
        while(isspace((unsigned char)*str)) str++;

        if(*str == 0)  // All spaces?
        return str;

        // Trim trailing space
        end = str + strlen(str) - 1;
        while(end > str && isspace((unsigned char)*end)) end--;

        // Write new null terminator character
        end[1] = '\0';

        return str;
}

/* counts number of occurences of character in given string 
Source: https://stackoverflow.com/questions/4235519/counting-number-of-occurrences-of-a-char-in-a-string-in-c */
int charCount(char *str, char c) {
        int count = 0;

        for (int i = 0; str[i]; i++)
                count += (str[i] == c);

        return count;
}

/* prompts user for command and collects command line input */
void getCommandLine(char *commandLine) {
        /* Print prompt */
        printf("sshell@ucd$ ");
        fflush(stdout);
        
        /* Get command line */
        fgets(commandLine, CMD_LINE_MAX, stdin);
}

/* required input processing from skeleton code and check for empty command line */
void processCommandLine(char *commandLine) {
        /* Print command line if stdin is not provided by terminal */
        if (!isatty(STDIN_FILENO)) {
                printf("%s", commandLine);
                fflush(stdout);
        }

        /* Remove trailing newline from command line */
        char *nl = strchr(commandLine, '\n');
        if (nl)
                *nl = '\0';
}

/* if */
char* fileNameCheck(char* line){
        if(line == NULL) {
                return NULL;
        }

        char* noSpaceLine = strdup(line);
        int lineIt = 0;
        int newLineIt = 0;
        bool hitChar, hitSpace;
        
        while(line[lineIt] != '\0') {
                if(!isspace(line[lineIt])) {
                        hitChar = true;

                        if(hitChar && hitSpace) {
                                fprintf(stderr, "Error: Invalid File Name\n");
                                return NULL;
                        }

                        noSpaceLine[newLineIt] = line[lineIt];
                        lineIt++;
                        newLineIt++;
                } else {
                        if(hitChar)
                                hitSpace = true;
                        lineIt++;
                }
        }

        noSpaceLine[newLineIt++] = '\0';
        return noSpaceLine;
}

/* parses individual command into tokens and returns array of command arguments
   Warning: parameter line is modified (send duplicate to preserve original string)
   Source: https://brennan.io/2015/01/16/write-a-shell-in-c/ */
char** tokenizeCommand(char *command){
        char **tokens = (char**) malloc((ARG_COUNT_MAX+1) * sizeof(char*));
        
        if (tokens == NULL) {
                fprintf(stderr, "malloc() cannot allocate memory");
                exit(EXIT_FAILURE);
        }
        
        char *token = strtok(command, DELIMITERS);
        int tokenIndex = 0;

        while (token != NULL && tokenIndex < ARG_COUNT_MAX) {
                tokens[tokenIndex] = (char*) malloc(strlen(token) * sizeof(char));

                if (tokens[tokenIndex] == NULL) {
                        fprintf(stderr, "malloc() cannot allocate memory");
                        exit(EXIT_FAILURE);
                }

                strcpy(tokens[tokenIndex], token);
                token = strtok(NULL, DELIMITERS);
                tokenIndex++;
        }

        if (token != NULL && tokenIndex == ARG_COUNT_MAX) {
                fprintf(stderr, "Error: too many process arguments\n");
                return NULL;
        }

        tokens[tokenIndex] = NULL; // terminate array of strings with NULL
        return tokens;
}

/* parses a command and returns Node with update attributes/flags
   Warning: command string is modified (send duplicate to preserve original string)
   Source: https://brennan.io/2015/01/16/write-a-shell-in-c/ */
Node *processCommand(char *command, char *commandLine) {
        Node* listNode = malloc(sizeof(Node));

        if (listNode == NULL) {
                fprintf(stderr, "malloc() cannot allocate memory");
                exit(EXIT_FAILURE);
        }

        char* token = NULL;
        char* fileName = NULL;
        //char* tok = NULL;
        //char* invalidChars = ">|";
        char* commandCopy = strdup(command);
        // reset args to ensure error checking terminates appropriately
        listNode->args = NULL;
        
        // parse for background task with error checking
        listNode->isBackgroundTask = false;
        if (strchr(command, '&')) {
                int commandLength = strlen(command);
                if (command[commandLength - 1] != '&' || charCount(command, '&') > 1) {
                        fprintf(stderr, "Error: mislocated background sign\n");
                        return listNode;
                }
                listNode->isBackgroundTask = true;
                listNode->cmdLine = commandLine;

                command[commandLength - 1] = '\0';
                commandCopy = strdup(command);
        }

        // If there is an append, then set the nodes value of isAppend to true
        listNode->isAppended = strstr(command, ">>");
        listNode->isTruncated = !listNode->isAppended;

        // parse and check for output redirection
        token = strtok(commandCopy, ">");
        if(!strcmp(token, command)) {
                listNode->args = tokenizeCommand(token);
                listNode->isAppended = listNode->isTruncated = false;
                return listNode;
        }
        
        // no command before redirection operator
        if (command == strchr(command, '>')) {
                fprintf(stderr, "Error: missing command\n");
                return listNode;
        }

        fileName = strrchr(command, '>') + 1;

        // missing output file
        if (isEmptyString(fileName)) {
                fprintf(stderr, "Error: no output file\n");
                return listNode;
        }

        // checks for any extra arguments in file name
        fileName = fileNameCheck(fileName);
        if(fileName == NULL) {
                listNode->error = true;
                return listNode;
        }
        
        // check if file doesn't exit or does exist but can't be accessed
        // Source: https://www.ibm.com/docs/en/zos/2.1.0?topic=functions-access-determine-whether-file-can-be-accessed
        if(access(fileName, F_OK) == 0 && access(fileName, W_OK) != 0) {
                fprintf(stderr, "Error: cannot open output file\n");
                return listNode;
        }

        listNode->args = tokenizeCommand(token);
        listNode->fileName = fileName;
        return listNode;
}

/* appends command to end of Linked List */
bool appendCommand(List *commandList, char *command, char *commandLine) {
        command = trimString(command);

        if(strlen(command) == 0)
                return false;

        Node *commandNode = processCommand(command, commandLine);

        //printf("command is good\n");
        appendList(commandList, commandNode);

        // skip append if any errors occurred during command processing
        if (!commandNode->args) {
                return false;
        }

        return true;
}

bool parsePipeline(List *commandList, char *commandLine) {
        char *commandLineCopy = strdup(commandLine);
        bool parseError = false;
        char *command;

        while (!parseError && (command = strtok_r(commandLineCopy, "|", &commandLineCopy)))
                parseError = !appendCommand(commandList, command, commandLine);
        
        // command not provided before or after pipe character
        //printf("%d %d\n", commandList->length, charCount(commandLine, '|') + 1);
        if (commandList->length != charCount(commandLine, '|') + 1) {
                fprintf(stderr, "Error: missing command\n");
                return false;
        }

        if (parseError)
                return false;

        
        // // output redirect is not the last command
        for(Node *cmd = commandList->head; cmd != NULL; cmd = cmd->next) {
                if ((cmd->isTruncated || cmd->isAppended) && cmd != commandList->tail) {
                        fprintf(stderr, "Error: mislocated output redirection\n");
                        return false;
                }
        }

        return true;
}

bool verifyBackgroundTasks(List *commandList) {
        Node *command = commandList->head;
        commandList->hasBgTask = false;

        for (int i = 0; i < commandList->length; i++) {
                if (command->isBackgroundTask) {
                        if (command != commandList->tail) {
                                fprintf(stderr, "Error: mislocated background sign\n");
                                return false;
                        }
                        commandList->hasBgTask = true;
                }
                command = command->next;
        }
        
        return true;
}

/* checks for built in comands and executes them, returns true if builtin command was found */
bool builtInCommand(Node *command, char *commandLine, BgList *bgList) {
        if (!strcmp(command->args[0], "exit")) {
                if(bgList->length > 0) {
                        command->status = EXIT_FAILURE;
                        fprintf(stderr, "Error: active jobs still running\n");
                        return true;
                }
                command->status = EXIT_SUCCESS;
                fprintf(stderr, "Bye...\n");
                fprintf(stderr, "+ completed '%s' [%d]\n", commandLine, command->status);
                exit(EXIT_SUCCESS);
        } else if (!strcmp(command->args[0], "cd")) {
                if(!chdir(command->args[1])) {
                        command->status = EXIT_SUCCESS;
                } else {
                        fprintf(stderr, "Error: cannot cd into directory\n");
                        command->status = EXIT_FAILURE;
                }
                return true;
        } else if (!strcmp(command->args[0], "pwd")) {
                char output[FILE_PATH_MAX];
                getcwd(output, sizeof(output));
                fprintf(stdout, "%s\n", output);
                command->status = EXIT_SUCCESS;
                return true;
        }
        return false;
}


/* Source 1: https://stackoverflow.com/questions/8082932/connecting-n-commands-with-pipes-in-a-shell
Source 2: https://stackoverflow.com/questions/60804552/pipe-two-or-more-shell-commands-in-c-using-a-loop */
void processPipeline(List *commands, char *commandLine, BgList *bgList) {
        int fd[2];
        int previousPipe = STDIN_FILENO;

        Node *command = commands->head;
        for (int i = 0; i < commands->length - 1; i++) {
                pipe(fd);
                pid_t pid = fork();
                if (pid == 0) {
                        if (previousPipe != STDIN_FILENO) {
                                dup2(previousPipe, STDIN_FILENO);
                                close(previousPipe);
                        }

                        if (fd[1] != STDOUT_FILENO) {
                                dup2(fd[1], STDOUT_FILENO);
                                close(fd[1]);
                        }

                        if (!builtInCommand(command, commandLine, bgList)) {
                                command->status = EXIT_SUCCESS;
                                execvp(command->args[0], command->args);
                                perror("Error: command not found\n");
                                command->status = EXIT_FAILURE;
                        }
                } else if (pid < 0) {
                        fprintf(stderr, "Error: fork() cannot spawn child\n");
                        exit(EXIT_FAILURE);
                }
                
                command->pid = pid;
                close(fd[1]);
                previousPipe = fd[0];
                command = command->next;
        }

        if (builtInCommand(command, commandLine, bgList))
                return;
                
        pid_t pid = fork(); 
        if (pid == 0) {
                // Child
                if (previousPipe != STDIN_FILENO) {
                        dup2(previousPipe, STDIN_FILENO);
                        close(previousPipe);
                }
                
                if(command->isAppended) {
                        fd[1] = open(command->fileName, O_RDWR | O_CREAT | O_APPEND, 0644);
                        dup2(fd[1], STDOUT_FILENO);
                        close(fd[1]);
                }

                if(command->isTruncated) {
                        fd[1] = open(command->fileName, O_RDWR | O_CREAT | O_TRUNC, 0644);
                        dup2(fd[1], STDOUT_FILENO);
                        close(fd[1]);
                }

                execvp(command->args[0], command->args);
                fprintf(stderr, "Error: command not found\n");
                exit(EXIT_FAILURE);
        } else if (pid > 0) {
                // Parent
                int retval;
                command->pid = pid;
                if(!command->isBackgroundTask)
                        waitpid(pid, &retval, 0);
                command->status = WEXITSTATUS(retval);
        } else {
                fprintf(stderr, "Error: fork() cannot spawn child\n");
                exit(EXIT_FAILURE);
        }
}

/* prints completion message to stderr for all commands with return values */
void printResultCommand(List *commandList, char *commandLine) {
        Node *command;
        fprintf(stderr, "+ completed '%s' ", commandLine);
        for(command = commandList->head; command != NULL; command = command->next)
                fprintf(stderr, "[%d]", command->status);
        fprintf(stderr, "\n");
}

/* prints completion message to stderr for background tasks with return values  */
void printResultBackground(BgNode *bgTask) {
        char *status = bgTask->status;
        fprintf(stderr, "+ completed '%s' ", bgTask->commandLine);
        for(int i = 0; status[i] != '\0'; i++)
                fprintf(stderr, "[%c]", status[i]);
        fprintf(stderr, "\n");
}

/* Source: https://stackoverflow.com/questions/4200373/just-check-status-process-in-c */
void checkBgTasks(BgList *bgList) {
        BgNode *task = bgList->head;
        BgNode *nextTask = task;
        int length = bgList->length;
        pid_t return_pid; 
        int status;

        //printf("before current bglist length check: %d\n", bgList->length);
        for (int i = 0; i < length; i++) {
                task = nextTask;
                return_pid = waitpid(task->pid, &status, WNOHANG);
                // status may have changed for last command (background task)
                task->status[strlen(task->status) - 1] = status ? ZERO_ASCII + 1 : ZERO_ASCII;

                //printf("return: %d, node pid: %d\n", return_pid, task->pid);
                // background task has finished
                if (return_pid == task->pid) {
                        printResultBackground(task);
                        
                        if(bgList->length == 1) {
                                bgList->head = bgList->head->next;
                                bgList->tail = bgList->tail->prev;
                                task->prev = task->next;
                        } else if(task->prev == NULL) { // head node
                                bgList->head = bgList->head->next;
                                nextTask = task->next;
                        } else if (task->next == NULL) { // tail node
                                bgList->tail = bgList->tail->prev;
                        } else {
                                task->prev = task->next;
                                nextTask = task->next;
                        }

                        free(task);
                        bgList->length--;
                }
        }
        //printf("after current bglist length check: %d\n", bgList->length);
}

void appendBgTask(BgList *bgList, List *commandList, char *commandLine) {
        BgNode *newTask = malloc(sizeof(BgNode));

        if (newTask == NULL) {
                fprintf(stderr, "malloc() cannot allocate memory");
                exit(EXIT_FAILURE);
        }
       
        newTask->commandLine = strdup(commandLine);
        newTask->pid = commandList->head->pid;
        newTask->status = malloc(commandList->length);

        if (newTask->status == NULL) {
                fprintf(stderr, "malloc() cannot allocate memory");
                exit(EXIT_FAILURE);
        }

        // store current status values of each command
        Node *command = commandList->head;
        for (int i = 0; i < commandList->length; i++) {
                newTask->status[i] = command->status ? ZERO_ASCII + 1 : ZERO_ASCII;
                command = command->next;
        }

        //printf("commandline bgtask: %s\n", commandLine);
        //printf("command status: %s\n", newTask->status);
        appendBgList(bgList, newTask);
}

int main(void) {
        char commandLine[CMD_LINE_MAX];
        List *commandList = malloc(sizeof(List));
        BgList *bgList = malloc(sizeof(BgList));

        if (commandList == NULL || bgList == NULL) {
                fprintf(stderr, "malloc() cannot allocate memory");
                exit(EXIT_FAILURE);
        }

        initBgList(bgList);

        while (true) {
                initList(commandList);

                getCommandLine(commandLine);
                processCommandLine(commandLine);
                        
                if (!isEmptyString(commandLine)) {
                        //printf("not empty\n");
                        if (!parsePipeline(commandList, commandLine)) {
                                freeList(commandList);
                                continue;
                        }
                        //printf("pipe parsed\n");
                        if (!verifyBackgroundTasks(commandList)) {
                                freeList(commandList);
                                continue;
                        }
                        //printf("bg verified\n");

                        checkBgTasks(bgList);
                        processPipeline(commandList, commandLine, bgList);
                        //printf("pipe processed\n");
                }

                
                //printf("BG checked\n");
                if (commandList->hasBgTask) {
                        appendBgTask(bgList, commandList, commandLine);
                        //printf("task appended\n");
                } else if(!isEmptyString(commandLine)) {
                        checkBgTasks(bgList);
                        printResultCommand(commandList, commandLine);   
                        //printf("result printed\n");  
                }

                freeList(commandList);
        }

        return EXIT_SUCCESS;
}
