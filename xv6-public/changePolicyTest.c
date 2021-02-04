#include "types.h"
#include "stat.h"
#include "user.h"

#include <stddef.h>

int main(int argc, char *argv[]){

    // get policy from command line
    char str[20];
    strcpy(str, argv[1]);
    int i=0, policy=0;

    // convert string to int
     while(argv[1][i]!='\0'){
          if(argv[1][i]< 48 || argv[1][i] > 57){
              printf(1,"Unable to convert it into integer.\n");
              return 0;
          }
          else{
                policy = policy *10 + (argv[1][i] - 48);
              i++;
          }
     }
    printf(1, "new schedule policy: %d\n",changePolicy(policy)) ;
    exit();
}