#include "types.h"
#include "stat.h"
#include "user.h"
#include "timeElem.h"
#include "param.h"

int main(){

    // set sched policy to priority sched
    changePolicy(PRIORITY_SCHEDULE_POLICY);

    int pid = fork();
    setPriority(6);
    for(int j=0; j<30; j++){
        if (pid != 0 ){
            pid = fork();
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
            }
        }
    }
    
    int i;
    for(i=0; i<250; i++){  
        printf(1,"|%d:%d|\n",getpid(),i);
    }    
    
    
    struct timeElem te;
    exitT(&te);
    int cbt=te.runningTime;
    int wt= te.waitTime;
    int tt=te.ExitTime-te.creationTime;

    sleep(1500);
    printf(1,"[%d]:  CBT: %d TT:  %d WT:  %d\n ",getpid(),cbt,tt,wt);
    sleep(1500);
    if(pid>0){
        totalTime(getpid());
    }
        
    
    sleep(6000);
    while (wait()!=-1){}
    exit();
}