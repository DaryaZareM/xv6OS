//#include<unistd.h> 
//#include<stdio.h> 
//#include<stdlib.h> 
//#include<sys/wait.h> 
#include <stddef.h>

#include "types.h"
#include "stat.h"
#include "user.h"

int main(){
    int p_id = fork();
    wait(); 
    printf(1, "Process id 1 is: %d\n",p_id);
    printf(1, "Parent id 1 is: %d\n", getParentID());

    p_id = fork();
    wait();
    printf(1, "Parent id 2 is: %d\n", getParentID());

    exit();
}