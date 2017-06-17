//
//  ranker.cpp
//  Rank_SAT
//
//  Created by Graham Palmer on 5/26/17.
//  Copyright © 2017 Graham Palmer. All rights reserved.
//

#include "ranker.h"

Ranker::Ranker(const input_map & vars) {
    for (auto it = vars.begin(); it != vars.end(); it++) {
        uint idx = it->first;
        
        Variables[idx].pos_affects = it->second.first;
        Variables[idx].neg_affects = it->second.second;
    }
}

void Ranker::change_set(const input_map &vars) {
    Variables.clear();
    for (auto it = vars.begin(); it != vars.end(); it++) {
        uint idx = it->first;
        
        Variables[idx].pos_affects = it->second.first;
        Variables[idx].neg_affects = it->second.second;
    }
}

uint Ranker::find_best_choice() {
    
    calc_scores(Variables);
    
    InfInt top_score = 0;
    uint top_score_idx = 0;
    for (auto it = Variables.begin(); it != Variables.end(); it++) {
        if (it->second.score > top_score) {
            top_score = it->second.score;
            top_score_idx = it->first;
        }
    }
    calculated = true;
    return top_score_idx;
}

void Ranker::print_results() {
    
    cout << "Printing scores: " << endl;
    cout << "Format is: var_idx | First 10 digits | numDigits" << endl;
    cout << "______________________________________" << endl;
    for (auto it = Variables.begin(); it != Variables.end(); it++) {
        string num = it->second.score.toString();
        unsigned long numDigits = it->second.score.numberOfDigits();
        
        cout << it->first << " | " << num.substr(0, 10) << " | " << numDigits << "\n";
    }
}
//_______________________PRIVATE___________________________

void Ranker::calc_scores(var_map &vars) {
    for (auto it = vars.begin(); it != vars.end(); it++) {
        uint idx = it->first;
        var &curr_var = it->second;
        
        curr_var.sub_vars = vars;
        curr_var.sub_vars.erase(idx);
        //Erase idx in question from all pos and neg affects
        for (auto var_it = curr_var.sub_vars.begin(); var_it != curr_var.sub_vars.end(); var_it++) {
            var_it->second.pos_affects.erase(idx);
            var_it->second.neg_affects.erase(idx);
        }
        calc_scores(curr_var.sub_vars, idx);
        
        const unordered_map<uint, int> &pos_affects = curr_var.pos_affects;
        for (auto var_it = pos_affects.begin();
             var_it != pos_affects.end();
             var_it++) {
            auto temp = pos_affects;
            temp.erase(var_it->first);
            struct var this_var = curr_var.sub_vars[var_it->first];
            update_score(this_var, temp);
            
            InfInt score_increase = this_var.score * var_it->second;
            curr_var.score += score_increase;
            curr_var.contributors[var_it->first].score += score_increase;
            merge_contributions(curr_var.contributors,
                                this_var.contributors,
                                var_it->first,
                                var_it->second);
            
        }
        const unordered_map<uint, int> &neg_affects = curr_var.neg_affects;
        for (auto var_it = neg_affects.begin();
             var_it != neg_affects.end();
             var_it++){
            auto temp = neg_affects;
            temp.erase(var_it->first);
            struct var this_var = curr_var.sub_vars[var_it->first];
            update_score(this_var, temp);
            
            InfInt score_increase = this_var.score * var_it->second;
            curr_var.score += score_increase;
            curr_var.contributors[var_it->first].score += score_increase;
            merge_contributions(curr_var.contributors,
                                this_var.contributors,
                                var_it->first,
                                var_it->second);
        }
        curr_var.scored = true;
    }
}
void Ranker::calc_scores(var_map &vars, uint removed) {
    
    //TODO: Vectors with indexes are fastor to iterate through perhaps than maps?
    
    unordered_map<uint, bool> have_sub_vars;
    
    //Find out what we already know (scores already calculated)
    for (auto it = vars.begin(); it != vars.end(); it++) {
        uint idx = it->first;
        var &curr_var = it->second;
        
        //We have already calculated the score for this variable
        if (curr_var.scored) {
            update_score(curr_var, removed);
            
            if (!curr_var.sub_vars.empty()) {
                //Edit sub_vars.
                curr_var.sub_vars.erase(removed);
                for (auto it2 = curr_var.sub_vars.begin(); it2 != curr_var.sub_vars.end(); it2++) {
                    update_score(it2->second, removed);
                }
                have_sub_vars[idx] = true;
            }
            
        }
    }
    //Ends with a break when we detect all variables are scored
    while (true) {
        //This keeps track of the best candidates to do a full calculation on
        unordered_map<uint, uint> candidates;
        uint best_candidate_idx = 0;
        uint best_candidate_score = 0;
        
        //Search for easily calculable scores without retaining full information (sub_vars)
        for (auto it = vars.begin(); it != vars.end(); it++) {
            uint curr_var_idx = it->first;
            var &curr_var = it->second;
            
            
            if (!curr_var.scored) {
                uint pos_helper = 0;
                uint neg_helper = 0;
                //Check if var pos_affects any full info scored vars
                const unordered_map<uint, int> &pos_affects = curr_var.pos_affects;
                for (auto it2 = pos_affects.begin(); it2 != pos_affects.end(); it2++) {
                    if (have_sub_vars.find(it2->first) != have_sub_vars.end()) {
                        pos_helper = it2->first;
                        break;
                    }
                }
                //If not found, update candidates
                if (!pos_helper) {
                    for (auto it2 = pos_affects.begin(); it2 != pos_affects.end(); it2++) {
                        if (!vars[it2->first].scored) {
                            if (candidates.find(it2->first) != candidates.end()) {
                                candidates[it2->first] = 1;
                            }
                            else{
                                candidates[it2->first] += 1;
                            }
                            if (candidates[it2->first] > best_candidate_score) {
                                best_candidate_score += 1;
                                best_candidate_idx = it2->first;
                            }
                        }
                    }
                }
                //Check if var neg_affects any full info scored vars
                const unordered_map<uint, int> &neg_affects = curr_var.neg_affects;
                for (auto it2 = neg_affects.begin(); it2 != neg_affects.end(); it2++) {
                    if (have_sub_vars.find(it2->first) != have_sub_vars.end()) {
                        neg_helper = it2->first;
                        break;
                    }
                }
                //If not found, update candidates
                if (!neg_helper) {
                    for (auto it2 = neg_affects.begin(); it2 != neg_affects.end(); it2++) {
                        if (!vars[it2->first].scored)  {
                            if (candidates.find(it2->first) != candidates.end()) {
                                candidates[it2->first] = 1;
                            }
                            else{
                                candidates[it2->first] += 1;
                            }
                            if (candidates[it2->first] > best_candidate_score) {
                                best_candidate_score += 1;
                                best_candidate_idx = it2->first;
                            }
                        }
                    }
                }
                //If it both pos and neg affects full info scored vars, we can solve
                //for its score and contributors
                if (pos_helper && neg_helper) {
                    for (auto var_it = pos_affects.begin();
                         var_it != pos_affects.end();
                         var_it++) {
                        if (var_it->first == pos_helper) {
                            auto temp = pos_affects;
                            temp.erase(pos_helper);
                            //We need to also update curr_var's influence under this guy
                            temp[curr_var_idx] = 0;
                            struct var this_var = vars[pos_helper];
                            update_score(this_var, temp);
                            
                            InfInt score_increase = this_var.score * var_it->second;
                            curr_var.score += score_increase;
                            curr_var.contributors[var_it->first].score += score_increase;
                            merge_contributions(curr_var.contributors,
                                                this_var.contributors,
                                                var_it->first,
                                                var_it->second);
                        }
                        else {
                            auto temp = pos_affects;
                            temp.erase(var_it->first);
                            temp.erase(pos_helper);
                            temp[curr_var_idx] = 0;
                            struct var this_var = vars[pos_helper].sub_vars[var_it->first];
                            update_score(this_var, temp);
                            
                            InfInt score_increase = this_var.score * var_it->second;
                            curr_var.score += score_increase;
                            curr_var.contributors[var_it->first].score += score_increase;
                            merge_contributions(curr_var.contributors,
                                                this_var.contributors,
                                                var_it->first,
                                                var_it->second);
                        }
                    }
                    for (auto var_it = neg_affects.begin();
                         var_it != neg_affects.end();
                         var_it++) {
                        if (var_it->first == neg_helper) {
                            auto temp = neg_affects;
                            temp.erase(neg_helper);
                            temp[curr_var_idx] = 0;
                            struct var this_var = vars[neg_helper];
                            update_score(this_var, temp);
                            
                            InfInt score_increase = this_var.score * var_it->second;
                            curr_var.score += score_increase;
                            curr_var.contributors[var_it->first].score += score_increase;
                            merge_contributions(curr_var.contributors,
                                                this_var.contributors,
                                                var_it->first,
                                                var_it->second);
                        }
                        else {
                            auto temp = neg_affects;
                            temp.erase(var_it->first);
                            temp.erase(neg_helper);
                            temp[curr_var_idx] = 0;
                            struct var this_var = vars[neg_helper].sub_vars[var_it->first];
                            update_score(this_var, temp);
                            
                            InfInt score_increase = this_var.score * var_it->second;
                            curr_var.score += score_increase;
                            curr_var.contributors[var_it->first].score += score_increase;
                            merge_contributions(curr_var.contributors,
                                                this_var.contributors,
                                                var_it->first,
                                                var_it->second);
                        }
                    }
                    curr_var.scored = true;
                    validate_score(curr_var, curr_var_idx);
                }
                //Didn't need a neg_helper
                else if (pos_helper && neg_affects.empty()) {
                    for (auto var_it = pos_affects.begin();
                         var_it != pos_affects.end();
                         var_it++) {
                        
                        if (var_it->first == pos_helper) {
                            auto temp = pos_affects;
                            temp.erase(pos_helper);
                            temp[curr_var_idx] = 0;
                            struct var this_var = vars[pos_helper];
                            update_score(this_var, temp);
                            
                            InfInt score_increase = this_var.score * var_it->second;
                            curr_var.score += score_increase;
                            curr_var.contributors[var_it->first].score += score_increase;
                            merge_contributions(curr_var.contributors,
                                                this_var.contributors,
                                                var_it->first,
                                                var_it->second);
                        }
                        //Ouch we forgot to add the pos_helper's score too
                        else {
                            auto temp = pos_affects;
                            temp.erase(var_it->first);
                            temp.erase(pos_helper);
                            temp[curr_var_idx] = 0;
                            struct var this_var = vars[pos_helper].sub_vars[var_it->first];
                            update_score(this_var, temp);
                            
                            InfInt score_increase = this_var.score * var_it->second;
                            curr_var.score += score_increase;
                            curr_var.contributors[var_it->first].score += score_increase;
                            merge_contributions(curr_var.contributors,
                                                this_var.contributors,
                                                var_it->first,
                                                var_it->second);
                        }
                    }
                    curr_var.scored = true;
                    validate_score(curr_var, curr_var_idx);
                }
                //Didn't need a pos_helper
                else if (pos_affects.empty() && neg_helper) {
                    for (auto var_it = neg_affects.begin();
                         var_it != neg_affects.end();
                         var_it++) {
                        
                        if (var_it->first == neg_helper) {
                            auto temp = neg_affects;
                            temp.erase(neg_helper);
                            temp[curr_var_idx] = 0;
                            struct var this_var = vars[neg_helper];
                            update_score(this_var, temp);
                            
                            InfInt score_increase = this_var.score * var_it->second;
                            curr_var.score += score_increase;
                            curr_var.contributors[var_it->first].score += score_increase;
                            merge_contributions(curr_var.contributors,
                                                this_var.contributors,
                                                var_it->first,
                                                var_it->second);
                        }
                        else {
                            auto temp = neg_affects;
                            temp.erase(var_it->first);
                            temp.erase(neg_helper);
                            temp[curr_var_idx] = 0;
                            struct var this_var = vars[neg_helper].sub_vars[var_it->first];
                            update_score(this_var, temp);
                            
                            InfInt score_increase = this_var.score * var_it->second;
                            curr_var.score += score_increase;
                            curr_var.contributors[var_it->first].score += score_increase;
                            merge_contributions(curr_var.contributors,
                                                this_var.contributors,
                                                var_it->first,
                                                var_it->second);
                        }
                    }
                    curr_var.scored = true;
                    validate_score(curr_var, curr_var_idx);
                }
                //Score defaults to 1. Show that is has been scored
                else if (pos_affects.empty() && neg_affects.empty()) {
                    curr_var.scored = true;
                    if (curr_var_idx == 6) {
                        
                    }
                    validate_score(curr_var, curr_var_idx);
                }
            } //Endif - search for and/or calculation of scores of variables
            //That affect already scored vars with full info
        }
        //Everything must have been scored, or all candidates were scored
        if (!best_candidate_idx) {
            //Check for anything that hasn't been scored
            for (auto it = vars.begin(); it != vars.end(); it++) {
                if (!it->second.scored) {
                    best_candidate_idx = it->first;
                    break;
                }
            }
            //Now if we still haven't found anything, we are done
            if (!best_candidate_idx) {
                break;
            }
        }
        //Check that somehow an already scored var is the candidate (possible but unlikely)
        else if (vars[best_candidate_idx].scored) {
            //Find a new best candidate
            best_candidate_idx = 0;
            best_candidate_score = 0;
            for (auto it = candidates.begin(); it != candidates.end(); it++) {
                if (it->second > best_candidate_score && !vars[it->first].scored) {
                    best_candidate_score = it->second;
                    best_candidate_idx = it->first;
                }
            }
            //There MUST be unscored variables, but they only affect scored
            //variables. We just find the first one
            for (auto it = vars.begin(); it != vars.end(); it++) {
                if (!it->second.scored) {
                    best_candidate_idx = it->first;
                    break;
                }
            }
        }
        
        //_______________Do full calculation_________________________________
        uint idx = best_candidate_idx;
        var & curr_var = vars[best_candidate_idx];
        curr_var.sub_vars = vars;
        curr_var.sub_vars.erase(idx);
        
        //Erase idx in question from all pos and neg affects
        //TODO: also remove idx from their contributions?
        for (auto var_it = curr_var.sub_vars.begin(); var_it != curr_var.sub_vars.end(); var_it++) {
            var_it->second.pos_affects.erase(idx);
            var_it->second.neg_affects.erase(idx);
        }
        calc_scores(curr_var.sub_vars, idx);
        
        const unordered_map<uint, int> &pos_affects = curr_var.pos_affects;
        for (auto var_it = pos_affects.begin();
             var_it != pos_affects.end();
             var_it++) {
            auto temp = pos_affects;
            temp.erase(var_it->first);
            struct var this_var = curr_var.sub_vars[var_it->first];
            if (var_it->first == 6) {
                
            }
            update_score(this_var, temp);
            validate_score(this_var, var_it->first);
            
            InfInt score_increase = this_var.score * var_it->second;
            curr_var.score += score_increase;
            curr_var.contributors[var_it->first].score += score_increase;
            merge_contributions(curr_var.contributors,
                                this_var.contributors,
                                var_it->first,
                                var_it->second);
        }
        const unordered_map<uint, int> &neg_affects = curr_var.neg_affects;
        for (auto var_it = neg_affects.begin();
             var_it != neg_affects.end();
             var_it++){
            auto temp = neg_affects;
            temp.erase(var_it->first);
            struct var this_var = curr_var.sub_vars[var_it->first];

            update_score(this_var, temp);
            validate_score(this_var, var_it->first);
            
            InfInt score_increase = this_var.score * var_it->second;
            curr_var.score += score_increase;
            curr_var.contributors[var_it->first].score += score_increase;
            merge_contributions(curr_var.contributors,
                                this_var.contributors,
                                var_it->first,
                                var_it->second);
        }
        curr_var.scored = true;
        validate_score(curr_var, idx);
    }
    
    //remove subscores.
    for (auto it = vars.begin(); it != vars.end(); it++) {
        it->second.sub_vars.clear();
    }
    cout << "Level " << vars.size() << "Completed " << endl;
}

void Ranker::update_score(var &updated, uint removed) {
 
    const contributor &cont = updated.contributors[removed];
    updated.score -= cont.score;
    if (updated.score < 1) {
        
    }
    //Remove this variables influence under other variables' contributions
    for (auto it = cont.contributors.begin(); it != cont.contributors.end(); it++) {
        
        vector<uint> path;
        path.push_back(it->first);
        cross_update_cont(path, updated.contributors, it->second);
    }
    
    //delete this contribution
    updated.contributors.erase(removed);
    
    //Now search through other variables and remove their contributions under this
    //variable. Note the vector is necessary to prevent the iterator from breaking
    vector<uint> erasable;
    for (auto it = updated.contributors.begin(); it != updated.contributors.end(); it++) {
        if (it->second.contributors.find(removed) != it->second.contributors.end()) {
            if (delete_contributions(removed, it->second)) {
                erasable.push_back(it->first);
            }
        }
    }
    for (uint i = 0; i < erasable.size(); i++) {
        updated.contributors.erase(erasable[i]);
    }
}

void Ranker::update_score(var &updated, const unordered_map<uint, int> &removed) {
    for (auto it = removed.begin(); it != removed.end(); it++) {
        update_score(updated, it->first);
        validate_score(updated, it->first);
        
    }
}

//___________________MERGE CONTRIBUTIONS_______________________

void Ranker::merge_contributions(cont_map &upper_cont,
                         const cont_map &lower_cont,
                         uint under_idx,
                         int multiplier) {
    for (auto it = lower_cont.begin(); it != lower_cont.end(); it++) {
        InfInt score_increase = it->second.score * multiplier;
        upper_cont[it->first].score += score_increase;
        upper_cont[it->first].contributors[under_idx].score += score_increase;
        merge_contributions(upper_cont[it->first].contributors,
                            it->second.contributors,
                            multiplier);
        merge_contributions(upper_cont[it->first].contributors[under_idx].contributors,
                            it->second.contributors,
                            multiplier);
        
    }
}
void Ranker::merge_contributions(cont_map &upper_cont,
                                 const cont_map &lower_cont,
                                 int multiplier) {
    for (auto it = lower_cont.begin(); it != lower_cont.end(); it++) {
        InfInt score_increase = it->second.score * multiplier;
        upper_cont[it->first].score += score_increase;
        merge_contributions(upper_cont[it->first].contributors,
                            it->second.contributors,
                            multiplier);
    }
}

//_______________________DELETE CONTRIBUTIONS_________________________

bool Ranker::delete_contributions(uint removed, contributor &cont) {
    
    const contributor &removed_cont = cont.contributors[removed];
    cont.score -= removed_cont.score;
    
    //If our score is 0 at this point we should be set
    if (cont.score == 0) {
        return true;
    }
    else if (cont.score < 0) {
        cerr << "Error: Cannot have a negative score" << endl;
        exit(1);
    }
    //Remove contributors influence under other contributors
    inner_cross_update(cont.contributors, removed_cont);
    
    cont.contributors.erase(removed);
    //Now loop through other contributors and repeat the process
    vector<uint> erasable;
    for (auto it = cont.contributors.begin(); it != cont.contributors.end(); it++) {
        if (it->second.contributors.find(removed) != it->second.contributors.end()) {
            if (delete_contributions(removed, it->second)) {
                //Keep track of erasable things - don't erase yet because
                //it breaks the iterator
                erasable.push_back(it->first);
            }
        }
    }
    for (uint i = 0; i < erasable.size(); i++) {
        cont.contributors.erase(erasable[i]);
    }
    return false;
}

//_______________________________CROSS UPDATE___________________________________

void Ranker::cross_update_cont(vector<uint> &path,
                               cont_map &update_cont,
                               const contributor &deleted_cont) {
    contributor * to_update = &update_cont[path.back()];
    contributor * temp = to_update;
    for (uint i = 0; i < path.size() - 1; i++) {
        temp = to_update;
        to_update = &to_update->contributors[path[i]];
    }
    to_update->score -= deleted_cont.score;
    //Check to see if we can remove this contributor - if it still has its own
    //contributors, that means that said contributors will
    if (to_update->score < 0) {
        cerr << "ERROR in 'cross_update_cont': contributor score is now negative"
        << endl;
        exit(1);
    }
    if (to_update->score == 0) {
        //If path size is 1 do something else
        if (path.size() == 1) {
            update_cont.erase(path.front());
        }
        else {
            temp->contributors.erase(path.front());
        }
    }
    for (auto it = deleted_cont.contributors.begin();
         it != deleted_cont.contributors.end();
         it++) {
        path.push_back(it->first);
        cross_update_cont(path, update_cont, it->second);
        path.pop_back();
    }
}

//______________________________INNER CROSS UPDATE______________________________
void Ranker::inner_cross_update(cont_map &update_cont,
                                const contributor &deleted_cont) {
    for (auto it = deleted_cont.contributors.begin();
         it != deleted_cont.contributors.end();
         it++) {
        update_cont[it->first].score -= it->second.score;
        if (update_cont[it->first].score < 0) {
            cerr << "ERROR: contributor can not have negative score\n"
            << "Location: inner_cross_update" << endl;
            exit(1);
        }
        //Remove this contributor. Even if it has more contributors there should
        //be no need to continue because these should all be reduced to zero as well
        if (update_cont[it->first].score == 0) {
            update_cont.erase(it->first);
        }
        else {
            inner_cross_update(update_cont[it->first].contributors, it->second);
        }
        
    }
}

//________________________________VALIDATION____________________________________

void Ranker::validate_score(const var &curr_var, uint idx) {
    InfInt comp_score = sum_conts(curr_var.contributors) + 1;
    if (comp_score != curr_var.score) {
        cerr << "Score and/or contribution calculations went wrong!" << endl;
        exit(1);
    }
}

InfInt Ranker::sum_conts(const cont_map &conts) {
    InfInt score = 0;
    for (auto it = conts.begin(); it != conts.end(); it++) {
        score += it->second.score;
        score -= sum_conts(it->second.contributors);
    }
    return score;
}

