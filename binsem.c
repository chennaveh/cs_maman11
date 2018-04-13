#include "binsem.h"
#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

/*****************************************************************************
  Initializes a binary semaphore.
  Parameters:
    s - pointer to the semaphore to be initialized.
    init_val - the semaphore initial value. If this parameter is 0, the
    semaphore initial value will be 0, otherwise it will be 1.
*****************************************************************************/
void binsem_init(sem_t *s, int init_val) {
	if (s == NULL) {
		perror("s is NULL");
		exit(1);
	}
	*s = (init_val == 0) ? 0 : 1 ;
}

/*****************************************************************************
  The Up() operation.
  Parameters:
    s - pointer to the semaphore to be raised.
*****************************************************************************/
void binsem_up(sem_t *s) {
	if (s == NULL) {
		perror("s is NULL");
		exit(1);
	}
	*s = 1;
}

/*****************************************************************************
  The Down() operation.
  Parameters:
    s - pointer to the semaphore to be decremented. If the semaphore value is
    0, the calling thread will wait until the semaphore is raised by another
    thread.
  Returns:
      0 - on sucess.
     -1 - on a syscall failure.
*****************************************************************************/
int binsem_down(sem_t *s) {
	if (s == NULL) {
		perror("s is NULL");
		exit(1);
	}
	while (xchg(s, 0) == 0) {
		kill(getpid(), SIGALRM);
		if (errno != 0) {
			return -1;
		}
	}
	return 0;
}
