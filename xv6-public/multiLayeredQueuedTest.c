#include "types.h"
#include "stat.h"
#include "user.h"
#include "timeElem.h"
#include "param.h"

int main(){

    // set sched policy to multi layered sched
    changePolicy(MULTI_LAYERED_POLICY);
    int mark;
    int pid = fork();
    mark = setQueueLayer(1);

    for(int j=0; j<40; j++){
        if (pid != 0){
            pid = fork();
            if (pid == 0){ 
                    if(j<10){
                    mark = setQueueLayer(1);
                    }else if(j<20){
                        mark = setQueueLayer(2);
                        if(j<12){
                            setPriority(5);
                        }else if(j<14){
                            setPriority(4);
                        }else if(j<16){
                            setPriority(3);
                        }else if(j<18){
                            setPriority(2);
                        }else{
                            setPriority(1);
                        }
                    }else if(j<30){
                        mark = setQueueLayer(3);
                        if(j<22){
                            setPriority(5);
                        }else if(j<24){
                            setPriority(4);
                        }else if(j<26){
                            setPriority(3);
                        }else if(j<28){
                            setPriority(2);
                        }else{
                            setPriority(1);
                        }
                    }else{
                        mark = setQueueLayer(4);
                    }
            }
        }
    }
    int i;
    if(mark == 1)
        i = 0;
    for(i=0; i<200; i++){  
        printf(1,"%d:%d\n",getpid(),i);
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
        leyerAve(getpid());
    }
        
    
    sleep(6000);
    while (wait()!=-1){}
    exit();
}