//
//  SAT.hpp
//  SAT_SOLVER2
//
//  Created by Graham Palmer on 11/26/16.
//  Copyright © 2016 Graham Palmer. All rights reserved.
//

#ifndef SAT_h
#define SAT_h

#include <stdio.h>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <cassert>

using namespace std;

struct variable {
    vector<uint> pos_sat;
    vector<uint> neg_sat;
    unordered_map<uint, int> pos_affects;
    unordered_map<uint, int> neg_affects;
    int affected = 0;
    double pos_affect = 1;
    double neg_affect = 1;
    int score = 0;
};

struct clause {
    vector<int> satisfiers;
    int count = 0;
};

class SAT {
public:
    //Input is of the form:
    /*
     c comment
     p num_vars(4) num_clauses(5)
     2 3 0
     -1 4 3 0
     3 1 -2 0
     1 2 -3 4 0
     2 -3 0
     
     */
    //REQUIRES: variables are from 1 - num_var. There SHOULD be the number
    //          of clauses specified. If this is not the case, then the problem
    //          is automatically unsatisfiable
    //MODIFIES: variables, curr_clauses
    //EFFECTS:  Initialize variables and curr_clauses
    //          Each variable has a list of the clauses its positive solves
    //          and a list of each its negation solves. Note that variables[0]
    //          is invalid because variables are 1-indexed. Curr_clauses will
    //          be used to keep track of which
    SAT(istream &is, ostream &out_stream);
    
    //If the solver has been properly initialized with a valid Boolean formula,
    //  a satisfying assignment for that formula will be returned if one exists,
    //  otherwise an empty vector will be returned.
    vector<int> solve();
    
    //check if given solution vector solves all clauses
    bool verify(const vector<int> & vec);
    
    //
    //void print_initial_ranks();
    
private:
    vector<variable> variables;
    vector<clause> clauses;
    vector<bool> curr_clauses;
    vector<uint> unit_clauses;
    vector<bool> curr_variables;
    vector<int> choices;
    unordered_set<int> chosen;
    unordered_map< vector<bool>, unordered_set< vector<bool> > > states;
    uint clause_count;
    ostream &os;
    int num_choices = 0;
    
    //Looks at the most recent element in choices and updates all necessary
    //information, including
    // 1. Setting all corresponding clauses to false in curr_clauses and
    //    reducing clause count
    // 2. Removing any instances of the complement variable from all clauses
    //    and updating unit_clauses if necessary
    // 2.5 Update scores of variables that solve same clause as complement
    // 3. Searching for all variables that also solve the same clauses and
    //    updating their scores accordingly (i.e. we remove clause 3 but both
    //    variable 2 and 5 also solved this clause and shared one other clause
    //    in common that they both solved. We divide both of their scores by 2.
    //    In general if they share more than this clause in common we divide by
    //    the number they share in common and multiply by that number minus 1
    // 4. Indicate whether or not a solution is still possible (true = solvable)
    unordered_map<int, int> update_forward();
    
    //Undo everything from above
    void update_backward();
    
    //Takes a clause index as input and adjusts scores of variables that solve this
    //clause, decreasing them appropriately.
    void update_scores_forward(uint c_idx);
    
    void update_scores_backward(uint c_idx);
    
    //use whenever variable.affect or variable.affected is modified
    void update_score(uint idx) {
        variables[idx].score = variables[idx].affected *
            (variables[idx].pos_affect + variables[idx].neg_affect);
    }
    
    //returns a vector containing the variables that solve the boolean formula
    vector<int> gather_solution();
    
    //Searches for the next variable and adds it to the stack
    void choose_next_var();
};

#endif /* SAT_h */