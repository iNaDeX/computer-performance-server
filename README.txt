gcc server.c libapue.c libapue.h -o server
gcc client.c libapue.c libapue.h -o client

append to /etc/services these two lines
cs671processinfo 	12200/tcp # homework cs671 process info server
cs671processinfo 	12200/udp # homework cs671 process info server

start agent program like in previous projects
it will fork a server program on its own.

(place server executable next to agent executable)

to start a client: 
./client -h localhost -s # for summary, connecting to server on localhost
./client -h localhost -c # for current data
