#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <fcntl.h>


// split line
// https://www.educative.io/edpresso/splitting-a-string-using-strtok-in-c

#define COMMAND_LEN 1024
#define MAX_SIZE 1024
#define DICT_SIZE 100

typedef struct EntryObj
{
  char *key;
  char *value;
} EntryObj;

typedef struct DictionaryObj
{
  int tableSize;
  int dictSize;
  EntryObj **table;

} DictionaryObj;

typedef struct EntryObj *Entry;
int hash(DictionaryObj* D, char *key);


Entry newEntry(char *key, char *value)
{
    Entry n = malloc(sizeof(EntryObj));
    n -> key = (char*)malloc(strlen(key));
    n -> value = (char*)malloc(strlen(value));
    strcpy(n -> key, key);
    strcpy(n -> value, value);
    // printf("newEntry(): %s %s\n",n->key,n->value);
    return n;
}

DictionaryObj* newDictionary(int dictSize)
{
    DictionaryObj* d = malloc(sizeof(DictionaryObj));
    d->tableSize = 0;
    d->dictSize = dictSize;
    d->table = (Entry *)malloc(dictSize * sizeof(Entry ));
    for (int i = 0; i < dictSize; i++)
    {
        d->table[i] = NULL;
    }
    return d;
}

char *lookup(DictionaryObj* D, char *k)
{
    int array_offset = hash(D, k);
    if(D->table[array_offset] != NULL){
        if (strcmp(D->table[array_offset]->key, k) == 0)
        {
            return D->table[array_offset]->value;
        }
    }
    return NULL;
}

// insert()
// inserts new (key,value) pair into D
void insert(DictionaryObj* D, char *k, char *v)
{
    int array_offset = hash(D, k);
    Entry e = D->table[array_offset];
    Entry newE = newEntry(k,v);  
    if(e == NULL){
        if(D->tableSize == D->dictSize){
            printf("no more variables available to set");
            return;
        }
        else{
            D->table[array_offset] = newE;
            D->tableSize++;

        }
    }
    else {
        if (strcmp(e -> key, k) == 0) {
            strcpy(D -> table[array_offset] -> value, v);
            return;
        } 
    }
}

// printDictionary()
// pre: none
// prints a text representation of D to the file pointed to by out
void printDictionary(DictionaryObj* D)
{
    if(D->tableSize == 0){
        printf("no variables set\n");
        return;
    }

    for (int i = 0; i < D->dictSize; i++)
    {
        if (D -> table[i]) {
            printf("%s = %s\n", D->table[i]->key, D->table[i]->value);
        }
        
    }
}

unsigned int rotate_left(unsigned int value, int shift)
{
    int sizeInBits = 8 * sizeof(unsigned int);
    shift = shift & (sizeInBits - 1);
    if (shift == 0)
    {
        return value;
    }
    return (value << shift) | (value >> (sizeInBits - shift));
}

// pre_hash()
// turn a string into an unsigned int
unsigned int pre_hash(char *input)
{
    unsigned int result = 0xBAE86554;
    while (*input)
    {
        result ^= *input++;
        result = rotate_left(result, 5);
    }
    return result;
}

// hash()
// turns a string into an int in the range 0 to tableSize-1
int hash(DictionaryObj* D, char *key)
{
    return pre_hash(key) % D->dictSize;
}

bool fork_logic(char* command, char** components, char* lookup, int count){
    pid_t pid = fork();
        //printf("fork returned: %d\n", (int)pid );
    int status;
    if(pid < 0){
        perror("Fork Failed");
        exit(1);
    }
    if(pid == 0){
        //printf("I am the child with pid %d\n", (int)getpid());
        //sleep(5);
        execvp(command, components);
        if (execvp(command, components) < 0) {
            if(strcmp(lookup, "alias_lookup") == 0){
                for(int i = 0; i <count; i++){
                    printf("%s", components[i]);
                    if(i != count - 1)
                        printf(" ");
                    else if(i == count - 1)
                        printf("\n");
                }
            }
            else{
                printf("execvp failed\n");
            }
            exit(1);
        }
    }
    else if(pid > 0){
        //printf("I am the parent with pid %d\n", (int)getpid());
        wait(&status);
        if (WIFEXITED(status)) {
            int exit_status = WEXITSTATUS(status);
            if (exit_status != 0) {
                printf("exit code: %d\n", exit_status);
            }
        }
        return true;
    }
    return true;
}

bool process_components(char** components, int compCount, DictionaryObj* setDict,  DictionaryObj* aliasDict, char* line){


    char* command = components[0];
    if (command[strlen(command) - 1] == '\n'){
        strcat(command, "\0");
    }
    char checkVar = command[0]; 

    if(checkVar == '$'){
        char * v = lookup(setDict, command+=1);
        if(v == NULL){
            printf("no value for variable\n");
        }else{
            printf("%s\n", v);
        }
        return true;
    } 
    //printf("%s", command);
    char* lookup_alias_command = lookup(aliasDict, command);
    if(lookup_alias_command){
        char *lookup_components[100];
        int lookupCompCount = 0;

        char* token = strtok(lookup_alias_command, " ");
        while(token != NULL){
            lookup_components[lookupCompCount] = token;
            //printf("%s\n", lookup_components[lookupCompCount]);
            token = strtok(NULL, " ");
            lookupCompCount++;
        }
        lookup_components[lookupCompCount] = NULL;
        
        if(true){
            fork_logic(lookup_components[0],lookup_components, "alias_lookup", lookupCompCount);
            return true;
        }
        
        process_components(lookup_components, lookupCompCount, setDict, aliasDict, line);
        return true;
    }

    if(strcmp(command,"echo") == 0){
        int i = 1;
        while(i < compCount){
            printf("%s", components[i]);
            if(i != compCount - 1)
                printf(" ");
            else if(i == compCount - 1)
                printf("\n");
            i++;
        }
        return true;
    }
    else if (strcmp(command,"exit") == 0){
        return false;
    }
    else if (strcmp(command, "set") == 0){
        char value_set[MAX_SIZE];
        char val_set[MAX_SIZE];

        if(compCount == 1){
            printDictionary(setDict);
            return true;
        }

        value_set[0] = '\0';
        char* pPosition = strchr(components[1], '=');
        if (pPosition == NULL){
            for(int i = 3; i < compCount; i++){
                strcat(value_set, components[i]);
                if(i != compCount - 1)
                    strcat(value_set, " ");
            }
            strcpy(val_set, value_set);
            insert(setDict, components[1],val_set);
            memset(value_set, 0 , sizeof(value_set));
        }
        else{
            printf("check '=' sign, try again\n");
        }

        return true;
    }
    else if (strcmp(command, "alias") == 0){
        char value_alias[MAX_SIZE];
        char val_alias[MAX_SIZE];
        if(compCount == 1){
            printDictionary(aliasDict);
            return true;
        }

        value_alias[0] = '\0';
        for(int i = 2; i < compCount; i++){
            strcat(value_alias, components[i]);
            if(i != compCount - 1)
                strcat(value_alias, " ");
        }
        strcpy(val_alias, value_alias);
        insert(aliasDict, components[1],val_alias);
        memset(value_alias, 0 , sizeof(value_alias));


        return true;
    }
    else{
       return fork_logic(command, components, "", 0);
    }
    
}

bool process_redirect(char** components, int compCount, char* line, char* symbol, DictionaryObj* setDict, DictionaryObj* aliasDict){

    char* tok = strtok(line, symbol);
    while(tok != NULL){
        if ( tok[strlen(tok) - 1] == ' '){
            tok[strlen(tok) - 1] = '\0';
        }
        else if (tok[0] == ' '){
            tok+=1;
        }

        components[compCount] = tok;
        tok = strtok(NULL, symbol);
        compCount++;
    }
    components[compCount] = NULL;

    char* out_components[100];
    char* output = components[0];
    char* output_tok = strtok(line, " ");
    int outCompCount = 0;

    while(output_tok != NULL){
        out_components[outCompCount] = output_tok;
        output_tok = strtok(NULL, " ");
        outCompCount++;
    }
    out_components[outCompCount] = NULL;

    int file;
    int pid;
    int status;

    if(strcmp(symbol, ">") == 0){
        pid = fork();
        if(pid < 0){
            perror("Fork Failed");
            exit(1);
        }   
        if (pid == 0) {
            close(1);
            file = open(components[compCount-1], O_WRONLY | O_TRUNC | O_CREAT, 0666);
            execvp(out_components[0], out_components);
            exit(1);
        }
        else if(pid > 0){
            wait(&status);
            if (WIFEXITED(status)) {
                int exit_status = WEXITSTATUS(status);
                if (exit_status != 0) {
                    printf("exit code: %d\n", exit_status);
                }
            }
        }
    }
    else if(strcmp(symbol, "<") == 0){
        pid = fork();
        if(pid < 0){
            perror("Fork Failed");
            exit(1);
        }   
        if (pid == 0) {
            close(0);
            file = open(components[compCount-1], O_RDONLY, 0600);
            execvp(out_components[0], out_components);
            exit(1);
        }
        else if(pid > 0){
            wait(&status);
            if (WIFEXITED(status)) {
                int exit_status = WEXITSTATUS(status);
                if (exit_status != 0) {
                    printf("exit code: %d\n", exit_status);
                }
            }
        }
    }
    return true;
}


// https://stackoverflow.com/questions/33884291/pipes-dup2-and-exec

bool process_pipe(char** components , int compCount, char *line, char* symbol, DictionaryObj* setDict, DictionaryObj* aliasDict){
    char* tok = strtok(line, symbol);
    while(tok != NULL){
        if ( tok[strlen(tok) - 1] == ' '){
            tok[strlen(tok) - 1] = '\0';
        }
        else if (tok[0] == ' '){
            tok+=1;
        }

        components[compCount] = tok;
        printf("%s\n", components[compCount]);
        tok = strtok(NULL, symbol);
        compCount++;
    }
    components[compCount] = NULL;

    char *firstCommand[100];
    int firstCount = 0;
    char* tok_first = strtok(components[0], " ");

    while(tok_first != NULL){
        firstCommand[firstCount] = tok_first;
        printf("%s\n", firstCommand[firstCount]);
        tok_first = strtok(NULL, " ");
        firstCount++;
    }
    firstCommand[firstCount] = NULL;

    char *secondCommand[100];
    int secondCount = 0;
    char* tok_second = strtok(components[1], " ");

    while(tok_second != NULL){
        secondCommand[secondCount] = tok_second;
        printf("%s\n", secondCommand[secondCount]);
        tok_second = strtok(NULL, " ");
        secondCount++;
    }
    secondCommand[secondCount] = NULL;

    pid_t pid;
    int status;

    int fd[2];

    if (pipe(fd) < 0) 
        exit(1);
    
    pid = fork();

    if(pid < 0){
        perror("Fork Failed");
        exit(1);
    } 
    if (pid == 0) {
        pid = fork();
        dup2(fd[1], STDOUT_FILENO);
        close(fd[0]);
        close(fd[1]);
        execvp(firstCommand[0], firstCommand);
        fprintf(stderr, "Failed to execute %s\n", firstCommand[0]);
        exit(1);
    }
    else if(pid > 0){
        pid = fork();
        if(pid == 0){
            dup2(fd[0], STDIN_FILENO);
            close(fd[1]);
            close(fd[0]);
            execvp(secondCommand[0], secondCommand);
            fprintf(stderr, "Failed to execute %s\n", secondCommand[0]);
            exit(1);
        }
        else{
            close(fd[0]);
            close(fd[1]);
            wait(&status);
            wait(&status);
            if (WIFEXITED(status)) {
                int exit_status = WEXITSTATUS(status);
                if (exit_status != 0) {
                    printf("exit code: %d\n", exit_status);
                }
            }
        }
    }
    printf("here\n");
    
    return true;
}
    

int main(int argc, char *argv[]){
    bool cond = true;
    DictionaryObj* setDict = newDictionary(DICT_SIZE);
    DictionaryObj* aliasDict = newDictionary(DICT_SIZE);

    while(cond){
        printf("CShell>> ");
        char line[COMMAND_LEN];
        char *components[100];
        int compCount = 0;
        int line_length;


        fgets(line, COMMAND_LEN, stdin);
       
        line_length = strlen(line);
        if (line_length > 0 && line[line_length-1] == '\n') {
            line[line_length-1] = '\0';
        }

        char* pipe = strchr(line, '|');
        char* redirInput = strchr(line, '<');
        char* redirOutput = strchr(line, '>');
 
        if (redirOutput != NULL){
            process_redirect(components, compCount, line, ">", setDict, aliasDict);
        }
        else if (redirInput != NULL){
            process_redirect(components,compCount, line, "<", setDict, aliasDict);
        }
        else if (pipe != NULL){
            process_pipe(components,compCount, line, "|", setDict, aliasDict);
        }

        else{
            char* tok = strtok(line, " ");
            while(tok != NULL){
                components[compCount] = tok;
                tok = strtok(NULL, " ");
                compCount++;
            }
            components[compCount] = NULL; 
            
            cond = process_components(components, compCount, setDict, aliasDict, line);
        }
        //printf("I am the parent waiting for child to end %d\n", (int) getpid());
        
        //printf("Parent ending\n");
    }

    return EXIT_SUCCESS;
}