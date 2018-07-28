/*
gcc tsd_once_2nd.c -o tsd_once_2nd -W -Wall -g -lpthread
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

typedef struct tsd_tag {
    char *      string;
}   tsd_t;

pthread_key_t   tsd_key;
pthread_once_t  key_once = PTHREAD_ONCE_INIT;
tsd_t * value = 0;

void   once( void );
void * func( void * arg );

int main( ) {

    pthread_t id[3];

    pthread_create( &id[0], 0, func, "thread 1");
    pthread_create( &id[1], 0, func, "thread 2");
    pthread_create( &id[2], 0, func, "thread 3");
    pthread_exit( 0 );

    return 0;
}

void once( void ) {

    printf( "initializing key\n" );
    pthread_key_create( &tsd_key, 0 );
}

void * func( void * arg ) {

    pthread_once( &key_once, once );

    value = ( tsd_t * )pthread_getspecific( tsd_key );
    if( value == 0 ) {
        fprintf( stdout, "pthread_getspecific is NULL!!\n" );
        value = ( tsd_t * )malloc( sizeof( tsd_t ));
        pthread_setspecific( tsd_key, value );
    }

    printf( "[%s] set tsd value %p\n", ( char * )arg, value );
    value -> string = ( char * )arg;
    printf( "[%s] starting...\n", value -> string );

    sleep( 2 );

    value = ( tsd_t * )pthread_getspecific( tsd_key );
    printf( "%s done...\n", value -> string );
    return 0;
}

