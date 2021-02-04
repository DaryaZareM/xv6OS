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
        
                    
                struct childrenArray childarrp;
                printf(1,"PID %d children are \n ", n1);
                
                getChildren(&childarrp);

                
                int i=0;
                while( i < (childarrp.len)){
                    printf(1," %d/ ",childarrp.children[i]);
                    i=i+1;
                }
                    
    }

   while(wait() != -1) { }
    /* give time to parent to reach wait clause */
    sleep(1);
    exit();

}
