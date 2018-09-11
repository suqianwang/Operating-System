# High Concurrency without too many Threads

In this machine problem we try to further improve the performance of the client by reducing the thread management overhead required to handle the worker threads. We do this by replacing the collection of worker threads by a single event handler: Instead of forking off a large number of worker threads and have each handle a separate request channel, in this machine problem we have a single event handler thread manage all the data communication with the data server. (The communication over the control channel is still handled by the main thread of the client.)
You are to improve on the client program from MP3 as follows:
	1. Instead of spawning multiple worker threads, and have each thread separately communicate to
the data server, spawn a single event handler thread, which handles all data request channels.
	2. (BONUS) Have the client periodically show a simple display of the histograms. This is to be implemented by first installing a timer signal handler that displays the histograms and then by periodically generating a timer signal.
