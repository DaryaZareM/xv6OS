#include "types.h"
#include "stat.h"
#include "user.h"
#include "timeElem.h"
int main(){
    int pid = fork();
    for(int j=0; j<30; j++){
        if (pid == 0){

            if(j<5){
            setPriority(6);
            }else if(j<10){
                setPriority(5);
            }else if(j<15){
                setPriority(4);
            }else if(j<20){
                setPriority(3);
            }else if(j<25){
                setPriority(2);
            }else{
                setPriority(1);
            }

            pid = fork();
        }
    }
    int i;
    for(i=0; i<250; i++){  
        printf(1,"%d:%d\n",pid,i);
    }    
    while (wait()!=-1){}
    
    struct timeElem te;
    exitT(&te);
    int cbt=te.runningTime;
    int wt= te.waitTime;
    int tt=te.ExitTime-te.creationTime;
    printf(1,"[%d]:  CBT: %d TT:  %d WT:  %d\n ",pid,cbt,tt,wt);

    
    exit();
}