### Command and Control
This simple client-server model program has a control server manage up to 5 agents, each IDed by their ip addresses. The command denies access to unauthorized agents, and provides information to authorized agents (list of active agents, log of agent activities).  A real-time log is maintained by the server, recording each agent's request and/or fulfillment.  Server and agents must have unique ip addresses, unless using only one agent (127.0.0.1).

### List of Actions
- JOIN: request to engage with server.  If granted, server adds ip address to list of active agents.
- LEAVE: request to end engaging with server.  If granted, server removes ip address from list of active agents.
- LIST: request to receive list of active agents.  Only granted to active agents.
- LOG: request of server's log.  Only granted to active agents.

### How to Run	
Server
	<executable, portNumber>
	./server 8080
Agent
	<executable, serverName, portNumber, action>
	./agent 42.42.42.42, 8080, JOIN

### Meta
- Author: Anne Leach
- Instructor: Dr. Mina Guirguis
- Course: Computer Networking
- Date: Fall 2018
