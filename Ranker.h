//
//  ranker.h
//  Rank_SAT
//
//  Created by Graham Palmer on 5/26/17.
//  Copyright © 2017 Graham Palmer. All rights reserved.
//


//TODO: Try to reduce time complexity. May be possible to chop off branches
//using more complicated dynamic memory solutions
#ifndef ranker_h
#define ranker_h

#include <stdio.h>
#include <vector>
#include <unordered_map>
#include "infint.h"


using namespace std;

typedef unsigned int uint;

struct contributor {
    InfInt score = 0;
    unordered_map<uint, contributor> contributors;
};
struct var {
    unordered_map<uint, int> pos_affects;
    unordered_map<uint, int> neg_affects;
    unordered_map<uint, contributor> contributors;
    unordered_map<uint, var> sub_vars;
    InfInt score = 1;
    bool scored = false;
};


typedef unordered_map<uint, var> var_map;
typedef unordered_map<uint, contributor> cont_map;

typedef pair<unordered_map<uint, int>, unordered_map<uint, int>> pos_neg_affect;
typedef unordered_map<uint, pos_neg_affect> input_map;

class Ranker {
public:
    //Initialize the Ranker with a set of variables and maps of what other
    //variables they directly affect
    Ranker(const input_map & vars);
    
    //Re-initialize the Ranker with a new set
    void change_set(const input_map & vars);
    
    //Calculates the ranks of each variable and returns the one with the highest
    //rank
    uint find_best_choice();
    
    //Prints the results of the rank calculations. Requires that the ranks have
    //been calculated
    void print_results();
    
private:
    unordered_map<uint, var> Variables;
    bool calculated = false;
    
    //Calculates the scores of each variable
    void calc_scores(var_map & vars);
    //Calculates the scores of each variable based on previous scores
    void calc_scores(var_map & vars, uint removed);
    
    //Requires: updated and old are the "same" variable, but "updated"  no longer
    //considers the variable "removed"
    //Score of "updated" is updated to not include contributions of "removed"
    void update_score(var & updated, uint removed);
    
    //Score of "updated" is updated to not include contributions of "removed"
    void update_score(var & updated, const unordered_map<uint, int> & removed);
    
    //Update the contributions given the removed variable
    void update_contributions(cont_map & contributors, uint removed);
    
    void merge_contributions(cont_map &upper_cont,
                             const cont_map &lower_cont,
                             uint under_idx,
                             int multiplier);
    void merge_contributions(cont_map &upper_cont,
                             const cont_map &lower_cont,
                             int multiplier);
    
    //Returns true if score of contributor is now 0, false otherwise
    bool delete_contributions(uint removed, contributor &cont);
    
    //The premise behind this is that when we delete a contribution, it has chains
    //of other contributions it comes under and that we have to manage these
    //chains recursively. The "top" of the chain is actually the base level contributor
    //(The thing that this contributor is directly underneath). So we have to
    //basically keep a track of the path to the things we want to update
    void cross_update_cont(vector<uint> &path,
                           cont_map &update_cont,
                           const contributor &deleted_cont);
    //This works a little differently, within a contributor
    void inner_cross_update(cont_map &update_cont,
                            const contributor &deleted_cont);
    //Track down these bugs
    void validate_score(const var &curr_var, uint idx);
    
    //validate score helper
    InfInt sum_conts(const cont_map &conts);

};



#endif /* ranker_h */
