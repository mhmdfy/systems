Systems Spring 2014
Emily Montgomery
Mohammad Al Yahaya

=======

To create our shell we first implemented the prompt, then getArgs and getWord so that we could store the parsed prompt arguments. Then we tested to make sure that it stores the correct arguments. Next we implemented execute which forks a child then executes the arguments in the child and waits in the parent. Then we addedsupport for redirection, which parses the list of argumentslooks for the symbol(s) of redirection then stores it in a
global variable. In exectute we use the global varible value
trying to open the file and perform the redirection. To
implement background processes we looked for the '&' in
the list of arguments, then store the result in a global
variable. In execute if the process is a backround process 
then the parent does not wait for the child. For escape
characters we look for escape characters and check to see
if the next value is one of the supported escape characters.
Then we applied the result to the arguments. Lastly, we
made sure to cover all of the error handling. In our tests
we used the same structure as the original test, naming ours 
ourTest and modifying make to support both test files and tested for error handling and cases that the first 
test file might have missed. The biggest problem that we had was 
getting the arguments from the prompt and separating arguments
that are valid, but missing spaces with the '&'. We were able 
to successfully take care of these cases, making it one of the 
strongest features that we have.
