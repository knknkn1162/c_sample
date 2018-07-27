/* gcc pipe_test.c -o pipe_test -W -Wall -g -lpthread */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

typedef struct data_tag {
    char *   string;
}   data_t;


typedef struct stage_tag {
    pthread_mutex_t	mutex;
    pthread_cond_t	cond;
    int			index;
    int			data_ready;
    int			result;
    data_t *		data;
    int			( * func )( data_t * );
    pthread_t		thread;
    struct stage_tag *	next;
}   stage_t;

typedef struct pipe_tag {
    stage_t *		head;
    int			stages;
}   pipe_t;

void * pipe_thread	( void * arg );
int    func_1		( data_t * data );
int    func_2		( data_t * data );
int    func_3		( data_t * data );
int    func_4		( data_t * data );
int    func_5		( data_t * data );

#define FUNC(i) func_##i

int main(void) {
    pipe_t my_pipe;
    char   line[256];
    int i;
    stage_t* tmp;
    stage_t** stage = &(my_pipe.head);

    my_pipe.head = NULL;
    my_pipe.stages = 5;

    for(i = 0; i < 5; i++) {
      tmp = (stage_t*)malloc(sizeof(stage_t));
      // tmp->mutex = PTHREAD_MUTEX_INITIALIZER;
      pthread_mutex_init(&tmp->mutex, 0);
      // tmp->cond = PTHREAD_COND_INITIALIZER;
      pthread_cond_init(&tmp->cond, 0);

      tmp->index = i + 1;
      tmp->data_ready = 0;
      tmp->result = 0;
      tmp->data = NULL;
      tmp->thread = NULL;
      tmp->next = NULL;
      tmp->func = FUNC(1);
      *stage = tmp;
      stage = &((*stage)->next);
    }

    for(tmp = my_pipe.head; tmp; tmp = tmp->next) {
      pthread_create(&tmp->thread, 0, pipe_thread, (void*)&my_pipe);
    }

    printf( "Enter string values.\n" );

    while( 1 ) {
      printf( "Data ( string or \"quit\" ) --> " );

      fgets( line, sizeof( line ), stdin );
      if( strlen( line ) <= 1 ) {
          continue;
      }
      line[strlen( line ) - 1] = '\0';

      if( memcmp( line, "quit", 4 ) == 0 ) {
          break;
      }

      tmp = my_pipe.head;
      pthread_mutex_lock(&tmp->mutex);
      tmp->data = malloc(sizeof(data_t));
      tmp->data->string = malloc(strlen(line)+1);
      strcpy(tmp->data->string, line);
      tmp->data_ready = 1;
      pthread_cond_signal(&tmp->cond);
      pthread_mutex_unlock(&tmp->mutex);
    }

    for(tmp = my_pipe.head; tmp; tmp = tmp->next) {
      pthread_join(tmp->thread, NULL);
    }

    return 0;
}

void * pipe_thread( void * arg ) {
    pipe_t *  pipe = ( pipe_t * )arg;
    stage_t * stage = 0;

    for( stage = pipe -> head; stage; stage = stage -> next ) {
      if( stage -> thread == pthread_self( )) {
          break;
      }
    }

    while( 1 ) {
      pthread_mutex_lock( &stage->mutex );
      while( stage->data_ready == 0 ) {
          pthread_cond_wait( &stage->cond, &stage->mutex );
      }

      if( stage->data == 0 ) {
          pthread_mutex_unlock( &stage->mutex );
          break;
      }

      if( stage->result == 0 ) {
          stage->result = stage->func( stage->data );
      }

      if( stage -> next ) {
        pthread_mutex_lock( &stage -> next -> mutex );
        stage -> next -> data       = stage -> data;
        stage -> next -> result     = stage -> result;
        stage -> next -> data_ready = 1;
        pthread_cond_signal ( &stage -> next -> cond );
        pthread_mutex_unlock( &stage -> next -> mutex );
      } else {
        free( stage -> data -> string );
        free( stage -> data );
      }

      stage->data = 0;
      stage->data_ready = 0;
      stage->result = 0;
      pthread_mutex_unlock( &stage->mutex );
    }
    fprintf( stdout, "thread:%d end...\n", stage->index );
    return 0;
}


int func_1( data_t * data ) {
    printf( "func_1:[%s] start.\n", data -> string );
    sleep( 1 );
    printf( "func_1:[%s] end.\n", data -> string );
    return 0;
}

int func_2( data_t * data ) {
    printf( "func_2:[%s] start.\n", data -> string );
    sleep( 1 );
    printf( "func_2:[%s] end.\n", data -> string );
    return 0;
}

int func_3( data_t * data ) {
    printf( "func_3:[%s] start.\n", data -> string );
    sleep( 1 );
    printf( "func_3:[%s] end.\n", data -> string );
    return 0;
}

int func_4( data_t * data ) {
    printf( "func_4:[%s] start.\n", data -> string );
    sleep( 1 );
    printf( "func_4:[%s] end.\n", data -> string );
    return 0;
}

int func_5( data_t * data ) {
    printf( "func_5:[%s] start.\n", data -> string );
    sleep( 1 );
    printf( "func_5:[%s] end.\n", data -> string );
    return 0;
}
