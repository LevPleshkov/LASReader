//
//  LAS.hpp
//  las reader
//
//  Created by Lev Pleshkov on 27.02.2020.
//  Copyright © 2020 Lev Pleshkov. All rights reserved.
//

#ifndef LAS_hpp
#define LAS_hpp

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <array>

namespace LAS {

    struct Parameter {
        std::string name;
        std::string unit;
        std::string value;
        std::string description;
    private:
        bool isSet = false;
        friend struct LASFile;
    };

    struct LASFile {
        
    public:
        
        LASFile();
        LASFile(const std::string path);
        
        void read(const std::string path);
        void info() const;
        
        std::string version() const;
        Parameter start() const;
        Parameter stop() const;
        Parameter step() const;
        Parameter null() const;
        Parameter company() const;
        Parameter well() const;
        Parameter field() const;
        Parameter location() const;
        Parameter province() const;
        Parameter county() const;
        Parameter state() const;
        Parameter country() const;
        Parameter service() const;
        Parameter date() const;
        Parameter uwi() const;
        Parameter api() const;
        Parameter licence() const;
        std::string other() const;
        std::vector<Parameter> parameters() const;
        std::vector<Parameter> curves() const;
        std::vector<std::vector<float>> data() const;
        
    private:
        
        // Reference to a file that must be read
        std::ifstream file;
        
        
        // Utility functions and variables
        
        enum Section {
            VERSION = 0, WELL,
            CURVE,       PARAMETERS,
            OTHER,       ASCII,
            COMMENT,     UNDEFINED
        };
        
        Section section = UNDEFINED;
        
        void set_default_names();
        void read_v(std::fstream::streampos);
        void read_w(std::fstream::streampos);
        void read_c(std::fstream::streampos);
        void read_p(std::fstream::streampos);
        void read_o();
        void read_a();
        std::fstream::streampos read_line_for_parameter(std::fstream::streampos pos, LAS::Parameter& p);
        std::string prepare_to_set(std::string);
        bool last_colon();
        
        bool wrapped;
        static const int vers_size = 2;  // sizes may change only if LAS standart
        static const int well_size = 17; // changes the number of default parameters
        
        
        // Data
        
        // All members with names starting with _
        // are initialized with data directly from file
        
        // ~Version Information
        std::array<Parameter, vers_size> _vers_parameters;
        std::array<std::string, vers_size> vers_names = {
            // names of default parameters according to LAS standart
            "VERS", "WRAP"
        };
        
        // ~Well Information
        std::array<Parameter, well_size> _well_parameters;
        std::array<std::string, well_size> well_names = {
            // names of default parameters according to LAS standart
            "STRT", "STOP", "STEP", "NULL", "COMP", "WELL", "FLD" , "LOC" , "PROV",
            "CNTY", "STAT", "CTRY", "SRVC", "DATE", "UWI" , "API" , "LIC"
        };
        
        // ~Curve Information
        std::vector<Parameter> _curves;
        
        // ~Parameter Information
        std::vector<Parameter> _parameters;
        
        // ~Other Information
        std::string _other;
        
        // ~ASCII Log Data
        std::vector<std::vector<float>> _data;
        
    };

}

#endif /* LAS_hpp */
