### Command and Control
This simple client-server model program has a control server manage up to 5 agents, each IDed by their ip addresses. The command denies access to unauthorized agents, and provides information to authorized agents (list of active agents, log of agent activities).  A real-time log is maintained by the server, recording each agent's request and/or fulfillment.  Server and agents must have unique ip addresses, unless using only one agent (127.0.0.1).

List of Actions
- 

### How to Run	
Server
	<executable, portNumber>
Agent
	<executable, serverName, portNumber, action>
Example:  
	./server 8080
	./agent 42.42.42.42, 8080, JOIN

FUNCTION




DESIGN
	
A structure, Agent, shapes the individual agent with variables
	ipaddr and start (which monitors the number of seconds the agent
	has been active.)
	

The Agent structures are held in an array (max 5).
 When an agent is "added", an empty Agent in the array has its
 ip address filled out.
 When an agent is "removed", the agent simply has its ip address
	erased.



WHO
	
Anne Leach
	
Networking 4310.01
	
Professor Guirguis
	
12/01/2019
