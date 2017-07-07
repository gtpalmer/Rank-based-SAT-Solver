//
//  main.cpp
//  SAT_finder
//
//  Created by Graham Palmer on 11/10/16.
//  Copyright © 2016 Graham Palmer. All rights reserved.
//

#include <iostream>
#include "SAT.h"
#include <chrono>
#include <thread>
#include <algorithm>
#include <fstream>
#include <getopt.h>

using namespace std;

struct abs_less {
    bool operator()(int a, int b) {
        return abs(a) < abs(b);
    }
};
void find_solution(istream &is, ostream &os);

int main(int argc, char * argv[]) {
    
    //switches
    bool verbose = false;
    bool input = false;
    bool output = false;
    bool location = false;
    
    //Optional command line arguments
    static struct option longopts[] = {
        { "help", no_argument, nullptr, 'h' },
        { "verbose", no_argument, nullptr, 'v' },
        { "input", required_argument, nullptr, 'i' },
        { "output", required_argument, nullptr, 'o' },
        { "location", required_argument, nullptr, 'l' },
        { nullptr, 0, nullptr, '\0'}
    };
    
    string in_file;
    string out_file;
    string filepath = "";
    
    int idx = 0;
    int c;
    while ((c = getopt_long(argc, argv, "hvi:o:l:", longopts, &idx)) != -1) {
        switch (c) {
            case 'h':
                cout << "Welcome! You are currently using a Rank-Based Boolean "
                    << "formula solver, developed by Graham Palmer.\n\n\n ";
                cout << "To use the solver, you must enter an input file using "
                    << "--input or -i followed by the file name, or pass the "
                    << "file through standard input. Files must follow the "
                    <<" following format: \n\n";
                cout << "c comment (For any number of lines as long as the first"
                    << "letter is c)\n";
                cout << "p numvars(4) numclauses(5)\n";
                cout << "2 3 0\n";
                cout << "-1 4 3 0\n";
                cout << " 3 1 -2 0\n";
                cout << " 1 2 -3 4 0\n";
                cout << " 2 -3 0\n\n";
                cout << "The line starting with 'p' designates how many variables "
                    << "and how many clauses the formula will have. "
                    << "There must be exactly as many clauses as designated and "
                    << "No variables outside of the range 1-numvars present. Note "
                    << "that each following line represents a clause and each clause "
                    << "is separated by a 0. Each variable can be separated by any "
                    << "amount of white space. \n\n\n";
                cout << "Normal output will be printed to standard out and will "
                    << "simply output a correct assignment if there is one, or "
                    << "'NO SOLUTION' if there is none.\n";
                cout << "If --output or -o is present as a command line argument, "
                    << "a filename argument is expected and output will be directed "
                    << "to this file\n\n\n";
                cout << "If -verbose or -v is present, then the program will also "
                    << "print the number of branches created in the algorithm to "
                    << "track algorithm complexity.\n\n\n";
                cout << "If you wish to run the program in a directory other than "
                    << "the working directory, you may add --location or -l and "
                    << "add the directory path. Output will be set in the "
                    << "same directory as input.\n\n\n";
                exit(0);
                break;
            case 'v':
                if (verbose) {
                    cerr << "Verbose argument specified twice." << endl;
                    exit(1);
                }
                else {
                    verbose = true;
                }
                break;
            case 'i':
                if (input) {
                    cerr << "Input argument specified twice." << endl;
                    exit(1);
                }
                else {
                    input = true;
                    in_file = optarg;
                }
                break;
            case 'o':
                if (output) {
                    cerr << "Output argument specified twice." << endl;
                    exit(1);
                }
                else {
                    output = true;
                    out_file = optarg;
                }
                break;
            case 'l':
                if (location) {
                    cerr << "Location argument specified twice." << endl;
                    exit(1);
                }
                else {
                    location = true;
                    filepath = optarg;
                }
                break;
            default:
                cerr << "ERROR: Invalid command line argument" << endl;
                exit(1);
        }
    }
    
    
    if (input) {
        ifstream fin;
        fin.open(filepath + in_file);
        assert(fin.is_open());
        
        if (output) {
            ofstream fout;
            fout.open(filepath + out_file);
            assert(fout.is_open());
            
            find_solution(fin, fout);
            
            fout.close();
        }
        else {
            find_solution(fin, cout);
        }
        fin.close();
    }
    else {
        if (output) {
            ofstream fout;
            fout.open(filepath + out_file);
            assert(fout.is_open());
            
            find_solution(cin, fout);
            
            fout.close();
        }
        else {
            find_solution(cin, cout);
        }
    }
    
}

void find_solution(istream &is, ostream &os) {
    SAT mySAT(is, os);
    
    vector<int> solution = mySAT.solve();
    
    if (solution.empty()) {
        os << "NO SOLUTION\n";
    }
    else {
        abs_less comp;
        sort(solution.begin(), solution.end(), comp);
        os << "SOLUTION: \n";
        for (uint i = 0; i < solution.size(); i++) {
            os << solution[i] << " ";
        }
    }
    if (!solution.empty()) {
        if (mySAT.verify(solution)) {
            os << "VERIFIED!" << endl;
        }
        else {
            os << "Incorrect solution :(\n";
        }
        
    }
}
