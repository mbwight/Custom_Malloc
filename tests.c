#include "allocator.c"

/**
 * Add your allocator test cases here. You can run 'make test' to compile
 * everything and run this mini program.
 */
int main(void)
{
    
    // int *my_num = malloc(sizeof(int));
    
    // printf("My num: %d\n", *my_num);

    
    // char *str = malloc(25);
    // strcpy(str, "Hooray for malloc!");

    // char *big = malloc(4900);
    // strcpy(big, "Whoa, big allocation!");

    // malloc(1);
    

    // print_memory();

    // return 0;
    void *a1 = malloc(100);  
    void *b2 = malloc(200);  
    void *c3 = malloc(80);   
    void *d4 = malloc(450);  
    void *e5 = malloc(25);   
    void *f6 = malloc(4000); 
                             
    free(a1);                
    free(c3);                
    free(e5);                
    free(f6);                
                             
    void *g7 = malloc(80);   
    void *h8 = malloc(25);   
    void *i9 = malloc(30);   
    void *j10 = malloc(7);   
                             
    void *k11 = malloc(6000);
    free(k11);               
    void *l12 = malloc(6000);
    void *z13 = malloc_description(3000, "test");                         
    print_memory();          

    void *lookup = malloc_lookup("test");
    print_block((struct mem_block*) lookup-1);
    return 0;        
}
