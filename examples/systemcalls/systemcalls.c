#include "systemcalls.h"
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

/**
 * @param cmd the command to execute with system()
 * @return true if the command in @param cmd was executed
 *   successfully using the system() call, false if an error occurred,
 *   either in invocation of the system() call, or if a non-zero return
 *   value was returned by the command issued in @param cmd.
*/
bool do_system(const char *cmd)
{

/*
 * TODO  add your code here
 *  Call the system() function with the command set in the cmd
 *   and return a boolean true if the system() call completed with success
 *   or false() if it returned a failure
*/

    int val = system(cmd);
    return val == -1? false : true;
}

/**
* @param count -The numbers of variables passed to the function. The variables are command to execute.
*   followed by arguments to pass to the command
*   Since exec() does not perform path expansion, the command to execute needs
*   to be an absolute path.
* @param ... - A list of 1 or more arguments after the @param count argument.
*   The first is always the full path to the command to execute with execv()
*   The remaining arguments are a list of arguments to pass to the command in execv()
* @return true if the command @param ... with arguments @param arguments were executed successfully
*   using the execv() call, false if an error occurred, either in invocation of the
*   fork, waitpid, or execv() command, or if a non-zero return value was returned
*   by the command issued in @param arguments with the specified arguments.
*/

bool do_exec(int count, ...)
{
    bool result = false;
    va_list args;
    va_start(args, count);
    char * command[count+1];
    for(int i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;

/*
 * TODO:
 *   Execute a system command by calling fork, execv(),
 *   and wait instead of system (see LSP page 161).
 *   Use the command[0] as the full path to the command to execute
 *   (first argument to execv), and use the remaining arguments
 *   as second argument to the execv() command.
 *
*/

    printf("Entering test: %s\n", command[1]);
    int status;

    // Once we fork, 2 process will continue the execution. The child process takes over the parent process state 
    // and begin execution at line 57
    pid_t pid = fork();
    if(pid < 0)
    {
            fprintf(stderr, "Fork failed");
            return false;
    }
    else if(pid == 0) 
    {
        // It is important to remember that the child process has the same code than its parent, 
        // and we can only differentiate them by their PID. This branch of the if is never reached 
        // by the parent process, only by the child process
        printf("Child pid is %d\n", pid);
        int ret = execv(command[0], command);
        if (ret < 0)
        { 
            // It is really important to stop the child process here, otherwise you get strange behavior,
            // like the tests run a second time
            exit(1);
            return false;
        }
        // execv() returns -1 if an error occured, else never returns. So this line should never be reached 
        exit(1);
    }
    else
    {
        // After the fork, the parent process comes here and wait for the child process to finish
        wait(&status);
        result = WEXITSTATUS(status) == 0 && WIFEXITED(status);
    }

    va_end(args);
    printf("do_exec test result: %d\n", result);
    return result;
}

/**
* @param outputfile - The full path to the file to write with command output.
*   This file will be closed at completion of the function call.
* All other parameters, see do_exec above
*/
bool do_exec_redirect(const char *outputfile, int count, ...)
{
    va_list args;
    bool result = false;
    va_start(args, count);
    char * command[count+1];
    int i, status, ret = 0;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;

/*
 * TODO
 *   Call execv, but first using https://stackoverflow.com/a/13784315/1446624 as a reference,
 *   redirect standard out to a file specified by outputfile.
 *   The rest of the behaviour is same as do_exec()
 *
*/
    // The trick here is to set the file at the file descriptor ID=1. FD=1 should be the standard output,
    // so now echo will output the text there. The fact that standard output file descriptor points to the file  
    // allows to redirect the text to it.
    int fd = open(outputfile, O_WRONLY|O_TRUNC|O_CREAT, 0644);
    if (fd < 0) { perror("open"); return false; }

    pid_t pid = fork();
    if(pid < 0)
    {
            fprintf(stderr, "Fork failed");
    }
    // Code for the child process
    else if(pid == 0) 
    {
        printf("Child pid is %d\n", pid);
        if (dup2(fd, 1) < 0)
        { 
            perror("dup2");
            return false; 
        }
        close(fd);
        ret = execv(command[0], command);
        if (ret < 0)
        {
            // execv() returns -1 if an error occured, else never returns
            exit(1);
            return false;
        }
        exit(1);
    }
    // Code for the parent process
    else
    {
        wait(&status);
        result = WEXITSTATUS(status) == 0 && WIFEXITED(status);
    }


    va_end(args);

    return result;
}
