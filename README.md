# tcsh Shell

This is an interactive program.  
Repeatedly prints a prompt and reads the whole line from user.  
Then the line is split into its components.  
If the first component is tcsh's "set", "echo", "alias", or "exit", the program obeys the command.
Otherwise fork and exec are used to make it happen; wait is used for subprocess not terminated.
Used Hashtable to store variables and aliases.
If exit code is not zero, a warning exit code is printed.     
The program also limitedly obeys symbols  
   > < is used to redirect a program's standard input so that it comes
     from a file, e.g. a.out < inputs.txt
   > > is used to redirect a program's standard output so that it goes
     into a file, e.g. a.out > results
   > | is used to pipe the standard output of one program into the standard
     input of another, e.g. a.out | wc

gcc tcshShell.c -o tcshShell && ./tcshShell

TESTS:  
  echo helloWorld   
  ls  
  mkdir dir  
  date  
  rm -r dir  
  exit  
  set x = 0  
  set y = ppp  
  $x  
  set  
  alias  
  alias x ppp  
  x  
  alias  
  alias lm ls  
  lm  
  set x= 'o'  
  ls -al > listings  
  cat listings  
  touch test.txt  
  wc -l < test.txt  
  ls -l | wc -l  
