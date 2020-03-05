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
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <math.h>

namespace LAS
{

    struct Parameter
    {
        std::string mnemonic;
        std::string unit;
        std::string value;
        std::string description;
    private:
        bool set = false;
        bool required = false;
        friend struct LASFile;
    };


    struct LASFile
    {
        
    public:
        
        LASFile();
        LASFile(const std::string path);
        
        // Interface
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
        std::vector<float> index() const;
        std::vector<std::vector<float>> data() const;
        
    private:
        
        // reference to a file that must be read
        std::ifstream file;
        
        bool haveRead = false;
        
        // Current section in file
        enum Section {
            VERSION = 0, WELL,
            CURVE,       PARAMETERS,
            OTHER,       ASCII,
            COMMENT,     UNDEFINED
        };
        
        Section section = UNDEFINED;
        
        bool wrapped();
        bool consistentIndex();
        void makeRequired();
        void parse_parameter(const std::string&, std::map<std::string, Parameter>&);
        void parse_other(const std::string&);
        void parse_data(const std::string&);
        std::string prepare(std::string);
        void validate();
        
        
        // ~Version Information
        std::map<std::string, Parameter> _version = {
            {"VERS", Parameter()}, {"WRAP", Parameter()}
        };
        
        // ~Well Information
        std::map<std::string, Parameter> _well = {
            {"STRT", Parameter()}, {"STOP", Parameter()},
            {"STEP", Parameter()}, {"NULL", Parameter()}
        };
        
        // ~Curve Information
        std::map<std::string, Parameter> _curve;
        
        // ~Parameter Information
        std::map<std::string, Parameter> _parameter;
        
        // ~Other Information
        std::string _other;
        
        // ~ASCII Log Data
        std::map<float, std::vector<float>> _data;
        float currentIndex;
        float indexPrecision = 0.001;
        
    };

}

#endif /* LAS_hpp */
