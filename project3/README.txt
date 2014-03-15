Emily Montgomery
Mohammad Al Yahya

To start we parsed through the input and saved the pieces into the appropriate global 
variable. We used strtok to split the parts of the name and server to verify that they 
were correcly formatted. We also used it to separate the port form the server.

Next we created the sructures for the header and the question according to to the 
specifications in the implementation notes and put them together in a structure called 
query to be used in dump_packet. 
