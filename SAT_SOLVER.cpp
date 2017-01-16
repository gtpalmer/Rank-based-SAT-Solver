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

using namespace std;

struct abs_less {
    bool operator()(int a, int b) {
        return abs(a) < abs(b);
    }
};

int main() {
    
//    std::this_thread::sleep_for(std::chrono::nanoseconds(100000000));
    /*
    if (getenv("MYARQ")) {
        ifstream arq(getenv("MYARQ"));
        cin.rdbuf(arq.rdbuf());
    }
    */
   /*
    string filename = "large_sat.txt";
    ifstream fin;
    fin.open(filename);
    assert(fin.is_open());
    */
    
    SAT mySat(cin);
    
    vector<int> solution = mySat.solve();
    if (solution.empty()) {
        cout << "NO SOLUTION\n";
    }
    else {
        abs_less comp;
        sort(solution.begin(), solution.end(), comp);
        cout << "SOLUTION: \n";
        for (uint i = 0; i < solution.size(); i++) {
            cout << solution[i] << " ";
        }
    }
    if (!solution.empty()) {
        if (mySat.verify(solution)) {
            cout << "VERIFIED!" << endl;
        }
        else {
            cout << "Incorrect solution :(\n";
        }

    }
}
