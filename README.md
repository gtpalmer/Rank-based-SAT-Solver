# Rank-based-SAT-Solver

Overview: Takes input representing a set of clauses and determines whether or not there is a satisfying variable assignment and if so returns one such assignment. May verify whether or not assignments satisfy the given clauses as well. 

  This program takes in input in the form of a cnf file through std_in. Ex.
  
     c comment
     p cnf num_vars(4) num_clauses(5)
     2 3 0
     -1 4 3 0
     3 1 -2 0
     1 2 -3 4 0
     2 -3 0
     
  Every comment line begins with a 'c'
  When we are ready to define our set of clauses, we begin a line with 'p' and specify the type of file (right now the program only     supports cnf format). Then we define how many variables and how many clauses are present in the problem. 
  For each clause, we list out the varibles that satisfy it, separated by white space. Variables must be in the range -num_var < var < num_var, and var != 0. A 0 in the input denotes the end of a clause. There should be num_clauses 0's.
  
Layout: 
  There is a simple driver function provided with a few command line options. --help or -h should give a brief overview of how the program works. By default, the program reads in standard input and standard output. If you wish to specify a file, you may add the command line argume --input/-i <filename>. Similarly, you may use --output/-o <filename>. If your file is in another directory, simply add --location/-l <filepath>. Finally, since the program often takes a long time to execute, you may type --verbose/-v and you can somewhat follow the progress of the algorithm. Every time the tree splits into 2 branches, it will print out the updated number of branches. This is helpful because the final count for the number of branch splits shows the number of leaves, and the overall time complexity is then simply O(tree depth * number of branches). 
    
Method:
  The program uses backtracking combined with a rank-based selection heuristic to generate and search through a binary decision-tree. A variable is chosen using our heuristic, the problem is updated, and then we first search all of the subproblems genearated for the positive assignment of this variable, and then all of the subproblems for the negative assignment.
  The complexity of this algorithm is exponential - this is expected due to the NP-complete nature of the problem. To prune search space and reduce the time of execution as much as possible, attempt to choose variables that will lead to the least amount of "binary decisions" over time. A binary decision is simply when we have to search the space of both the positive and negative assignment of a variable. To avoid this, we reduce the problem in such a way as to expose "unit-clauses", or clauses that can only be satisfied by a single variable assignment. In this case, only the positive or negative assignment of a variable will be legal, and so we will not increase the amount of subproblems to check.
  I use a rank-based heuristic that essentially looks at each variable and assigns it a score based on how many clauses it solves in common with other variables. The inspiration behind this heuristic is the fact that if one variable solves many of the same clauses as another variable, then selecting these variables will result in many unit clauses. There is a bit more intracacy to this process that will be described in a future update.
  
