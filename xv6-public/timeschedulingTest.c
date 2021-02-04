#include "types.h"
#include "stat.h"
#include "user.h"
#include "timeElem.h"
int main(){
    int pid = fork();
    for(int j=0; j<30; j++){
        if (pid == 0){
            pid = fork();
        }
    }

    if(pid<10){
        setPriority(6);
    }else if(pid<15){
        setPriority(5);
    }else if(pid<20){
        setPriority(4);
    }else if(pid<25){
        setPriority(3);
    }else if(pid<30){
        setPriority(2);
    }else{
        setPriority(1);
    }
    
    int i;
    for(i=0; i<250; i++){  
       // printf(1,"%d:%d\n",pid,i);
    }
    wait();
    wait();
    wait();
    wait();
    wait();
    struct timeElem te;
    exitT(&te);

    printf(1,"%d        %d\n ",pid,te.creationTime);

    
    exit();
}