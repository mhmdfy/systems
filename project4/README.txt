Emily Montgomery
Mohammad Al Yahya

Project 4
------------------------

Send:
The sender was really complicated to figure out.
When we first started of, we implemented a buffer to save what we read from stdin in case
we need to retransmit. We have a pointer for what is sent, what is sent and acked, and what one
for fin (to know where the data ends).
After that, we started implementing a static window of 10. We send 10 packets, wait for 10 acks,
repeat. That got us through the basic tests, but not further than that.
Things then got more complicated. Dealing with dropped packets was almost impossibe, though
reordering and duplication was working fine.
After getting some help from Professor Mislove, we reimplemented it again to be acting on each
ack received (instead of waiting for all the acks at once). To do that, we created another array,
which has all the sequences and timestamps for packets sent. Using the timestamp, we figure out
the timeout that we need to wait for for that packet. If a timeout occurrs, we resend packets 
from the array that have their timeout expire. When we receive and ack for the eof, we terminate.
For efficiency, we had to have a sliding window that doubles in size each roundtrip. After we
get a timeout, we stop increasing the window. The roundtrip timeout average is also computed
each time we receive an ack, in order to increase efficiency even further.

Receive:
For the receiver we tried to go with a simple implementation.
It basically just waits for packets, once it receives something it has multiple cases:
1- The packet is the one it is expecting, in this case it saves it into the main buffer.
If the main buffer was full, it writes everything to stdout. After it does that, it sends
and ack with the next expected packet.

2- The packet sequence is more than it is expecting. In this case it adds it to the buffer,
while adding the header into OutOfOrder (OOO) array. If we receive the missing packet, we go
through this array and figure out that we have this packet too, so the ack should be the one after
this packet.

3- The packet sequence is less than it is expecting. It does nothing with it, and sends an ack
for the packet that it is expecting.

4- If the packet is and eof, and in order (everything else was received already), it sends
and ack for eof, and then writes the buffer to stdout, and terminates.

5- Corrupted packets are ignored.

6- When it times out, it writes the buffer to stdout and then terminates.
