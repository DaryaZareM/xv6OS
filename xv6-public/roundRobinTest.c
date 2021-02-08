#include "types.h"
#include "stat.h"
#include "user.h"
#include "timeElem.h"
#include "param.h"

int main(){
    // set sched policy to round robin sched
    changePolicy(RR_SCHEDULE_POLICY);

    int pid = fork();
    for(int j=0; j<10; j++){
        if (pid == 0){
            pid = fork();
        }
    }
    
    int i;
    for(i=0; i<1000; i++){  
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