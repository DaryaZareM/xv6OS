#include "types.h"
#include "stat.h"
#include "user.h"

int main(){
    // 1
    int p_id = fork();
    wait(); 
    for(int i=0; i<1000; i++){
        printf(1, "%d: %d\n", p_id, i);
    }
    // 2
    p_id = fork();
    wait(); 
    for(int i=0; i<1000; i++){
        printf(1, "%d: %d\n", p_id, i);
    }
    // 3
    p_id = fork();
    wait(); 
    for(int i=0; i<1000; i++){
        printf(1, "%d: %d\n", p_id, i);
    }
    // 4
    p_id = fork();
    wait(); 
    for(int i=0; i<1000; i++){
        printf(1, "%d: %d\n", p_id, i);
    }
    // 5
    p_id = fork();
    wait(); 
    for(int i=0; i<1000; i++){
        printf(1, "%d: %d\n", p_id, i);
    }
    // 6
    p_id = fork();
    wait(); 
    for(int i=0; i<1000; i++){
        printf(1, "%d: %d\n", p_id, i);
    }
    // 7
    p_id = fork();
    wait(); 
    for(int i=0; i<1000; i++){
        printf(1, "%d: %d\n", p_id, i);
    }
    // 8
    p_id = fork();
    wait(); 
    for(int i=0; i<1000; i++){
        printf(1, "%d: %d\n", p_id, i);
    }
    // 9
    p_id = fork();
    wait(); 
    for(int i=0; i<1000; i++){
        printf(1, "%d: %d\n", p_id, i);
    }
    // 10
    p_id = fork();
    wait(); 
    for(int i=0; i<1000; i++){
        printf(1, "%d: %d\n", p_id, i);
    }
    struct timeElem te;
    exitT(&te);

    printf(1,"%d        %d\n ",pid,te.creationTime);
    
    exit();
}