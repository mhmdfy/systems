Emily Montgomery
Mohammad Al Yahya

For the query:

To start we parsed through the input and saved the pieces into the appropriate global 
variable. We used strtok to split the parts of the name and server to verify that they 
were correcly formatted. We also used it to separate the port form the server.
Next we created the sructures for the header and the question according to to the 
specifications in the implementation notes and put them together in a structure called 
query to be used in dump_packet. 

For the response:

We first checked to see if there wasn't enought time for a response and then we got the 
header and checked to see that all of the paramaters of the header were the correct 
responses. Then we checked to see that there was an answer in the response and handled it
appropriately. Then we parsed the question to make sure that the question had the same 
name as the original question. Then parsed as many answers as was indicated in the header.
For each answer we got the name(by following pointer appropriately) and then printed out 
the answer based on the type.
