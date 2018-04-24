# Performance Statistics Agent.

gcc server.c libapue.c libapue.h -o server
gcc client.c libapue.c libapue.h -o client

Append to /etc/services these two lines (the port can be changed, it should not be already in use! )
processinfo 	12200/tcp # process info server
processinfo 	12200/udp # process info server

Run the server: Just start the <a href="https://github.com/iNaDeX/computer-performance-agent/">agent</a> program. 
It will fork a server program on its own.
(place server executable next to agent executable)

Start a client: 
./client -h localhost -s # for summary, connecting to server on localhost
./client -h localhost -c # for current data

NB: Code based from samples from the book "Advanced Programming in the UNIX Environment"
