#include"headers.h"
//structure for thread input and output
struct arg{
    int n;
    int p;
    int t;
    long u;//result
};
void * calculate(void *arg){
    //start_routine function
    struct arg *a = arg;//arg will be assigned to the address pointed to by a
    a->u=0;
    for(int i=1; i<=a->p; i++)
        a->u+= ((a->t-1)*a->p+i);
    printf("u_%d = %ld\n", a->t, a->u);
    pthread_exit(arg);//arg will be copied to the location that pthread_join:retval points to
}
int main(int argc, char *argv[]){
    int n, p;
    long sum;
    struct arg *a;
    if(argc!=3){
        fprintf(stderr, "usage: %s n p\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    n = atoi(argv[1]);
    p = atoi(argv[2]);
    pthread_t tabtid[n];
    //main thread starts
    for(int i=0; i<n; i++){
        a = malloc(sizeof a);
        a->n = n;
        a->p = p;
        a->t = i+1;
        if(pthread_create(&tabtid[i], NULL, &calculate, a)!=0){
            perror("pthread_create");
            exit(1);
        }
    }
    //waiting for termination
    sum = 0;
    for(int i=0; i<n; i++){
        struct arg *a;
        if(pthread_join(tabtid[i], (void**) &a)==-1){
            perror("pthread_join");
            exit(1);
        }
        sum+=a->u;
        free(a);
    }
    printf("sum = %ld\n", sum);

    return 0;
}