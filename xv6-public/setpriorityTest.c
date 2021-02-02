#include "types.h"
#include "stat.h"
#include "user.h"
int main(){

    int pid = fork();
    wait();
    if (pid == 0){
        //child
        printf(1,"child");
        setPriority(4);
    }
    else{
        //parent
        printf(1,"parent");
        setPriority(10);

    }


    exit();
}