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
#include <sys/wait.h>

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
   char *cmd1[20];  /*array for arguments of first command */
   char *cmd2[20];  /*array for arguments of second command */
   char *cmd3[20];  /*array for arguments of third command */
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

int doublePipe(char **cmd1, char **cmd2, char **cmd3)
{
    int pipe_fd1[2]; // Pipe between cmd1 and cmd2
    int pipe_fd2[2]; // Pipe between cmd2 and cmd3

    if (pipe(pipe_fd1) == -1 || pipe(pipe_fd2) == -1)
    {
        perror("Pipe creation failed");
        exit(EXIT_FAILURE);
    }

    pid_t pid1, pid2;

    // Fork the first child process (cmd1)
    if ((pid1 = fork()) == -1)
    {
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

        // Redirect standard output to write to pipe_fd1
        dup2(pipe_fd1[1], STDOUT_FILENO);

        // Close the remaining pipe ends
        close(pipe_fd1[1]);

        // Execute cmd1
        execvp(cmd1[0], cmd1);

        // This line should not be reached unless there is an error in execvp
        perror("Execvp failed");
        exit(EXIT_FAILURE);
    }
    else
    {
        // Fork the second child process (cmd2)
        if ((pid2 = fork()) == -1)
        {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }

        if (pid2 == 0)
        {
            // Child process (cmd2)

            // Close unnecessary pipe ends
            close(pipe_fd1[1]); // Close write end of pipe_fd1
            close(pipe_fd2[0]); // Close read end of pipe_fd2

            char buffer[4096];
            ssize_t bytes_read;

            // Read from pipe_fd1 and write to pipe_fd2
            while ((bytes_read = read(pipe_fd1[0], buffer, sizeof(buffer))) > 0)
            {
                write(pipe_fd2[1], buffer, bytes_read);
            }

            // Close the remaining pipe ends
            close(pipe_fd1[0]);
            close(pipe_fd2[1]);

            exit(EXIT_SUCCESS);
        }
        else
        {
            // Fork the third child process (cmd3)
            pid_t pid3;
            if ((pid3 = fork()) == -1)
            {
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

                char buffer[4096];
                ssize_t bytes_read;

                // Read from pipe_fd2 and write to stdout
                while ((bytes_read = read(pipe_fd2[0], buffer, sizeof(buffer))) > 0)
                {
                    write(STDOUT_FILENO, buffer, bytes_read);
                }

                // Close the remaining pipe ends
                close(pipe_fd2[0]);

                exit(EXIT_SUCCESS);
            }
            else
            {
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

                // Return success
                return 0;
            }
        }
    }

    // This line should not be reached
    return 0;
}
