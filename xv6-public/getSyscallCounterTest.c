#include "types.h"
#include "stat.h"
#include "user.h"

#include <stddef.h>

int main(int argc, char *argv[]){

    // call getParentID to test(with ID==22)
    printf(1, "Parent id 1 is: %d\n", getParentID());
    printf(1, "Parent id 2 is: %d\n", getParentID());

    // get id from command line
    char str[20];
    strcpy(str, argv[1]);
    printf(1," ih %s\n", str);
    int i=0, sysCallID=0;

    // convert string to int
     while(argv[1][i]!='\0'){
          if(argv[1][i]< 48 || argv[1][i] > 57){
              printf(1,"Unable to convert it into integer.\n");
              return 0;
          }
          else{
              sysCallID = sysCallID *10 + (argv[1][i] - 48);
              i++;
          }
     }
    // get output of getSyscallCounter syscall
    printf(1, "Number of times sysCall with %s has been called by this process: %d\n", argv[1], getSyscallCounter(sysCallID));
    exit();
}