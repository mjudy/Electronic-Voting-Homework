/**
  * @Author Mark Judy (mjudy1@umbc.edu)
  * This is the evote.c file for CMSC421 Homework 2.
  * This program accepts 4 command line arguments that
  * represent candidates in a primary election, then spawns
  * child processes to track the votes for each candidate.
  * From there the program offers a number of functions
  * related to the management of votes during the election
  * as well as a function that allows a user to commit
  * voter fraud, by specifying a number of votes to add
  * for a candidate.
*/

/**
  * When I modified the program to no longer close the writing
  * end of the children's unnamed pipes the program continued
  * to function normally. The only change was that the child
  * processes did not terminate upon exiting the program.
  * This experiment showed why we need to properly close the
  * ends of unnamed pipes, because child processes cannot 
  * terminate while they maintain an open file descriptor.
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <signal.h>
#include <stdio.h>

/**
  * This struct defines the data related to a candidate
  * in the election and allows a the parent process to
  * store data about its children without unnecessary
  * communication between processes.
*/
struct candidate {
  int fd[2];
  char *name;
  int votes;
  pid_t pid;
};

void menu (int pArgc, char **pArgv, struct candidate *cAry);
void castBallot (int pArgc, char **pArgv, struct candidate *cAry);
void suspendCampaign(int pArgc, char **pArgv, struct candidate *cAry);
void showVoteCount (int pArgc, char **pArgv, struct candidate *cAry);
void commitFraud (int pArgc, char **pArgv, struct candidate *cAry);

/**
  * main() - The entry point for our evoting program.
  *
  * @argc - A count of the commandline arguments.
  * @argv - Array of commandline arguments.
  * Return: 0 on success. Nonzero on error.
*/
int main (int argc, char *argv[]) {
  struct candidate c0, c1, c2, c3;
  struct candidate cAry[4];
  pid_t pid = getpid();

  cAry[0] = c0;
  cAry[1] = c1;
  cAry[2] = c2;
  cAry[3] = c3;

  if (argc != 5) {
    perror("Invalid number of arguments");
    exit(EXIT_FAILURE);
  } else {
    int i;
    for (i = 0; i < argc - 1; i++) {
      //create unnamed pipe for candidate
      if (pipe(cAry[i].fd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
      }

      if((pid = fork()) == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
      }

      cAry[i].name = argv[i+1];
      if(pid == 0) {
        //child process
        cAry[i].pid = pid;   //store child pid
        close(cAry[i].fd[1]);     //close writing end of child
        dup2(cAry[i].fd[0], 0);   //dup reading end of child to stdin
        close(cAry[i].fd[0]);     //close old reading end of child
        if (execlp("./candidate", cAry[i].name, NULL) == -1) {
          perror("exec");
          exit(EXIT_FAILURE);
        }
        break;
      } else {
        //parent process
        close(cAry[i].fd[0]);
        cAry[i].pid = pid;   //store child pid
        printf("Candidate %s is pid %ld\n", cAry[i].name, (long)pid);
      }//end if-else
    }//end for
    while (pid != 0) {
      menu(argc, argv, cAry);
    }
    exit(EXIT_SUCCESS);
  }//end if-else
}//end main()

/**
  * menu() - Displays the main menu for interacting with
  * the e-voting program.
  *
  * @pArgc - A pointer to the number of commandline arguments
  * @pArgv - A pointer to the array of commandline arguments
  * @cAry - A pointer to the array of candidate data
*/
void menu (int pArgc, char **pArgv, struct candidate *cAry) {
  int menuOption;

  printf("Main Menu:\n\t1. Cast ballot\n\t2. Suspend Campaign\n\t3. Show vote counts\n\t4. Commit voter fraud\n\t5. End all campaigns\nChoose an option>");
  if(scanf("%d", &menuOption) == EOF) {
    perror("scanf");
    exit(EXIT_FAILURE);
  }//end if

  switch (menuOption) {
    case 1:       //cast ballot
      castBallot(pArgc, pArgv, cAry);
      break;
    case 2:       //suspend campaign
      suspendCampaign (pArgc, pArgv, cAry);
      break;
    case 3:       //show vote counts
      showVoteCount (pArgc, pArgv, cAry);
      break;
    case 4:       //commit voter fraud
      commitFraud (pArgc, pArgv, cAry);
      break;
    case 5:       //end all campaigns
      exit(EXIT_SUCCESS);
      break;
    default:
      printf("Invalid menu option.\n");
      menu(pArgc, pArgv, cAry);
  }//end switch
}//end menu()

/**
  * castBallot() - allows the user to cast votes for a given candidate.
  * 
  * @pArgc - A pointer to the number of commandline arguments
  * @pArgv - A pointer to the array of commandline arguments
  * @cAry - A pointer to the array of candidate data
*/
void castBallot(int pArgc, char **pArgv, struct candidate *cAry) {
  int ballotOption = 0;
  int voteSignal = SIGUSR1;
  int i;
  for (i = 1; i < pArgc; i++) {
    printf("%d. %s\n", i, pArgv[i]);
  }//end for

  printf("Choose a candidate>");
  if (scanf("%d", &ballotOption) == EOF) {
    perror("scanf");
    exit(EXIT_FAILURE);
  }//end if

  switch (ballotOption) {
    case 1:
      if (write(cAry[0].fd[1], &voteSignal, sizeof(int)) > 0) {
      } else {
        perror("write");
      }
      break;
    case 2:
      if (write(cAry[1].fd[1], &voteSignal, sizeof(int)) > 0) {
      } else {
        perror("write");
      }
      break;
    case 3:
      if (write(cAry[2].fd[1], &voteSignal, sizeof(int)) > 0) {
      } else {
        perror("write");
      }
      break;
    case 4:
      if (write(cAry[3].fd[1], &voteSignal, sizeof(int)) > 0) {
      } else {
        perror("write");
      }
      break;
    default:
      printf("Invalid menu option.\n");
      castBallot(pArgc, pArgv, cAry);
  }//end switch
}//end castBallot()

/**
  * suspendCampaign() - Allows the user to stop a candidates campaign
  * so that the candidate can no longer recieve votes.
  *
  * @pArgc - A pointer to the number of commandline arguments
  * @pArgv - A pointer to the array of commandline arguments
  * @cAry - A pointer to the array of candidate data
*/
void suspendCampaign (int pArgc, char **pArgv, struct candidate *cAry) {
  int option = 0;
  int status;
  int i;
  for (i = 1; i < pArgc; i++) {
    printf("%d. %s\n", i, pArgv[i]);
  }//end for

  printf("Choose a campaign to suspend>");
  if (scanf("%d", &option) == EOF) {
    perror("scanf");
    exit(EXIT_FAILURE);
  }//end if
 
  switch (option) {
    case 1:
      if (close(cAry[0].fd[1]) == 0) {
        waitpid(cAry[0].pid, &status, WNOHANG);
      } else {
        perror("close");
      }//end if-else
      break;
    case 2:
      if (close(cAry[1].fd[1]) == 0) {
        waitpid(cAry[1].pid, &status, WNOHANG);
      } else {
        perror("close");
      }//end if-else
      break;
    case 3:
      if (close(cAry[2].fd[1]) == 0) {
        waitpid(cAry[2].pid, &status, WNOHANG);
      } else {
        perror("close");
      }//end if-else
      break;
    case 4:
      if (close(cAry[3].fd[1]) == 0) {
        waitpid(cAry[3].pid, &status, WNOHANG);
      } else {
        perror("close");
      }//end if-else
      break;
    default:
      printf("Invalid menu option\n");
      suspendCampaign(pArgc, pArgv, cAry);
  }//end switch
}//end suspendCampaign()

/**
  * showVoteCount() - Displays the currently running candidates, their
  * process ID numbers, the current state of that candidates associated
  * process, and the number of votes recorded for each currently running
  * candidate.
  *
  * @pArgc - A pointer to the number of commandline arguments
  * @pArgv - A pointer to the array of commandline arguments
  * @cAry - A pointer to the array of candidate data
*/
void showVoteCount (int pArgc, char **pArgv, struct candidate *cAry) {
  char path[100];
  memset(path, '\0', 100);
  char *basepath = "/proc/";
  char cpidstr[64];
  char line[100];
  char *status;
  FILE *statFile;
  int signal = SIGUSR2;
  int i, j;
  for (i = 0; i < 4; i++) {
    strcat(path, basepath);
    snprintf(cpidstr, 64, "%ld", (long)cAry[i].pid);
    strcat(path, cpidstr);
    strcat(path, "/stat");
    statFile = fopen(path, "r");
    if(statFile == NULL) {
      perror("fopen");
      exit(EXIT_FAILURE);
    }//end if

    if (fgets (line, 100, statFile) != NULL) {
      status = strtok(line, " ");
      for(j = 0; j < 2; j++) {
        status = strtok(NULL, " ");
      }//end for
      if(strcmp(status, "S") == 0) {
        if(write(cAry[i].fd[1], &signal, sizeof(int)) < 1) {
          printf("%s: N/A\n", cAry[i].name);
        } else {
          printf("%s: pid %ld state %s\n", cAry[i].name, (long)cAry[i].pid, status);
        }//end if-else
      }//end if
    }//end for
    memset(path, '\0', 100);
  }//end for
}//end showVoteCount()

/**
  * commitFraud() - Provides the user with the ability to add a 
  * fraudulent number of votes to a specified candidates current
  * vote count.
  *
  * @pArgc - A pointer to the number of commandline arguments
  * @pArgv - A pointer to the array of commandline arguments
  * @cAry - A pointer to the array of candidate data
*/
void commitFraud (int pArgc, char **pArgv, struct candidate *cAry) {
  int option = 0;
  int fraudVotes = 0;
  int voteSignal = SIGUSR1;
  int i;
  for (i = 1; i < pArgc; i++) {
    printf("%d. %s\n", i, pArgv[i]);
  }//end for

  printf("Choose a candidate>");  
  if (scanf("%d", &option) == EOF) {
    perror("scanf");
    exit(EXIT_FAILURE);
  }//end if
  printf("Enter fraudulent votes>"); 
  if (scanf("%d", &fraudVotes) == EOF) {
    perror("scanf");
    exit(EXIT_FAILURE);
  }//end if

  switch (option) {
    case 1:
      for (i = 0; i < fraudVotes; i++) {
        if (write (cAry[0].fd[1], &voteSignal, sizeof(int)) < 1) {          
          break;
        }//end if
      }
      break;
    case 2:
      for (i = 0; i < fraudVotes; i++) {
        if (write (cAry[1].fd[1], &voteSignal, sizeof(int)) < 1) {
          break;
        }//end if
      }
      break;
    case 3:
      for (i = 0; i < fraudVotes; i++) {
        if (write (cAry[2].fd[1], &voteSignal, sizeof(int)) < 1) {
          break;
        }//end if
      }
      break;
    case 4:
      for (i = 0; i < fraudVotes; i++) {
        if (write (cAry[3].fd[1], &voteSignal, sizeof(int)) < 1) {
          break;
        }//end if
      }
      break;
    default:
      printf("Invalid menu option.");
      commitFraud(pArgc, pArgv, cAry);
  }//end switch
}//end commitFraud()
