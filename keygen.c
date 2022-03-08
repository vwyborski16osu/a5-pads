#include <stdlib.h>
#include <stdio.h>
#include <time.h>

int main(int argc, char* argv[])
{

    if (argc < 2)
    {
        printf("You must provide length of key to generate\n");
        printf("Example usage: ./movies 10\n");
        return EXIT_FAILURE;
    }
    char *p;
    long key_length = strtol(argv[1], &p, 10);
    char *new_file_path = malloc(key_length + 1);
    int new_char;
    time_t t;
    srand((unsigned) time(&t));
    for(long i=0; i < key_length; i++){
        // printf("%c\n", (char)((rand() % 27) + 65));
        new_char = (rand() % 27) + 65;
        if(new_char == 91){
            new_char = 32;
        }
        new_file_path[i] = new_char;
    }
    new_file_path[key_length] = '\0';
    printf("%s\n", new_file_path);

    return EXIT_SUCCESS;
}