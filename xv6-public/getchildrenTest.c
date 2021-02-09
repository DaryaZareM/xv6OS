#include "types.h"
#include "stat.h"
#include "user.h"
#include "childrenArray.h"

int main(){
    
    int n1 = fork(); 

    int n2 = fork();

    int n3 = fork();

    int n4 = fork(); 

    if (n1 > 0 && n2 > 0 && n3 > 0 && n4 > 0) { 
        
                    
        int* children=(int *)malloc(sizeof(int)*20);
        getChildren(children);
        //struct childrenArray *childarrp = malloc (sizeof (struct childrenArray));

        printf(1,"PID %d children are \n ", getpid());
        
        int i=0;
        while( children[i]>0){
            printf(1," %d/ ",children[i]);
            i=i+1;
        }
        printf(1,"\n");
                    
    }

   while(wait() != -1) { }
    /* give time to parent to reach wait clause */
    sleep(1);
    exit();

}
