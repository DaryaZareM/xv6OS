#include "types.h"
#include "stat.h"
#include "user.h"

int main(){
    int pid = fork();
     pid = fork();

    printf(1,"%d\n",pid);
    int x=0;
    int i;
    for(i=0; i<250; i++){  
        x++;
    }

    wait();
    wait();
    exit();
}