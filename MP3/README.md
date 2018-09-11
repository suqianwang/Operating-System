# Dealing with a Reluctant Data Server
In this machine problem we continue our work with the data server. Here the data server is slightly more realistic, and this in three ways:

	1. Before generating a reply to a data request, the data server has to go and look up the requested data. This causes a delay in responding to a data request. This delay is in the order of a few milliseconds.

	2. Instead of simply echoing back the content of the data request, the data server now returns some actual data. For this MP, the data returned to a data request is a number between 0 and 99. Specifically, a request of the type data Joe Smith will return a number between 0 and 99. The next request for data on Joe Smith will return another number.

	3. The data server can now support multiple request channels, which in turn are served con- currently. Whenever the client issues a request newthread the server creates a new request channel and returns the name of the new channel to the client. The client then can create the client side of the request channel and start sending requests over the new channel.

You are to improve on the client program from MP2 as follows:

	1. Have the client issue multiple requests concurrently to the data server. This is done by spawning multiple worker threads, and have each thread separately issue requests to the data server.

	2. Have the client maintain a histogram of returned values for each person. (We have three persons: Joe Smith, Jane Smith, and John Doe.) Each person has a statistics thread associ- ated, and worker threads deposit their new value with the appropriate statistics thread. The statistics thread keeps the histogram updated as new data comes in.

	3. Collect, say 10000 data values for each person. In order to keep the number of worker threads independent from the number of persons, have the data requests originate from so called request threads, one for each person. These should be deposited into a bounded bu↵er, and the worker threads consume these requests. Whenever the bu↵er is empty, the worker threads wait.
