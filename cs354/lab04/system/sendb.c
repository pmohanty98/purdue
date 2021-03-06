/* sendb.c - sendb */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  sendb  -  pass a message to a process and start recipient if waiting
 *            using blocking
 *------------------------------------------------------------------------
 */
syscall	sendb(
	  pid32		pid,					/* ID of recipient process		*/
	  umsg32	msg						/* contents of message			*/
	)
{
	intmask	mask;						/* saved interrupt mask			*/
	struct	procent *prptr;				/* ptr to process' table entry	*/
	struct	procent *sndprptr;			/* ptr to process' table entry	*/

	mask = disable();					/* save interrupts				*/
	
	if (isbadpid(pid)) {
		restore(mask);
		return SYSERR;
	}

	prptr = &proctab[pid];				/* check pid and prstate		*/
	if (prptr->prstate == PR_FREE) {
		restore(mask);
		return SYSERR;
	}

	sndprptr = &proctab[currpid];		/* get sending process entry	*/
	if (prptr->prhasmsg) {
		sndprptr->sndmsg = msg;			/* hold message					*/
		sndprptr->sndflag = TRUE;		/* indicate message is sending	*/
		sndprptr->prstate = PR_SND;		/* put process in sending state */

		enqueue(currpid, prptr->sndqueue);	/* enqueue on process		*/
		resched();						/*   and reschedule				*/
	}
	else {
		prptr->prmsg = msg;				/* deliver message 				*/
		prptr->prhasmsg = TRUE;			/* indicate message is waiting 	*/
	}

	/* If recipient waiting or in timed-wait make it ready */

	if (prptr->prstate == PR_RECV) {
		ready(pid, RESCHED_YES);
	} else if (prptr->prstate == PR_RECTIM) {
		unsleep(pid);
		ready(pid, RESCHED_YES);
	}

	restore(mask);						/* restore interrupts 			*/
	return OK;
}
