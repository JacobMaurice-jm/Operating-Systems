/*--------------------------------------------------------------------

File: dp.c
Student name(s): Jacob Maurice
Student id(s): 30018739

Description:  Double pipe program.  To pipe the output from the standard
          output to the standard input of two other processes.
          Usage:  dp  <cmd1> : <cmd2> : <cmd3>
          Output from process created with cmd1 is piped to
          processes created with cmd2 and cmd3

-------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
          
/* prototypes */
int doublePipe(char **,char **,char **);

/*--------------------------------------------------------------------

File: dp.c

Description: Main will parse the command line arguments into three arrays
         of strings one for each command to execv().
--------------------------------------------------------------------*/

int main(int argc, char *argv[])
{

   int i,j;         /*indexes into arrays */
   char *cmd1[10];  /*array for arguments of first command */
   char *cmd2[10];  /*array for arguments of second command */
   char *cmd3[10];  /*array for arguments of third command */
   if(argc == 1)
   {
     printf("Usage: dp <cmd1 arg...> : <cmd2 arg...> : <cmd3 arg....>\n");
     exit(1);
   }

   /* get the first command */
   for(i=1,j=0 ; i<argc ; i++,j++)
   {
      if(!strcmp(argv[i],":")) break; /* found first command */
      cmd1[j]=argv[i];
   }
   cmd1[j]=NULL;
   if(i == argc) /* missing : */
   {
      printf("Bad command syntax - only one command found\n");
      exit(1);
   }
   else i++;

   /* get the second command */
   for(j=0 ; i<argc ; i++,j++)
   {
      if(!strcmp(argv[i],":")) break; /* found second command */
      cmd2[j]=argv[i];
   }
   cmd2[j]=NULL;
   if(i == argc) /* missing : */
   {
      printf("Bad command syntax - only two commands found\n");
      exit(1);
   }
   else i++;

   /* get the third command */
   for(j=0 ; i<argc ; i++,j++) cmd3[j]=argv[i];
   cmd3[j]=NULL;
   if(j==0) /* no command after last : */
   {
      printf("Bad command syntax - missing third command\n");
      exit(1);
   }

   exit(doublePipe(cmd1,cmd2,cmd3));
}

/*--------------------------------------------------------------------------
  ----------------- You have to implement this function --------------------
  --------------------------------------------------------------------------
Function: doublePipe()

Description:  Starts three processes, one for each of cmd1, cmd2, and cmd3.
          The parent process will receive the output from cmd1 and
          copy the output to the other two processes.
-------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define SIZE 4096

int doublePipe(char **cmd1, char **cmd2, char **cmd3)
{
    int pipe_fd1[2]; // Pipe between cmd1 and cmd2
    int pipe_fd2[2]; // Pipe between cmd1 and cmd3
    int pipe_fd3[2];
    char buffer[SIZE];
    ssize_t bytes_read;

    if (pipe(pipe_fd1) == -1 || pipe(pipe_fd2) == -1)
    {
        perror("Pipe creation failed");
        exit(EXIT_FAILURE);
    }

    pid_t pid1, pid2, pid3;

   pid1 = fork();
   if (pid1 == -1) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid1 == 0)
    {
        // Child process (cmd1)

        // Close unnecessary pipe ends
        close(pipe_fd1[0]); // Close read end of pipe_fd1
        close(pipe_fd2[0]); // Close read end of pipe_fd2
        close(pipe_fd2[1]); // Close write end of pipe_fd2

        // Redirect standard output to write to the pipe_fd1
        dup2(pipe_fd1[1], STDOUT_FILENO);

        // Close the remaining pipe ends
        close(pipe_fd1[1]);

        // Execute cmd1
        execvp(cmd1[0], cmd1);
        perror("Execvp failed");
        exit(EXIT_FAILURE);
    }
   
    pid2 = fork();
          
    if (pid2 == -1) {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }

    if (pid2 == 0)
        {
            // Child process (cmd2)

            // Close unnecessary pipe ends
            close(pipe_fd1[1]); // Close write end of pipe_fd1
            close(pipe_fd2[0]); // Close write end of pipe_fd2

            if (pipe(pipe_fd3) == -1){
                      perror("Pipe creation failed");
                      exit(EXIT_FAILURE);
            }

            // Read from pipe_fd1 and write to cmd2
            while ((bytes_read = read(pipe_fd1[0], buffer, sizeof(buffer))) > 0)
            {
                write(pipe_fd2[1], buffer, bytes_read);
                write(pipe_fd3[1], buffer, bytes_read);    
            }     
	  // Close read write of pipe_fd3
          close(pipe_fd3[1]);
          // Redirect standard output to write to the pipe_fd3        
          dup2(pipe_fd3[0], STDIN_FILENO); 
                  
            // Execute cmd2
            execvp(cmd2[0], cmd2);
            perror("Execvp failed");
            exit(EXIT_FAILURE);
        }
        
          pid3 = fork();

          if (pid3 == -1){
                perror("Fork failed");
                exit(EXIT_FAILURE);
            }

            if (pid3 == 0)
            {
                // Child process (cmd3)

                // Close unnecessary pipe ends
                close(pipe_fd1[0]); // Close read end of pipe_fd1
                close(pipe_fd1[1]); // Close write end of pipe_fd1
                close(pipe_fd2[1]); // Close write end of pipe_fd2

                // Redirect standard input to read from pipe_fd2
                dup2(pipe_fd2[0], STDIN_FILENO);

                // Close the remaining pipe ends
                close(pipe_fd2[0]);

                // Execute cmd3
                execvp(cmd3[0], cmd3);
                perror("Execvp failed");
                exit(EXIT_FAILURE);
            }
                // Parent process

                // Close unnecessary pipe ends
                close(pipe_fd1[0]); // Close read end of pipe_fd1
                close(pipe_fd1[1]); // Close write end of pipe_fd1
                close(pipe_fd2[0]); // Close read end of pipe_fd2
                close(pipe_fd2[1]); // Close write end of pipe_fd2

                // Wait for all child processes to finish
                wait(NULL);
                wait(NULL);
                wait(NULL);

                return 0;
}

