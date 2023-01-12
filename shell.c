#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/signal.h>
#include <signal.h>

static pid_t child_pid;
static int jobID = 1;



// Implementing list for handling jobs
typedef struct Node
{
    /* data */
    int jobID;
    int ppid;
    int pid;
    char* status;
    char* cmd;

    struct Node* next;
    
} Node;


void traverse(Node** head); // to check status of all jobs and print
void add(Node** head, int pid, char* cmd, int ppid); // to add jobs
void delete(Node** head, int jobID); // to delete jobs
Node* getNode(Node** head, int jid); // to get specific node needed
int contains(Node** head, int jobID);
int builtIn(char** command, Node** head); //built ins cd,fg,bg,kill,exit,jobs
int parseInput(char *input, char **command); //parsing an input
void freeList(Node** head); //to free linked list
void fg(int jid, Node** head); //function for fg
void bg(int jid, Node** head); //function for bg
void handler(int sig);

//Freeing linked list
void freeList(Node** head)
{
    while (*head != NULL)
    {
        Node* tmp = *head;
        *head = (*head)->next;
        free(tmp->cmd);
        free(tmp->status);
        free(tmp);
    }
    
}

// Search and return the specific Node
Node* getNode(Node** head, int jid)
{
    Node* curr = *head;
    while(curr != NULL && curr->jobID != jid)
        curr = curr->next;
    return curr;
}


// iterate to check status and prints
void traverse(Node** head)
{
    Node* curr = *head;
    if(curr == NULL)
        return;
    int status;
    
    while(curr != NULL)
    {
        pid_t ret_val = waitpid(curr->pid,&status,WNOHANG|WUNTRACED);
        if (ret_val  == -1)  
        {
            //Deletes the completed jobs
            Node* next = curr->next;
            delete(head, curr->jobID);
            curr = next;
            continue;        
        }
        else if(strcmp(curr->status,"Terminated") == 0) 
        {
            // Deletes terminated jobs
            printf("[%d]  %d   %s   %s",curr->jobID, curr->pid, curr->status, curr->cmd);
            Node* next = curr->next;
            delete(head, curr->jobID);
            curr = next;
            continue;
        }
        else if(ret_val  == 0 && !strcmp(curr->status,"Stopped") == 0)
        {
            //check for running process
            strcpy(curr->status, "running");
        }
        else if(WIFSTOPPED(status))
        {
            // check for stopped process
            strcpy(curr->status, "Stopped");
            printf("[%d]  %d   %s   %s",curr->jobID, curr->pid, curr->status, curr->cmd);
            curr = curr->next;
            continue;
        }
        else if(ret_val  == curr->pid)
        {
            //check for completed process
            Node* next = curr->next;
            delete(head, curr->jobID);
            curr = next;
            continue;
        }
        printf("[%d]  %d   %s   %s",curr->jobID, curr->pid, curr->status, curr->cmd);
        curr = curr->next;
    }
}

// ADDS JOBS
void add(Node** head, int pid, char* cmd, int ppid)
{
    Node* new = malloc(sizeof(Node));
    new->cmd = (char*) malloc(strlen(cmd)* sizeof(char*));
    new->status = (char*) malloc(100* sizeof(char*));
    if(new == NULL)
    {
        return;
    }

    if(*head == NULL)
        jobID = 1;
    
    new->jobID = jobID;
    new->pid = pid;
    new->ppid = ppid;
    strcpy(new->cmd,cmd);
    jobID++;    
    new->next = NULL;

    if(*head == NULL)
    {
        *head = new;
        return;
    }

    Node* curr = *head;
    while(curr->next != NULL)
    {
        curr = curr->next;
    }
    curr->next = new;
}

// DELETES JOBS
void delete(Node** head, int jid)
{
    Node* curr = *head;
    if((*head) == NULL)
    {
        return;
    }

    if ((*head)->jobID == jid)
    {
        Node* toRemove = *head;
        *head = (*head)->next;
        free(toRemove->cmd);
        free(toRemove->status);
        free(toRemove);
        return;
    }
    
    while(curr->next != NULL)
    {
        if(curr->next->jobID == jid)
        {
            Node* toRemove = curr->next;
            curr->next = curr->next->next;
            free(toRemove->cmd);
            free(toRemove->status);
            free(toRemove);
            if(curr->next == NULL)
            {
                jobID--;
            }
            return;
        }
        curr = curr->next;
    }
}

int contains(Node** head, int jobID)
{
    Node* curr = *head;

    if(*head == NULL)
        return 0;
    
    while (curr != NULL)
    {
        if(curr->jobID == jobID)
        {
            return 1;
        }
        curr->next = curr;
    }
    return 0;  
}










// Main Functions
// FG FUNCTION
void fg(int jid, Node** head)
{
    Node* tmp = getNode(head, jid);

    if(tmp != NULL)
    {
        int status;
        kill(tmp->pid, SIGCONT);
        child_pid = tmp->pid;
        waitpid(tmp->pid,&status,0);
    }
    else
    {
        printf("fg %%%d: No Such Job\n", jid);
    } 
}

// BG FUNCTION
void bg(int jid, Node** head)
{ 
    Node* tmp = getNode(head, jid);

    if(tmp != NULL)
    {
        if(strcmp(tmp->status, "Stopped")== 0)
        {
            strcpy(tmp-> status, " ");
            kill(tmp->pid, SIGCONT);
        }
    }
    else
        printf("bg %%%d: No Such Job\n", jid); 
}

// Built In CD, BG, FG, KILL, EXIT, JOBS
int builtIn(char** command, Node** head)
{
    char dir[100];
    if(strcmp(command[0],"cd") == 0)
        {
            if(command[1] == NULL)
                return 1;
            
            if(strcmp(command[1],"..") == 0)
            {
                chdir("..");
                printf("%s\n", getcwd(dir, sizeof(dir)));
            }
            else if(command[1] != NULL && command[2] == NULL)
            {
                getcwd(dir, sizeof(dir));
                strcat(dir, "/");
                strcat(dir, command[1]);

                int num = chdir(dir);
                if(num >= 0)
                    printf("%s\n", dir);
                else
                {
                    perror(command[1]);
                }
            }
            else
            {
               printf("%s: too many arguments\n", command[0]); 
            }
            return 1;
        }
        if(strcmp(command[0],"jobs") == 0)
        {
            traverse(head);
            return 1;
        }
        if(strcmp(command[0], "fg") == 0)
        {
            fg(atoi(&command[1][1]), head);
            return 1;
        }
        if(strcmp(command[0], "bg") == 0)
        {
            bg(atoi(&command[1][1]), head);
            return 1;
        }
        if(strcmp(command[0],"kill") == 0)
        {
            int jid = atoi(&command[1][1]);
            Node* tmp = getNode(head, jid);
            if(tmp != NULL)
            {
                strcpy(tmp->status, "Terminated");
                kill(tmp->pid, SIGTERM);
                printf("[%d]   %d   Terminated by signal %d\n", tmp->jobID, tmp->pid, SIGTERM);
            }
            else
                printf("kill %%%d: No Such Job\n", jid);
            return 1;
        }
       return 0;
}

// Parsing Input
int parseInput(char *input, char **command) 
{
    char *parsed;
    int index = 0;
    int bg = 0; //to check for "&" to send process in bg

    parsed = strtok(input, " ");
    while (parsed != NULL) {
        command[index] = parsed;
        index++;
        parsed = strtok(NULL, " ");
    }
    
    command[index] = NULL;

    if(index == 0)
        return 0;

    if(strcmp(command[index-1],"&") == 0)
    {
        command[--index] = NULL;
        bg = 1;
    }
    return bg;
}

// Signal Hnadler
void handler(int sig)
{
    char space[2];
    if(sig == SIGQUIT)
    {
        kill(child_pid, SIGHUP);
        kill(child_pid, SIGCONT);
    }
    if(child_pid != 0)
    {
        if(!kill(child_pid, 0))
            kill(child_pid, sig);
    }
    if(sig == SIGINT && child_pid != 0)
    {
        char output[100];
        snprintf(output,100,"\n[%d]   %d  Terminated by signal %d", jobID -1, child_pid, SIGINT);
        write(1,output, strlen(output)+1);
    }
    snprintf(space,2,"\n");
    write(1,space, strlen(space)+1);
}









// MAIN
int main(int argc, char *argv[])
{
    int nRows = 30, nCol = 30, bg, status;
    char cmd[100];
    char* input = (char*) malloc(100* sizeof(char));
    char **command = (char **)malloc(nRows * sizeof(char *));
    strcpy(input, " ");
    Node* head = NULL;

    struct sigaction action;

    action.sa_handler = handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    if( sigaction(SIGINT, &action, NULL) == -1)
        perror("error:");
    if( sigaction(SIGTSTP, &action, NULL) == -1)
        perror("error:");
    if( sigaction(SIGQUIT, &action, NULL) == -1)
        perror("error:");

    sigset_t mask_all, mask_one, prev_one;
    sigfillset(&mask_all);
    sigemptyset(&mask_one);
    sigaddset(&mask_one, SIGCHLD);
    sigaddset(&mask_one, SIGTSTP);

    while(1)
    {
        printf("> ");
        fgets(input,99,stdin);

        if(feof(stdin))
        {
            exit(0); // exit if user hit ctrl d
        }

        strcpy(cmd, input);
        if(strlen(input) == 1)
            continue;

        input[strlen(input)-1] = '\0';

        bg = parseInput(input, command);

        if(command[0] == NULL)
            continue;

        if(strcmp(command[0], "exit") == 0)
        {
            freeList(&head);
            free(input);
            free(command);
            kill(child_pid, SIGHUP);
            kill(child_pid, SIGCONT);
            break;
        }
        
        if(builtIn(command, &head))
        {
            strcpy(input, " ");
            continue;
        }

        sigprocmask(SIG_BLOCK, &mask_one, &prev_one);
        child_pid = fork();

        // IN CHILD 
        if(child_pid == 0)
        {
            sigprocmask(SIG_SETMASK, &prev_one, NULL);
            setpgid(getpid(), 0);
            int stat = execvp(command[0], command);
            
            if(stat == -1)
            {
                int a = 0;
                char *temp = command[0];
                for(int i=0; i<strlen(command[0]); i++)
                {
                    if(temp[i] != '/')
                        a++;
                }                    
                if(a==strlen(command[0]))
                {
                    fprintf(stderr, "%s: command not found\n", command[0]);
                }
                else
                    perror(command[0]);
            }
            exit(stat);
        }

        sigprocmask(SIG_BLOCK, &mask_all, NULL);
        add(&head, child_pid,cmd,getpid());
        sigprocmask(SIG_SETMASK, &prev_one, NULL);

        //IN PARENT 
        if(!bg)
            waitpid(child_pid, &status, 0);
        else
            printf("[%d]  %d\n", jobID-1, child_pid);
        
        child_pid = 0;
        strcpy(input, " ");
    }
}