//
//  LAS.cpp
//  las reader
//
//  Created by Lev Pleshkov on 27.02.2020.
//  Copyright © 2020 Lev Pleshkov. All rights reserved.
//

#include "LAS.hpp"

LAS::LASFile::LASFile()
{
    set_default_names();
}

LAS::LASFile::LASFile(const std::string path)
{
    set_default_names();
    this->read(path);
}

void LAS::LASFile::read(const std::string path)
{
    file = std::ifstream(path);
    
    if (!file)
    {
        std::cerr << "File could not be opened." << std::endl;
        exit(1);
    }
    
    std::string line;
    
    while (std::getline(file, line))
    {
        // what is the upcoming section
        if (line.find("~V") == 0)
        {
            section = VERSION;
            read_v(file.tellg());
        }
        else if (line.find("~W") == 0)
        {
            section = WELL;
            read_w(file.tellg());
        }
        else if (line.find("~C") == 0)
        {
            section = CURVE;
            read_c(file.tellg());
        }
        else if (line.find("~P") == 0)
        {
            section = PARAMETERS;
            read_p(file.tellg());
        }
        else if (line.find("~O") == 0)
        {
            section = OTHER;
            read_o();
        }
        else if (line.find("~A") == 0)
        {
            section = ASCII;
            read_a();
        }
        else if (line.find("#") == 0)
        {
            //section = COMMENT;
        }
        else
            section = UNDEFINED;
        
    }
}

void LAS::LASFile::set_default_names()
{
    for (int i = 0; i < vers_size; i++)
        _vers_parameters[i].name = vers_names[i];
    
    for (int i = 0; i < well_size; i++)
        _well_parameters[i].name = well_names[i];
}

std::fstream::streampos LAS::LASFile::read_line_for_parameter(std::fstream::streampos pos,
                                                              LAS::Parameter& p)
{
    file.seekg(pos);
    
    ///*
    
    // make sure the section is not empty
    if (file.peek() == '~')
        return pos;
    
    std::string line;
    std::getline(file, line);
    
    size_t dot = line.find_first_of('.');                     // first occurence of '.'
    size_t ws  = line.substr(dot).find_first_of(' ') + dot;   // first occurense of ' ' after '.'
    size_t col = line.find_last_of(':');                      // last occurence of ':'
    
    std::string name = prepare_to_set(line.substr(0, dot));
    
    if (name != p.name && !p.name.empty())
        return pos;
    
    if (p.name.empty())
        p.name = name;
    
    p.unit = prepare_to_set(line.substr(dot + 1, ws));
    
    p.value = line.substr(ws, col - ws);
    
    p.description = prepare_to_set(line.substr(col + 1));
    
    p.isSet = true;
    
    //std::clog << "|" << line.substr(ws, col - ws).size() << "|" << std::endl;
    //file.ignore(512, '\n');
    
    return file.tellg();
    
    //*/
     
    /*
    
    if (file.peek() != '~') // make sure the section is not empty and not comment
    {
        int size = 512;
        char buffer[size];
        
        // read name
        file.get(buffer, size, '.');
        
        
        if (prepare_to_set(buffer) == p.name || p.name.empty())
        {
            if (p.name.empty()) // set name for variable parameters
                p.name = prepare_to_set(buffer);
            
            //std::clog << prepare_to_set(buffer) << std::endl;
                
            buffer[0] = 0; // clear
            
            file.ignore(1); // skip the '.'
            
            if (file.peek() != ' ')
            {
                // read unit
                file.get(buffer, size, ' ');
                p.unit = prepare_to_set(buffer);
                buffer[0] = 0; // clear
            }
            
            while (file.peek() == ' ')
                file.ignore(1);
            
            //file.get(buffer, size, '\n');
            //std::string str = buffer;
            // find position of last :
            // split value and description by position
            // set p.value and p.description
            
            if (file.peek() != ':') // make sure the value is in the line of the file
            {
                // read value
                file.get(buffer, size, ':');
                p.value = prepare_to_set(buffer);
                buffer[0] = 0;
            }
            
            file.ignore(1); // skip the ':'
            
            if (file.peek() != '\n') // make sure the description is in the line of the file
            {
                // read description
                file.get(buffer, size, '\n');
                p.description = prepare_to_set(buffer);
            }
            
            p.isSet = true;
            
            file.ignore(512, '\n');
            
            //std::clog << file.peek() << std::endl;
            
            return file.tellg();
        }
        else
        {
            return pos;
            
            //std::cerr << "Unexpected name of parameter in section: " << section << "."
            //          << " Name: " << prepare_to_set(buffer) << std::endl;
            
        }
    }
    else
    {
        //file.ignore(512, '\n');
        return pos;
    }
    
    */
}

std::string LAS::LASFile::prepare_to_set(std::string str)
{
    std::string out = str;
    
    if (out.size() > 1)
    {
        if (out.front() == ':')
            out.erase(out.begin());
        
        while (out.front() == ' ')
            out.erase(out.begin());
        
        while (out.back() == ' ')
            out.pop_back();
    }
    
    return out;
}

void LAS::LASFile::read_v(std::fstream::streampos pos)
{
    for (int i = 0; i < vers_size; i++)
        pos = read_line_for_parameter(pos, _vers_parameters[i]);
    
    if (_vers_parameters[1].value.find("NO"))
        wrapped = false;
    else if (_vers_parameters[1].value.find("YES"))
        wrapped = true;
    else
    {
        std::cerr << "Could not determine wrap mode." << std::endl;
        //exit(2);
    }
}

void LAS::LASFile::read_w(std::fstream::streampos pos)
{
    for (int i = 0; i < well_size; i++)
        pos = read_line_for_parameter(pos, _well_parameters[i]);
}

void LAS::LASFile::read_c(std::fstream::streampos pos)
{
    Parameter p;
    auto new_pos = read_line_for_parameter(pos, p);
    if (new_pos != pos)
    {
        _curves.push_back(p);
        read_c(new_pos);
    }
}

void LAS::LASFile::read_p(std::fstream::streampos pos)
{
    Parameter p;
    auto new_pos = read_line_for_parameter(pos, p);
    if (new_pos != pos)
    {
        _parameters.push_back(p);
        read_p(new_pos);
    }
}

void LAS::LASFile::read_o()
{
    while (file.peek() != '~')
        _other.push_back(file.get());
}

void LAS::LASFile::read_a()
{
    while (!file.eof())
    {
        std::string s;
        std::vector<float> row;
        for (int i = 0; i < _curves.size(); i++)
        {
            file >> s;
            if (!s.empty())
                row.push_back(std::stof(s));
        }
        if (!row.empty())
            _data.push_back(row);
    }
}

void LAS::LASFile::info() const
{
    for (Parameter p : _vers_parameters)
    {
        if (p.isSet)
            std::cout << p.name << " : " << p.unit << " : " << p.value << " : " << p.description << std::endl;
    }

    for (Parameter p : _well_parameters)
    {
        if (p.isSet)
            std::cout << p.name << " : " << p.unit << " : " << p.value << " : " << p.description << std::endl;
    }
    
    for (Parameter p : _curves)
        std::cout << p.name << " : " << p.unit << " : " << p.value << " : " << p.description << std::endl;
    
    for (Parameter p : _parameters)
        std::cout << p.name << " : " << p.unit << " : " << p.value << " : " << p.description << std::endl;
    
    std::cout << _other;;
    
    for (int i = 0; i < _data.size(); i++) {
        for (int j = 0; j < _data[0].size(); j++) {
            std::cout << _data[i][j] << " ";
        }
        std::cout << std::endl;
    }
        
}

std::string LAS::LASFile::version() const
{
    return _vers_parameters[0].value;
}

LAS::Parameter LAS::LASFile::start() const
{
    return _well_parameters[0];
}

LAS::Parameter LAS::LASFile::stop() const
{
    return _well_parameters[1];
}

LAS::Parameter LAS::LASFile::step() const
{
    return _well_parameters[2];
}

LAS::Parameter LAS::LASFile::null() const
{
    return _well_parameters[3];
}

LAS::Parameter LAS::LASFile::company() const
{
    return _well_parameters[4];
}

LAS::Parameter LAS::LASFile::well() const
{
    return _well_parameters[5];
}

LAS::Parameter LAS::LASFile::field() const
{
    return _well_parameters[6];
}

LAS::Parameter LAS::LASFile::location() const
{
    return _well_parameters[7];
}

LAS::Parameter LAS::LASFile::province() const
{
    return _well_parameters[8];
}

LAS::Parameter LAS::LASFile::county() const
{
    return _well_parameters[9];
}

LAS::Parameter LAS::LASFile::state() const
{
    return _well_parameters[10];
}

LAS::Parameter LAS::LASFile::country() const
{
    return _well_parameters[11];
}

LAS::Parameter LAS::LASFile::service() const
{
    return _well_parameters[12];
}

LAS::Parameter LAS::LASFile::date() const
{
    return _well_parameters[13];
}

LAS::Parameter LAS::LASFile::uwi() const
{
    return _well_parameters[14];
}

LAS::Parameter LAS::LASFile::api() const
{
    return _well_parameters[15];
}

LAS::Parameter LAS::LASFile::licence() const
{
    return _well_parameters[16];
}

std::string LAS::LASFile::other() const
{
    return _other;
}

std::vector<LAS::Parameter> LAS::LASFile::parameters() const
{
    return _parameters;
}

std::vector<LAS::Parameter> LAS::LASFile::curves() const
{
    return _curves;
}

std::vector<std::vector<float>> LAS::LASFile::data() const
{
    return _data;
}
