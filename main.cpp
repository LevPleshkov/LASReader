//
//  main.cpp
//  las reader
//
//  Created by Lev Pleshkov on 26.02.2020.
//  Copyright © 2020 Lev Pleshkov. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <vector>

#include "LAS.hpp"


int main(int argc, const char* argv[])
{
    std::string path = "/Users/levpleshkov/Developer/Projects/las reader/assets/survey.las";
    
    LAS::LASFile* lasFile = new LAS::LASFile(path);
    
    for (auto const& x : lasFile->index())
        std::cout << x << std::endl;
    
    delete lasFile;
    
    return 0;
}
