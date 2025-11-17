ims190003
Isabella Sobey

This file has two routines: one for the teller threads and one for the customer threads. The main function initializes the semaphores and creates the threads for the corresponding number of customers and tellers

The teller and customer routines communicate via semaphores and for each step the teller routine assigns text to a buffer and calls the helper logumentation which uses that to print out a log documenting each step. The end of the teller routine checks that the teller is set to idle (-1) to catch the error where teller 1 wouldn't ever shut down because it was never properly deassigned.
