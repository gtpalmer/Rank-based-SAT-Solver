//
//  SAT.cpp
//  SAT_SOLVER2
//
//  Created by Graham Palmer on 11/26/16.
//  Copyright © 2016 Graham Palmer. All rights reserved.
//

#include "SAT.h"

using namespace std;

SAT::SAT(istream &is) {
    string input;
    while (is >> input) {
        if (input[0] == 'c') {
            getline(is, input);
        }
        else if (input[0] == 'p') {
            break;
        }
        else {
            cerr << "ERROR: Invalid input file detected" << endl;
            exit(1);
        }
    }
    
    string input_type;
    uint num_vars;
    uint num_clauses;
    
    clause_count = num_clauses;
    
    is >> input_type;
    assert(input_type == "cnf");
    is >> num_vars;
    is >> num_clauses;
    
    variables.resize(num_vars + 1); //1 indexed so member 0 should not be used
    curr_variables.resize(num_vars + 1, true);
    curr_variables[0] = false;
    
    uint curr_clause = 0;
    int var;
    while (is >> var && curr_clause < num_clauses) {
        assert(var >= (0 - static_cast<int>(num_vars)) && var <= static_cast<int>(num_vars));
        if (var > 0) {
            variables[static_cast<uint>(var)].pos_sat.push_back(curr_clause);
            clauses[curr_clause].satisfiers.push_back(var);
        }
        else if (var < 0) {
            variables[static_cast<uint>(var * -1)].neg_sat.push_back(curr_clause);
            clauses[curr_clause].satisfiers.push_back(var);
        }
        else {
            curr_clause++;
        }
    }
    //start with all clauses
    curr_clauses.resize(clauses.size(), true);
    
    //find starting unit clauses
    for (uint i = 0; i < clauses.size(); i++) {
        if (clauses[i].satisfiers.size() == 1) {
            unit_clauses.push_back(i);
        }
    }
    
    //Determine interconnections to other variables
    for (uint i = 1; i < variables.size(); i++) {
        for (uint j = 1; j < i; j++) {
            int count = 0;
            for (uint k = 0; k < variables[i].pos_sat.size(); i++) {
                for (uint l = 0; l < variables[j].pos_sat.size(); l++) {
                    if (variables[i].pos_sat[k] == variables[j].pos_sat[l]) {
                        count++;
                    }
                }
                for (uint l = 0; l < variables[j].neg_sat.size(); l++) {
                    if (variables[i].pos_sat[k] == variables[j].neg_sat[l]) {
                        count++;
                    }
                }
            }
            if (count > 0) {
                variables[i].pos_affects[j] = count;
                count = 0;
            }
            for (uint k = 0; k < variables[i].neg_sat.size(); i++) {
                for (uint l = 0; l < variables[j].pos_sat.size(); l++) {
                    if (variables[i].neg_sat[k] == variables[j].pos_sat[l]) {
                        count++;
                    }
                }
                for (uint l = 0; l < variables[j].neg_sat.size(); l++) {
                    if (variables[i].neg_sat[k] == variables[j].neg_sat[l]) {
                        count++;
                    }
                }
            }
            if (count > 0) {
                variables[i].neg_affects[j] = count;
            }
        }
        for (uint j = i + 1; j < variables.size(); j++) {
            int count = 0;
            for (uint k = 0; k < variables[i].pos_sat.size(); i++) {
                for (uint l = 0; l < variables[j].pos_sat.size(); l++) {
                    if (variables[i].pos_sat[k] == variables[j].pos_sat[l]) {
                        count++;
                    }
                }
                for (uint l = 0; l < variables[j].neg_sat.size(); l++) {
                    if (variables[i].pos_sat[k] == variables[j].neg_sat[l]) {
                        count++;
                    }
                }
            }
            if (count > 0) {
                variables[i].pos_affects[j] = count;
                count = 0;
            }
            for (uint k = 0; k < variables[i].neg_sat.size(); i++) {
                for (uint l = 0; l < variables[j].pos_sat.size(); l++) {
                    if (variables[i].neg_sat[k] == variables[j].pos_sat[l]) {
                        count++;
                    }
                }
                for (uint l = 0; l < variables[j].neg_sat.size(); l++) {
                    if (variables[i].neg_sat[k] == variables[j].neg_sat[l]) {
                        count++;
                    }
                }
            }
            if (count > 0) {
                variables[i].neg_affects[j] = count;
            }
        }
        //compute initial scores
        for (uint j = 0; j < variables[i].pos_affects.size(); j++) {
            variables[i].affect *= variables[i].pos_affects[j];
        }
        for (uint j = 0; j < variables[i].neg_affects.size(); j++) {
            variables[i].affect *= variables[i].neg_affects[j];
        }
    }
    
    if (curr_clause != num_clauses) {
        cout << "WARNING: Wrong number of clauses detected. Program might "
        << "falsely detect no solution. Num clauses detected: "
        << curr_clause << " compared to desired " << num_clauses << endl;
    }
    
    cout << "INIT SUCCESSFUL!" << endl;
}


vector<int> SAT::solve() {
    
    //Eliminate unit clauses and update problem
    while (!unit_clauses.empty()) {
        int satisfier = 0;
        for (uint i = 0; i < clauses[unit_clauses.back()].satisfiers.size(); i++) {
            uint idx = static_cast<uint>(abs(clauses[unit_clauses.back()].satisfiers[i]));
            if (curr_variables[idx]) {
                satisfier = clauses[unit_clauses.back()].satisfiers[i];
            }
        }
        assert(satisfier != 0);
        choices.push_back(satisfier);
        unit_clauses.pop_back();
        curr_variables[static_cast<uint>(abs(satisfier))] = false;
        update_forward();
    }
    
    if (clause_count == 0) {
        return gather_solution();
    }
    
    //Select starting choice
    int best_var = 1;
    int best_var_score = variables[1].affect;
    for (uint i = 2; i < variables.size(); i++) {
        if (variables[i].affect > best_var_score) {
            best_var_score = variables[i].affect;
            best_var = i;
        }
    }
    
    choices.push_back(best_var);
    choices.push_back(-best_var);
    int count = 1;
    while (!choices.empty()) {
        if (update_forward()) {
            choose_next_var();
        }
        else {
            int temp = choices.back();
            choices.pop_back();
            while (!choices.empty() && temp != choices.back()) {
                temp = choices.back();
                choices.pop_back();
            }
        }
        
    }
    return {};
}

bool SAT::update_forward() {
    uint idx = static_cast<uint>(abs(choices.back()));
    bool constraint_broken = false;
    if (choices.back() > 0) {
        //mark false all solved clauses and
        for (uint i = 0; i < variables[idx].pos_sat.size(); i++) {
            if (curr_clauses[variables[idx].pos_sat[i]]) {
                clause_count--;
                curr_clauses[variables[idx].pos_sat[i]] = false;
            }
            
            //find all variables that solve the same clause and update scores if
            //necessary
            const uint c_idx = variables[idx].pos_sat[i];
            update_scores_forward(c_idx);
        }
        
        //reduce clause counts for clauses containing complement variables
        for (uint i = 0; i < variables[idx].neg_sat.size(); i++) {
            const uint c_idx = variables[idx].neg_sat[i];
            clauses[c_idx].count--;
            //Check if we now have a unit clause or a broken constrain.
            //Only applies if the clause hasn't been satisfied yet
            if (clauses[c_idx].count <= 1 && curr_clauses[c_idx]) {
                if (clauses[c_idx].count == 1) {
                    unit_clauses.push_back(c_idx);
                }
                else {
                    constraint_broken = true;
                }
            }
            //updated affect for all variables associated with this clause
            for (uint j = 0; j < clauses[c_idx].satisfiers.size(); j++) {
                const uint j_idx = static_cast<uint>(abs(clauses[c_idx].satisfiers[j]));
                variables[j_idx].affected++;
                update_score(j_idx);
            }
        }
    }
    else {
        //mark false all solved clauses
        for (uint i = 0; i < variables[idx].neg_sat.size(); i++) {
            if (curr_clauses[variables[idx].neg_sat[i]]) {
                clause_count--;
                curr_clauses[variables[idx].neg_sat[i]] = false;
            }
            //find all variables that solve the same clause and update scores if
            //necessary
            const uint c_idx = variables[idx].neg_sat[i];
            update_scores_forward(c_idx);
        }
        
        
        //reduce clause counts for clauses containing complement variables
        for (uint i = 0; i < variables[idx].pos_sat.size(); i++) {
            const uint c_idx = variables[idx].pos_sat[i];
            clauses[c_idx].count--;
            
            if (clauses[c_idx].count <= 1 && curr_clauses[c_idx]) {
                if (clauses[c_idx].count == 1) {
                    unit_clauses.push_back(c_idx);
                }
                else {
                    constraint_broken = true;
                }
            }
            //updated affect for all variables associated with this clause
            for (uint j = 0; j < clauses[c_idx].satisfiers.size(); j++) {
                const uint j_idx = static_cast<uint>(abs(clauses[c_idx].satisfiers[j]));
                variables[j_idx].affected++;
                update_score(j_idx);
            }
        }
    }
    
    return !constraint_broken;
}

void SAT::update_scores_forward(const uint c_idx) {
    
    //loop through each satisfier
    for (uint j = 0; j < clauses[c_idx].satisfiers.size(); j++) {
        //if positive
        if (clauses[c_idx].satisfiers[j] > 0) {
            const uint j_idx = static_cast<uint>(clauses[c_idx].satisfiers[j]);
            //compare to each other satisfier
            for (uint k = 0; k < j; k++) {
                const uint k_idx = static_cast<uint>(abs(clauses[c_idx].satisfiers[k]));
                //if it affects this other satisfier, reduce score
                if (variables[j_idx].pos_affects.find(k_idx)
                    != variables[j_idx].pos_affects.end()) {
                    int amount = --variables[j_idx].pos_affects[k];
                    
                    if (amount > 0) {
                        variables[j_idx].affect /= amount + 1;
                        variables[j_idx].affect *= amount;
                        update_score(j_idx);
                    }
                    else {
                        variables[j_idx].pos_affects.erase(k_idx);
                    }
                }
            }
            //compare to each other satisfier
            for (uint k = j+1; k < clauses[c_idx].satisfiers.size(); k++) {
                const uint k_idx = static_cast<uint>(abs(clauses[c_idx].satisfiers[k]));
                //if it affects this other satisfier, reduce score
                if (variables[j_idx].pos_affects.find(k_idx)
                    != variables[j_idx].pos_affects.end()) {
                    int amount = --variables[j_idx].pos_affects[k];
                    
                    if (amount > 0) {
                        variables[j_idx].affect /= amount + 1;
                        variables[j_idx].affect *= amount;
                        update_score(j_idx);
                    }
                    else {
                        variables[j_idx].pos_affects.erase(k_idx);
                    }
                }
            }
        }
        //if negative
        else {
            const uint j_idx = static_cast<uint>(-clauses[c_idx].satisfiers[j]);
            for (uint k = 0; k < j; k++) {
                const uint k_idx = static_cast<uint>(abs(clauses[c_idx].satisfiers[k]));
                if (variables[j_idx].neg_affects.find(k_idx)
                    != variables[j_idx].neg_affects.end()) {
                    int amount = --variables[j_idx].neg_affects[k];
                    
                    if (amount > 0) {
                        variables[j_idx].affect /= amount + 1;
                        variables[j_idx].affect *= amount;
                        update_score(j_idx);
                    }
                    else {
                        variables[j_idx].neg_affects.erase(k_idx);
                    }
                }
                //TODO: check for neg and neg
            }
            for (uint k = j+1; k < clauses[c_idx].satisfiers.size(); k++) {
                const uint k_idx = static_cast<uint>(abs(clauses[c_idx].satisfiers[k]));
                if (variables[j_idx].neg_affects.find(k_idx)
                    != variables[j_idx].neg_affects.end()) {
                    int amount = --variables[j_idx].neg_affects[k];
                    
                    if (amount > 0) {
                        variables[j_idx].affect /= amount + 1;
                        variables[j_idx].affect *= amount;
                        update_score(j_idx);
                    }
                    else {
                        variables[j_idx].neg_affects.erase(k_idx);
                    }
                }
            }
        }
        
    }
}

vector<int> SAT::gather_solution() {
    vector<int> solution;
    
    while (!choices.empty()) {
        int temp = choices.back();
        solution.push_back(temp);
        choices.pop_back();
        if (temp == -choices.back()) {
            choices.pop_back();
        }
    }
    return solution;
}

void SAT::choose_next_var() {
    if (!unit_clauses.empty()) {
        int satisfier = 0;
        for (uint i = 0; i < clauses[unit_clauses.back()].satisfiers.size(); i++) {
            uint idx = static_cast<uint>(abs(clauses[unit_clauses.back()].satisfiers[i]));
            if (curr_variables[idx]) {
                satisfier = clauses[unit_clauses.back()].satisfiers[i];
            }
        }
        assert(satisfier != 0);
        unit_clauses.pop_back();
        choices.emplace_back(satisfier);
    }
    
}