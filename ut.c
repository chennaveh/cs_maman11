/*
 * Create By:  Chen Naveh
 * Student ID: 301841664
 *
 * This file define user thread API. Including:
 * ut_init() 		 - initial user thread table
 * ut_spawn_thread() - append new thread to table
 * ut_start()		 - start running thread table
 * ut_get_vtime() 	 - query cpu time for thread id
 *
 */
#include "ut.h"
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

/* The contexts. */
static ut_slot_t table[MAX_TAB_SIZE];
static ucontext_t temp_uc;
static unsigned int next_idx = 0;
static unsigned int table_size;
static volatile int currThreadNum = 0;

void handler(int signal) {
	if (signal == SIGVTALRM) {
		// update the vtime statistics
		table[currThreadNum].vtime += 10;
	} else if (signal == SIGALRM) {
		alarm(1);
		int next = (currThreadNum + 1) % table_size;
		int curr_temp = currThreadNum;
		printf("in signal handler: switching from %d to %d\n", currThreadNum, next);
		currThreadNum = next;
		swapcontext(&table[curr_temp].uc, &table[next].uc);
	}
}

int ut_init(int tab_size) {
	/* setting up table size */
	if (tab_size > MAX_TAB_SIZE || tab_size < MIN_TAB_SIZE) {
		table_size = MAX_TAB_SIZE;
	} else {
		table_size = tab_size;
	}

	struct sigaction sa;
	struct itimerval itv;

	/* Initialize the data structures for SIGALRM handling. */
	sa.sa_flags = SA_RESTART;
	sigfillset(&sa.sa_mask);
	sa.sa_handler = handler;

	/* set up vtimer for accounting */
	itv.it_interval.tv_sec = 0;
	itv.it_interval.tv_usec = 10000;
	itv.it_value = itv.it_interval;

	if (sigaction(SIGVTALRM, &sa, NULL) < 0) return SYS_ERR;
	if (setitimer(ITIMER_VIRTUAL, &itv, NULL) < 0) return SYS_ERR;
	if (sigaction(SIGALRM, &sa, NULL) < 0) return SYS_ERR;

	return 0;
}

tid_t ut_spawn_thread(void (*func)(int), int arg) {
	/* appending new thread to table */
	if (next_idx > MAX_TAB_SIZE) return TAB_FULL;
	if (getcontext(&table[next_idx].uc) == -1) return SYS_ERR;
	table[next_idx].uc.uc_link = &temp_uc;
	table[next_idx].uc.uc_stack.ss_size = STACKSIZE;
	table[next_idx].uc.uc_stack.ss_sp = malloc(STACKSIZE);
	if (!table[next_idx].uc.uc_stack.ss_sp) return SYS_ERR;
	table[next_idx].vtime = 0;
	table[next_idx].func = func;
	table[next_idx].arg = arg;

	makecontext(&table[next_idx].uc, (void(*)(void)) func, 1, arg);
	return next_idx++;
}

int ut_start(void) {
	/* start running thread table */
	alarm(1);
	swapcontext(&temp_uc, &table[currThreadNum].uc);

	return 0;
}

unsigned long ut_get_vtime(tid_t tid) {
	/* return virtual CPU time used by tid */
	return table[tid].vtime;
}
