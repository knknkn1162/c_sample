#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>

void * signal_handler( void * arg );
void * worker_thread ( void * arg );

void invoke_signal(void) {
    sigset_t        ss ;
    pthread_t       pt;

    /* SIGHUP,SIGINTのブロックマスク */
    sigemptyset( &ss );
    sigaddset( &ss, SIGHUP );
    sigaddset( &ss, SIGINT );
    sigprocmask( SIG_BLOCK, &ss, NULL );

    /* SIGHUP,SIGINT処理用スレッド生成 */
    pthread_create( &pt, NULL, &signal_handler, NULL );
    pthread_detach( pt );
}

int main( int argc, char ** argv ) {
    invoke_signal();

    /* 実処理スレッド生成 */
    //pthread_create( &pt, NULL, &worker_thread, &max_cnt );
    //pthread_detach( pt );

    /* 実処理 */
    while(1) {
      char buf[256];
      fgets( buf, sizeof( buf ), stdin );
      if( memcmp( buf, "quit", 4 ) == 0 ) {
          printf("quit\n");
          break;
      }
      printf("result: %s\n", buf);
    }
    return 0;
}

void * signal_handler( void * arg ) {
    sigset_t  ss ;
    int       sig;

    ( void )arg;

    /* SIGHUP,SIGINTを待つ */
    sigemptyset( &ss );
    sigaddset( &ss, SIGHUP );
    sigaddset( &ss, SIGINT );

    while( 1 ) {
        if( sigwait( &ss, &sig )) {

            printf( "not SIGHUP,SIGINT signal!!\n" );
            continue;
        }
        switch( sig ) {
        case SIGHUP:
            printf( "signal_handler 'SIGHUP' Path!! [%d]\n", sig );
            break;

        case SIGINT:
            printf( "signal_handler 'SIGINT' Path!! [%d]\n", sig );
            break;

        default:
            break;
        }
    }
    printf( "sigwait END!!\n" );
    return NULL;
}

void * worker_thread( void * arg ) {
    int       count;
    int       max_cnt = *( int * )arg;

    /* 実処理 */
    for( count = 0; count < max_cnt; count ++ ) {
        sleep( 1 ) ;
        printf( "timer - [%d]\n", count );
    }
    return NULL;
}
