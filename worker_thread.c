/* gcc worker_test.c -o worker_test -W -Wall -g -lpthread */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <sys/stat.h>
#include <dirent.h>

typedef struct data_tag {
    // arg
    char* string;
    struct data_tag *	next;
}   data_t;

typedef struct worker_tag {
    int			index;
    pthread_t		thread;
    struct worker_tag *	next;
}   worker_t;

typedef struct {
    int			worker_size;
    worker_t *		worker;
    data_t *		data;
    pthread_mutex_t	mutex;
    pthread_cond_t	cond;
}   root_t;

void   root_create	( root_t * root, int worker_size );
void * worker_thread	( void * arg );
void   set_new_data	( root_t * root, char * filepath, char * searcher );
void   free_data	( data_t * data );
void   read_and_set	( root_t * root, data_t * data );
void   search_string	( data_t * data, worker_t * self );
void   another_file	( char * filepath, struct stat filestat );

#define WORKER_SIZE 5

void input() {
  printf( "Data ( \"directory or file:string\" or \"quit\" ) --> ");
}

int main( int argc, char ** argv ) {
    root_t	root;
    worker_t* head = NULL;
    char	buf[256];
    int i;

    root.data = NULL;
    root.worker = NULL;
    root.worker_size = WORKER_SIZE;
    pthread_mutex_init(&(root.mutex), 0);
    pthread_cond_init(&(root.cond), 0);

    // invoke worker thread
    for(i = 0; i < root.worker_size; i++) {
      worker_t* worker = malloc(sizeof(worker_t));
      worker->index = i;
      worker->thread = 0;
      root.worker = worker;
      worker->next = head;
      pthread_create(&worker->thread, NULL, worker_thread, &root);
      // update
      head = worker;
    }

    /* for( head = root.worker; head; head = head->next ) { */
      /* fprintf(stdout, "%d, \n", head->index); */
    /* } */

    while( 1 ) {
      input();
      fgets( buf, sizeof( buf ), stdin );

      if( strlen( buf ) <= 1 ) {
          continue;
      }
      if( memcmp( buf, "quit", 4 ) == 0 ) {
          break;
      }

      pthread_mutex_lock( &root.mutex );


      //set_new_data( &root, filepath, searcher );
      data_t* data  = malloc( sizeof( data_t ));
      // input data
      data->string   = strdup(buf);

      data->next = root.data;
      root.data = data;

      pthread_cond_signal( &root.cond );
      pthread_mutex_unlock( &root.mutex );
    }

    pthread_mutex_destroy( &root.mutex );
    pthread_cond_destroy ( &root.cond );

    return 0;
}

void * worker_thread( void * arg ) {

    root_t *		root = ( root_t * )arg;
    data_t *		data;
    worker_t *		self;

    for( self = root -> worker; self; self = self -> next ) {
      if( self -> thread == pthread_self( )) {
          break;
      }
    }

    while( 1 ) {
      pthread_mutex_lock( &root -> mutex );

      while(root->data == NULL) {
        fprintf(stdout, "wait thread: %d\n", self->index);
        pthread_cond_wait( &root -> cond, &root -> mutex );
      }
      fprintf(stdout, "start thread: %d\n", self->index);
      data = root -> data;
      root -> data = data -> next;

      pthread_mutex_unlock( &root -> mutex );

      sleep(3);
      fprintf(stdout, "filepath: %s\n", data->string);
      free( data -> string );
      free( data );
    }
}

