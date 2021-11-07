    void enqueue(struct proc *rp	/* this process is now runnable */)
 {
 /* Add 'rp' to one of the queues of runnable processes.  This function is
  * responsible for inserting a process into one of the scheduling queues.
  * The mechanism is implemented here.   The actual scheduling policy is
  * defined in sched() and pick_proc().
  *
  * This function can be used x-cpu as it always uses the queues of the cpu the
  * process is assigned to.
  */
//   int q = rp->p_priority;	 		/* scheduling queue to use */
//   struct proc **rdy_head, **rdy_tail;
   
//   assert(proc_is_runnable(rp));
    
//   assert(q >= 0);
    
//   rdy_head = get_cpu_var(rp->p_cpu, run_q_head);
//   rdy_tail = get_cpu_var(rp->p_cpu, run_q_tail);
   
/********** Your enqueue modification should be below ************/
   /* Now add the process to the queue. */
   if(rp->deadline == 0)
   {
        if (!rdy_head[q]) 
        {		/* add to empty queue */
            rdy_head[q] = rdy_tail[q] = rp; 		/* create a new queue */
            rp->p_nextready = NULL;		/* mark new end */
        }
        else
        { /* add to tail of queue */
            rdy_tail[q]->p_nextready = rp;		/* chain tail of queue */
            rdy_tail[q] = rp;				/* set new queue tail */
            rp->p_nextready = NULL;		/* mark new end */
        }
   }
   else{ // sorting deadlines that are greater than zero:
        
       if (!rdy_head[q]) 
        {/* add to empty queue */
            rdy_head[q] = rdy_tail[q] = rp; 		/* create a new queue */
            rp->p_nextready = NULL;		/* mark new end */
        }
        else if (rdy_head[q]->p_nextready == NULL)
        {
            rdy_tail[q]->p_nextready = rp;		/* chain tail of queue */
            rdy_tail[q] = rp;				/* set new queue tail */
            rp->p_nextready = NULL;		/* mark new end */
        } 
        else
        {
            struct proc *current_node = rdy_head[q];
            while( (current_node->p_nextready!= NULL)  && (current_node->p_nextready->deadline <= rp->deadline) && (current_node->p_nextready->deadline!=0))
            {
                current_node = current_node->p_nextready; /* iterate */ 
            }
            rp->p_nextready = current_node->p_nextready; /* mark previous deadlines as the end. */ 
            current_node->p_nextready = rp; /* mark previous deadlines as the end. */
        }
   }

 /********* Your enqueue modification should be above ************/
//   if (cpuid == rp->p_cpu) {
//  	  /*
//  	   * enqueueing a process with a higher priority than the current one,
//  	   * it gets preempted. The current process must be preemptible. Testing
//  	   * the priority also makes sure that a process does not preempt itself
//  	   */
//  	  struct proc * p;
//  	  p = get_cpulocal_var(proc_ptr);
//  	  assert(p);
//  	  if((p->p_priority > rp->p_priority) &&
//  			  (priv(p)->s_flags & PREEMPTIBLE))
//  		  RTS_SET(p, RTS_PREEMPTED); /* calls dequeue() */
//   }
//  #ifdef CONFIG_SMP
//   /*
//     * if the process was enqueued on a different cpu and the cpu is idle, i.e.
//     * the time is off, we need to wake up that cpu and let it schedule this new
//     * process
//     */
//   else if (get_cpu_var(rp->p_cpu, cpu_is_idle)) {
//  	  smp_schedule(rp->p_cpu);
//   }
//  #endif
 
//   /* Make note of when this process was added to queue */
//   read_tsc_64(&(get_cpulocal_var(proc_ptr)->p_accounting.enter_queue));
 
 
//  #if DEBUG_SANITYCHECKS
//   assert(runqueues_ok_local());
//  #endif
 }
