/**
  * @Author Mark Judy (mjudy1@umbc.edu)
  *
  * This is the candidate.c file for CMSC421 Homework 2.
  * It contains definitions for the signal handlers used
  * in the homework. One handler is used to increase a
  * candidate's vote count while the other is used to 
  * display the current number of votes for a candidate.
*/

#define _POSIX_SOURCE
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <errno.h>

int voteCount;
char *name;

/**
  * usr1Handler() - This signal handler increases a candidates
  * voteCount when the parent process sends the SIGUSR1 signal
  * to a child process.
  *
  *@signum - The signal number sent to the handler. The signal
  * number could be one of three numbers specified by the POSIX
  * standard.
*/
static void usr1Handler (int signum) {
  voteCount++;
}//end usr1Handler()

/**
  * usr2Handler - This signal handler displays the current
  * number of votes a candidate has when the parent process
  * sends the SIGUSR2 signal to a child process.
  *
  * @signum - The signal number sent to the handler. The signal
  * number could be one of three numbers specified by the POSIX
  * standard.
*/
static void usr2Handler (int signum) {
  printf("I am %s and I have %d vote(s).\n", name, voteCount);
}//end usr2Handler()

/**
  * main() - This is the entry point for the candidate process.
  * It takes signals from stdin and decides how to process them
  * based on the signal recieved.
  *
  * @argc - The number of arguments sent via the command line.
  * @argv - The arguments array sent via the command line.
*/
int main (int argc, char *argv[]) {
//  printf("I am being executed!");
  voteCount = 0;
  name = argv[0];
  pid_t pid = getpid();
  printf("Hi, my name is %s and I am %ld\n", name, (long)pid);
  int signal = -1;
  size_t readAmount = sizeof(int);
  ssize_t dataRead;

  sigset_t mask;
  sigemptyset(&mask);

  struct sigaction usr1 = {
    .sa_handler = usr1Handler,
    .sa_mask = mask,
    .sa_flags = 0
  };

  struct sigaction usr2 = {
    .sa_handler = usr2Handler,
    .sa_mask = mask,
    .sa_flags = 0
  };

  sigaction(SIGUSR1, &usr1, NULL);
  sigaction(SIGUSR2, &usr2, NULL);
  while((dataRead = read(0, &signal, readAmount)) > 0) {
    if (dataRead < 0) {
      if (errno == EINTR) {
        //stop whining, keep working
      } else {
        perror("read");
      }//end if-else
    } else {
      raise(signal);
    }//end if-else
  }//end while
  exit(EXIT_SUCCESS);
  
  return 0;
}
