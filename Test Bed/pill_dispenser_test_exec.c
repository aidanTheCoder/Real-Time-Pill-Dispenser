//
// Author: Aidan Rutherford
//
// Purpose: Implement a real-time pill dispenser with 3
// services: a loader, transaction, and dispenser. To
// simulate each service, a set of LEDs if blink is 
// a unique way. The system uses the POSIX real-time
// extension and is deployed on a Raspberry Pi
//
// Date Created: November 11, 2022
//

#include "pill_dispenser.h"

extern int selection;

int main(int argc, char* argv[])
{
	//thread handles and thread attribute declarations
	fputs("Declaring thread handles and attributes\n", stdout);
	pthread_t loader;
	pthread_t transaction;
	pthread_t dispenser;
	pthread_attr_t loader_sched_attr;
	pthread_attr_t transaction_sched_attr;
	pthread_attr_t dispenser_sched_attr;
	struct sched_param loader_param;
	struct sched_param transaction_param;
	struct sched_param dispenser_param;
	pid_t pid = getpid();

    fputs("pthread_attr_init\n", stdout);
    if (pthread_attr_init(&loader_sched_attr) 
		|| pthread_attr_init(&transaction_sched_attr)
		|| pthread_attr_init(&dispenser_sched_attr))
	{
		perror("pthread_attr_init error:");
		return EXIT_FAILURE;
	}

    fputs("pthread_attr_setinheritsched\n", stdout);
	if (pthread_attr_setinheritsched(&loader_sched_attr, 
									 PTHREAD_EXPLICIT_SCHED)
		|| pthread_attr_setinheritsched(&transaction_sched_attr,
										PTHREAD_EXPLICIT_SCHED)
		|| pthread_attr_setinheritsched(&dispenser_sched_attr,
										 PTHREAD_EXPLICIT_SCHED))
	{
		perror("pthread_attr_setinherit_sched error:");
		return EXIT_FAILURE;
	}
	
	fputs("pthread_attr_setsched_policy\n", stdout);
	if (pthread_attr_setschedpolicy(&loader_sched_attr,
									SCHED_FIFO)
		|| pthread_attr_setschedpolicy(&transaction_sched_attr,
									   SCHED_FIFO)
		|| pthread_attr_setschedpolicy(&dispenser_sched_attr,
									   SCHED_FIFO))
	{
		perror("pthread_attr_setschedpolicy error:");
		return EXIT_FAILURE;
	}
	
    fputs("setting service priorities\n", stdout);
	loader_param.sched_priority = sched_get_priority_min(SCHED_FIFO);
	transaction_param.sched_priority 
		= sched_get_priority_max(SCHED_FIFO);
	dispenser_param.sched_priority = loader_param.sched_priority * 2;
	
	fputs("sched_setscheduler\n", stdout);
	if (sched_setscheduler(pid, SCHED_FIFO, &loader_param)
		|| sched_setscheduler(pid, SCHED_FIFO, &transaction_param)
		|| sched_setscheduler(pid, SCHED_FIFO, &dispenser_param))
	{
		perror("sched_setscheduler error:");
		return EXIT_FAILURE;
	}
    
    fputs("pthread_create\n", stdout);
    if (pthread_create(&loader, NULL, loader_thread, NULL)
		|| pthread_create(&transaction, NULL, transaction_thread, NULL)
		|| pthread_create(&dispenser, NULL,  dispenser_thread, NULL))
    {
        perror("pthread_create error");
        return EXIT_FAILURE;
    }
    
    print_scheduler(pid);
	
	while(selection)
		menu();

    fputs("pthread_join\n", stdout);
    if (pthread_join(loader, NULL)
		|| pthread_join(transaction, NULL)
		|| pthread_join(dispenser, NULL))
	{
		perror("pthread_join error:");
		return EXIT_FAILURE;
	}
	
	fputs("pthread_attr_destroy\n", stdout);
	if (pthread_attr_destroy(&loader_sched_attr)
		|| pthread_attr_destroy(&transaction_sched_attr)
		|| pthread_attr_destroy(&dispenser_sched_attr))
	{
		perror("pthread_attr_destroy error:");
		return EXIT_FAILURE;
	}
	
	fputs("Test Complete\n", stdout);
	return EXIT_SUCCESS;
}