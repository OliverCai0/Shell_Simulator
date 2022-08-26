#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>

void myPrint(char *msg)
{
    write(STDOUT_FILENO, msg, strlen(msg));
}

void errorMsg(int exit_toggle){
    char error_message[30] = "An error has occurred\n";\
    write(STDOUT_FILENO, error_message, strlen(error_message));
    if(exit_toggle){
        exit(1);
    }
}

/*
 *Return args separated by whitespace,
 *Will handle the overflow (more than 512 and no new line character)
 */
char** lineSanitize(char* input_command, int *arg_number){
    char command[512];
    strcpy(command, input_command);
    //initialize the returned array
    //printf("sanitizing %s \n", command);
    char **args = (char**)malloc(sizeof(char*) * 256);
    //printf("stil works\n");
    if(args == NULL){
        printf("it's over grover\n");
        exit(1);
    }
    /*
    for(int i = 0; i < 512; i++){
        //buffer for each white space separated command
        args[i] = (char*)malloc(sizeof(char) * 200);
        if(args[i] == NULL){
            char error[13] = "malloc error";
            myPrint(error);
            exit(1);
        }
        memset(args[i], '\0', 200);
    }
    */
    //printf("stil works\n");
    
    int arg_pointer = 0;
    char* token = strtok(command, ";");
    while(token != NULL){
        if(token[strlen(token) - 1] == '\n'){
            token[strlen(token) - 1] = '\0';
        }
        args[arg_pointer] = (char*)malloc(sizeof(char) * (strlen(token) + 1));
        strcpy(args[arg_pointer], token);
        arg_pointer++;
        token = strtok(NULL, ";");
    }

    /*
    * Debug
    */
    /*
    myPrint("debuggin time \n");
    for(int i = 0; i < 512; i++){
        if(strlen(args[i]) > 0){
            myPrint(args[i]);
            myPrint("\n");
        }
    }
    */
    //return args
    //printf("Number of args: %d\n", arg_pointer);
    *arg_number = arg_pointer;
    return args; 
}

int goBack(){
    return 0;
}

int goThere(char* dest){
    return 0;
}
/*
int changeDirectory(char* arg){
    char* token = strtok(arg, "/");
    while(token != NULL){
        if(strcmp(token, "..") == 0 || strcmp(token, "..\n")){
            goBack();
        }
        else{
            goThere(token);
        }
        token = strtok(NULL, "/");
    }
    return 0;
}

*/

void getDirectory(){
    char buff[200];
    getcwd(buff, 200);
    myPrint(buff);  
    myPrint("\n");
}

int occurences(char* str, char c){

    int count = 0;
    for(int i = 0; i < strlen(str); i++){
        if(str[i] == c){
            count += 1;
        }
    }
    return count;

}

/*
 * returns [cmd, flags, args]
*/
char **getTokens(char* command, int* num_of_tokens){
    char** tokens = (char**)malloc(sizeof(char*) * 100);
    char temp[512];
    char* a;
    char* b;
    char* a_pointer;
    char* b_pointer;
    strcpy(temp, command);
    int token_pointer = 0;
    for(a = strtok_r(temp,"\t",&a_pointer); a != NULL; a=strtok_r(NULL, "\t", &a_pointer)){
        for(b = strtok_r(a," ",&b_pointer); b != NULL; b=strtok_r(NULL, " ", &b_pointer)){
            tokens[token_pointer] = (char*)malloc(sizeof(char) * (strlen(b) + 1));
            strcpy(tokens[token_pointer], b);
            token_pointer++;
            *num_of_tokens += 1;
        }
    } 
    //printf("Successfully returned tokens\n");
    tokens[token_pointer] = NULL;
    return tokens;
}


char **getAdvRedirection(char* command, int* num_of_tokens, int* redirect_error){
    char** tokens = (char**)malloc(sizeof(char*) * 2);
    char *pointer;
    if(((pointer = strstr(command, ">+")) != NULL) ){
        //printf("detected adv pointer\n");
        *num_of_tokens += 1;
        char temp[512];
        strcpy(temp, command);
        char* token = strtok(temp, ">+");
        if(token == NULL || (occurences(command, '>') > 1)){
            *redirect_error = 1;
            //printf("exit null oken\n");
            return tokens;
        }
        tokens[0] = (char*)malloc(sizeof(char) * (strlen(token) + 1));
        strcpy(tokens[0], token);
        if(((token = strtok(NULL, ">+")) != NULL) ){
            //printf("token for advanced: %s\n", token);
            char* cleaned = strtok(token, " ");
            if(cleaned == NULL){
                //printf("secondary error");
                *redirect_error += 1;
                return tokens;
            }
            tokens[1] = (char*)malloc(sizeof(char) * (strlen(cleaned) + 1));
            strcpy(tokens[1], cleaned);
            //*num_of_tokens += 1;
            //printf("second half of advanced: %s\n", tokens[1]);
        }
        else{
            //printf("second half is empty\n");
            free(tokens[0]);
            *redirect_error = 1;
            return tokens;
        }
    }
    /*
    char* token = strtok(temp, ">+");
    tokens[0] = strdup(token);
    //(char*)malloc(sizeof(char) * strlen(token));
    //strcpy(tokens[0], token);
    if(((token = strtok(NULL, ">+")) != NULL) && token[0] == '+'){
        printf("token for advanced: %s\n", token);
        char* cleaned = strtok(token, " ");
        tokens[1] = (char*)malloc(sizeof(char) * strlen(cleaned));
        strcpy(tokens[1], cleaned);
        *num_of_tokens += 1;
        printf("second half of advanced: %s\n", tokens[1]);
    }
    */
    return tokens;
}

char **getRegRedirection(char* command, int* num_of_tokens, int* redirect_error){
    char** tokens = (char**)malloc(sizeof(char*) * 2);
    char* pointer;
    if((pointer = strstr(command, ">")) != NULL){
        char temp[512];
        strcpy(temp, command);
        char* token = strtok(temp, ">");
        //printf("token for reg first: %s\n", token);
        if(token == NULL || (occurences(command, '>') > 1)){
            *redirect_error = 1;
            //printf("exit null oken\n");
            return tokens;
        }
        tokens[0] = (char*)malloc(sizeof(char) * (strlen(token) + 1));
        strcpy(tokens[0], token);
        //printf("copied over\n");
        if((token = strtok(NULL, ">")) != NULL){
            //printf("enetered if\n");
            char* cleaned = strtok(token, " ");
            if(cleaned == NULL){
                //printf("secondary error");
                *redirect_error += 1;
                return tokens;
            }
            tokens[1] = (char*)malloc(sizeof(char) * (strlen(cleaned) + 1));
            strcpy(tokens[1], cleaned);
            //*num_of_tokens += 1;
        }
        else{
            //printf("second half is empty\n");
            free(tokens[0]);
            *redirect_error = 1;
            return tokens;
        }
        /*
        else{
            printf("secondary error");
            *redirect_error = 1;
            return tokens;
        }
        */
        *num_of_tokens += 1;
    }
    return tokens;
}

int parseCommands(char **kargs, int batchfile, char* command, int num_of_args){
    //int* children = (int*)malloc(sizeof(int) * 100);
    char **args;
    char **reg;
    char **adv;
    int line_printed = 0;
    for(int i = 0; i < num_of_args; i++){
        //check for special operator

        if(strlen(kargs[i]) <= 0){
            break;
        }
        int advanced_redirection = 0;
        int regular_redirection = 0;
        int adv_error = 0;
        int reg_error = 0;
        adv = getAdvRedirection(kargs[i], &advanced_redirection, &adv_error);
        if(!advanced_redirection && !adv_error){
            //printf("initializd reg\n");
            reg = getRegRedirection(kargs[i], &regular_redirection, &reg_error);
        }
        //printf("redirection passed\n");
        int num_of_tokens = 0;
        if(regular_redirection && !reg_error){
            //printf("regular redirection entered\n");
            args = getTokens(reg[0], &num_of_tokens);
            //printf("after get Tokens call: %s\n", reg[0]);
        }
        else if(advanced_redirection && !adv_error){
            //printf("advanced redirection entered\n");
            //printf("advanced redirection args: %s, %s\n", adv[0], adv[1]);
            args = getTokens(adv[0], &num_of_tokens);
        }
        else{
            //printf("neither redirection \n");
            args = getTokens(kargs[i], &num_of_tokens); 
        }
        //printf("Number of tokens: %d\n", num_of_tokens);
        if((num_of_args > 1 || num_of_tokens > 0 || adv_error || reg_error) && batchfile && !line_printed){
            myPrint(command);
            line_printed = 1;
        }
        if(num_of_tokens == 0){
            //printf("Command skipped \n");
            if(regular_redirection || advanced_redirection){
                myPrint(command);
                errorMsg(0);
            }
        }
        else if(adv_error || reg_error){
            //printf("redirection error\n");
            errorMsg(0);
            
        }
        else if((strcmp(args[0], "cd") == 0 || strcmp(args[0], "pwd") == 0 || strcmp(args[0], "exit") == 0)\
            && (regular_redirection || advanced_redirection)){
            //printf("caught error \n");
            errorMsg(0);
        } 
        else{
        if(strcmp(args[0], "cd") == 0 || strcmp(args[0], "cd\n") == 0){
            //myPrint("Found cd command\n");
            if(num_of_tokens > 2){
                errorMsg(0);
            }
            else if(num_of_tokens == 1){
                //printf("%s",getenv("HOME"));
                chdir(getenv("HOME"));
                //myPrint("Changed to home\n");
            }
            //changeDirectory(cd_args); 
            else{
                if(chdir(args[1]) == -1){
                    errorMsg(0);
                }
            }
        }
        else if(strcmp(args[0], "pwd") == 0 || strcmp(args[0], "pwd\n") == 0){
            //myPrint("Got pwd\n");
            if(num_of_tokens > 1){
                errorMsg(0);
            }
            else{
                getDirectory();
            }
        }
        else if(strcmp(args[0], "exit") == 0){
            if(num_of_tokens > 1){
                errorMsg(0);
            }
            else{
                exit(0);
            }
        
        }
        else{
            //printf("execvp entered and num of tokens is %d\n", num_of_tokens);
            if(fork() == 0){
                if(regular_redirection && !advanced_redirection){
                    //printf("child detected regular \n");
                    //printf("trying to open %s\n", reg[1]);
                    if(!access(reg[1], F_OK)){
                        errorMsg(1);
                    }
                    int new_output = open(reg[1], O_RDWR | O_CREAT, 00700);
                    if(new_output == -1){
                        errorMsg(1);
                    }
                    dup2(new_output, STDOUT_FILENO);
                    if(execvp(args[0], args) == -1){
                        errorMsg(1);
                    }
                }
                else if(advanced_redirection){
                    //copy the file to buffer
                    //else{
                    int output = open(adv[1], O_RDWR, 00700);
                    int length = lseek(output, 0, SEEK_END); 
                    lseek(output, 0, SEEK_SET);
                    char *buffer = (char*)malloc(sizeof(char) * (length + 1));
                    read(output, buffer, length);
                    //printf("opened file: %s\n", adv[1]);
                    //printf("file length: %d\n", length);
                    //printf("Copied file: %s\n",buffer);
                    lseek(output, 0, SEEK_SET);
                    int trunced = open(adv[1], O_RDWR | O_CREAT | O_TRUNC, 00700);
                    if(trunced == -1){
                        errorMsg(1);
                    }
                    else{
                    dup2(trunced, STDOUT_FILENO);
                    int c;
                    if((c = fork()) == 0){
                        
                        if(execvp(args[0], args) == -1){
                            errorMsg(1);
                        }
                    }
                    else{
                        int child_status;
                        wait(&child_status);
                        //printf("child terminated\n");
                        lseek(STDOUT_FILENO, 0, SEEK_END);
                        write(STDOUT_FILENO, buffer, length);
                        exit(0);
                    }
                    }
                    //}
                }
                else if(execvp(args[0], args) == -1){
                    errorMsg(1);
                }
            }
            else{
                int status;
                wait(&status);
                //printf("parent escaped \n");
            }
        }
        }
        //printf("corruption check 1 \n");
        
        for(int f = 0; f < num_of_tokens; f++){
            free(args[f]);
        }
        
        //printf("corruption check 2 \n");
        if(advanced_redirection && !adv_error){
            //printf("advanced redirection \n");
            free(adv[1]);
            free(adv[0]);
            //printf("yes\n");
        }
        //free(adv[0]);
        
        //printf("corruption check 3 \n");
        if(regular_redirection && !reg_error){
            //printf("regular redirection \n");
            //printf("reg 0: %s, reg 1: %s\n", reg[0], reg[1]);
            free(reg[0]);
            //printf("freed reg0\n");
            free(reg[1]);
            //printf("free reg1\n");
        }
        //printf("corruption check 4 \n");
    }
    //printf("done parsing\n");
    return 1;
}

int main(int argc, char *argv[]) 
{
    char cmd_buff[514]; //= (char*)malloc(sizeof(char) * 513);
    //memset(cmd_buff, '\0', 513);
    char *pinput;
    int status;
    int batchfile = 0;
    //int* children;
    //printf("arguments given: %d\n", argc);
    if(argc > 1){
        if(access(argv[1], F_OK)  == -1){
            errorMsg(1);
        }
        if((batchfile = open(argv[1], O_RDONLY, 0666)) == -1){
            //printf("bad batch %s\n", argv[1]);
            exit(1);
        }
        dup2(batchfile, STDIN_FILENO);
    }
    else if(argc > 2){
        errorMsg(1);
    } 
    while (1) {
        int num_of_args = 0;
        wait(&status);
        if(!batchfile){
            myPrint("myshell> ");
        }
        pinput = fgets(cmd_buff, 514, stdin);
        if (!pinput) {
            //myPrint("line 126\n");
            exit(0);
        }
        //char c;
        /*

        if((cmd_buff[strlen(cmd_buff) - 1] != '\n') && ((c = getchar()) != EOF) && (c != '\n')){
           myPrint(cmdbuff);
           while(cmd_buff[strlen(cmd_buff) - 1] != '\n'){
                fgets(cmd_buff, 512, stdin);
                myPrint(cmdbuff);
           }
        }
        */
        //else{
        /*
        if(batchfile){
            //myPrint("Output command\n");
            myPrint(pinput);
        }
        */
        /*
        myPrint("Subroutine to get to end of stdin\n");
        int c;
        while((c = getc(stdin)) != EOF){
            printf("not at end \n");
        }
        myPrint("Successfully exited subroutine\n");
        //myPrint(cmd_buff);
        */  
        //printf("command: %s", pinput);
        //char **args = lineSanitize(pinput, &num_of_args);
        //children = 
        //printf("done sanitizing\n");
        if((pinput[strlen(pinput) - 1] != '\n')){
           //errorMsg(0);
           myPrint(pinput);
           while(cmd_buff[strlen(pinput) - 1] != '\n'){
                //myPrint("getting rest of line\n");
                pinput = fgets(cmd_buff, 514, stdin);
                //myPrint("got line\n");
                myPrint(pinput);
           }
           errorMsg(0);
        }
        else{
            
            char **args = lineSanitize(pinput, &num_of_args);
            if(num_of_args > 0){
            parseCommands(args, batchfile, pinput, num_of_args);
        }         
        else{
            //printf("empty line \n");
        }
        
        for(int i = 0; i < 256; i++){
            free(args[i]);
        }
        }
        //printf("try to free after parse\n");
        //for(int i = 0; i < 256; i++){
        //    free(args[i]);
        //}
        /*
        if((cmd_buff[strlen(pinput) - 1] != '\n')){
           myPrint(pinput);
           while((pinput != NULL) || pinput[strlen(pinput) - 1] != '\n'){
                pinput = fgets(cmd_buff, 512, stdin);
                myPrint(pinput);
           }
        }
        */
        //printf("successfully freeed\n");
        //}
    }
}
