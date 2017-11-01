# Threads_Vector_Logical_Clocks_MPI

CS441/541 - Advanced Operating Systems
Assignment # 2
Introduction
For this assignment, you are going to implement a vector logical clocks experiment. Each node
will send messages to the other nodes; the messages will include the node’s view of “time”
as it knows it. The receiving node will then update its view of time at the other nodes from
the information received in the message. We noted that one node’s notion of time at the other
nodes improves when more messages are sent. This assignment will explore that notion.
For this assignment, each node - represented by a separate thread - will send messages to other
random nodes (i.e., the destination node will be selected randomly). The message will consist
of the sending node’s vector clock values. The receiving node will update its vector with the
information that has been received. In order to see the effect of “busy” nodes (those sending
a lot of messages) vs “non-busy” nodes, the times at which nodes send messages should be
random, but based on a distribution. One way to do this is to vary the sending times of each
node, determined by its rank. On average, the root node should send out one message per
unit time, the next node should send out 2 1 per unit time, the next node should send out 4 1 per
unit time, etc. That is, the number of messages a node sends should be related to its rank n as
1
2 n .
Your program should create at least 4 threads. Run the experiment long enough so that several
hundred messages are sent, then compare the vector clocks at each node. Comment on the
trend that you see in the accuracy of the clocks at each node.
You implemented a message-passing mechanism for pthreads in assignment 1 - you should
use that mechanism to pass the messages among threads for this assignment. You may need
to extend that capability for this assignment, to handle messages sent to multiple threads.
