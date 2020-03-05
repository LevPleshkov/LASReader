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
    makeRequired();
}

LAS::LASFile::LASFile(const std::string path)
{
    makeRequired();
    read(path);
}

void LAS::LASFile::makeRequired()
{
    _version["VERS"].required = true;
    _version["WRAP"].required = true;
    _well["STRT"].required = true;
    _well["STOP"].required = true;
    _well["STEP"].required = true;
    _well["NULL"].required = true;
}

void LAS::LASFile::read(const std::string path)
{
    //if (path.rfind(".LAS") == 0)
        file = std::ifstream(path);
        
    
    if (!file)
    {
        std::cerr << "File could not be opened." << std::endl;
        exit(1);
    }
    
    std::string line;
    
    while (std::getline(file, line))
    {
        while (std::isspace(line.front()))
            line.erase(line.begin());
        
        if (line.find("#") == 0)
            ;//section = COMMENT;
        else if (line.find("~V") == 0)
            section = VERSION;
        else if (line.find("~W") == 0)
            section = WELL;
        else if (line.find("~C") == 0)
            section = CURVE;
        else if (line.find("~P") == 0)
            section = PARAMETERS;
        else if (line.find("~O") == 0)
            section = OTHER;
        else if (line.find("~A") == 0)
            section = ASCII;
        else
            // continue with current section
            // and try to parse the line
        
        switch (section)
        {
            case COMMENT:
                break;
            case VERSION:
                parse_parameter(line, _version);
                break;
            case WELL:
                parse_parameter(line, _well);
                break;
            case CURVE:
                parse_parameter(line, _curve);
                break;
            case PARAMETERS:
                parse_parameter(line, _parameter);
                break;
            case OTHER:
                parse_other(line);
                break;
            case ASCII:
                parse_data(line);
                break;
            case UNDEFINED:
                std::cerr << "Program has come to unexpected state." << std::endl;
                exit(5);
                break;
            default:
                break;
        }
    }
    
    // validate data
    validate();
    haveRead = true;
}

void LAS::LASFile::parse_parameter(const std::string& l, std::map<std::string, Parameter>& m)
{
    
    Parameter p;
    
    // locate delimiters in line
    size_t dot = l.find_first_of('.');                     // first occurence of '.'
    size_t ws  = l.substr(dot).find_first_of(' ') + dot;   // first occurense of ' ' after '.'
    size_t col = l.find_last_of(':');                      // last occurence of ':'
    
    // retrieve fields
    p.mnemonic    = prepare(l.substr(0, dot));
    p.unit        = prepare(l.substr(dot + 1, ws));
    p.value       = prepare(l.substr(ws, col - ws));
    p.description = prepare(l.substr(col + 1));
    p.set = true;
    
    //std::clog << p.mnemonic << " : " << p.unit << " : " << p.value << " : " << p.description << std::endl;
    
    m[p.mnemonic] = p;
    
}

void LAS::LASFile::parse_other(const std::string& l)
{
    _other.append(l);
    
    //std::clog << l << std::endl;
}

void LAS::LASFile::parse_data(const std::string& l)
{
    std::stringstream stream(l);
    float first;
    float next;
    
    stream >> first;
    
    if (!wrapped())
    {
        for (int i = 0; i < _curve.size() - 1; i++)
        {
            stream >> next;
            _data[first].push_back(next);
        }
    }
    else
    {
        std::cerr << "Reading in wrapped mode is not implemented yet." << std::endl;
        exit(3);
    }
    
    //std::clog << _data.rbegin()->first << " : ";
    //for (auto v : _data[first])
    //    std::clog << v << " ";
    //std::clog << std::endl;
}

std::string LAS::LASFile::prepare(std::string str)
{
    if (str.size() > 1)
    {
        if (str.front() == ':')
            str.erase(str.begin());
        
        while (std::isspace(str.front()))
            str.erase(str.begin());
        
        while (std::isspace(str.back()))
            str.pop_back();
    }
    
    return str;
}

bool LAS::LASFile::wrapped()
{
    if (_version.at("WRAP").value == "NO")
        return false;
    else if (_version.at("WRAP").value == "YES")
        return true;
    else
    {
        std::cerr << "Could not determine wrap mode: WRAP value should be YES or NO." << std::endl;
        exit(2);
    }
}

void LAS::LASFile::validate()
{
    // requred fields are in place
    for (auto const& m : _version)
        if (m.second.required && !m.second.set)
            std::cerr << "Required field " << m.first << " could not be read from file." << std::endl;
    
    for (auto const& m : _well)
        if (m.second.required && !m.second.set)
            std::cerr << "Required field " << m.first << " could not be read from file." << std::endl;
    
    // start and stop fields correspond to first and last index
    if (abs(std::stof(_well.at("STRT").value) - _data.begin()->first) > indexPrecision)
        std::cerr << "First index value " << _data.begin()->first
        << " is not equal to STRT value " << _well.at("STRT").value << "." << std::endl;
    
    if (abs(std::stof(_well.at("STOP").value) - _data.rbegin()->first) > indexPrecision)
        std::cerr << "Last index value " << _data.rbegin()->first
                  << " is not equal to STOP value " << _well.at("STOP").value << "." << std::endl;
    
    // step field is 0 for inconsistent index
    if (!consistentIndex() && std::stof(_well.at("STEP").value) != 0)
        std::cerr << "STEP must have a value of 0 for inconsistent step increment of index." << std::endl;
    
    // step field corresponds to actual step of index values
    float step = std::next(_data.begin())->first - _data.begin()->first;
    if (abs(std::stof(_well.at("STEP").value) - step) > indexPrecision && consistentIndex())
        std::cerr << "Actual index step " << step
                  << " is not equal to STEP value " << _well.at("STEP").value << "." << std::endl;
    
}

bool LAS::LASFile::consistentIndex()
{
    float step = std::next(_data.begin())->first - _data.begin()->first;
    for (auto i = ++(_data.begin()); i != --(_data.end()); i++)
        if (abs(step - ((++i)->first - (--i)->first)) > indexPrecision)
        {
            std::cerr << step << ":" << i->first << std::endl;
            return false;
        }
    return true;
}

void LAS::LASFile::info() const
{
    
}

std::string LAS::LASFile::version() const
{
    if (haveRead)
        return _version.at("VERS").value;
    else
        return "";
}

LAS::Parameter LAS::LASFile::start() const
{
    if (haveRead)
        return _well.at("STRT");
    else
        return LAS::Parameter();
}

LAS::Parameter LAS::LASFile::stop() const
{
    if (haveRead)
        return _well.at("STOP");
    else
        return LAS::Parameter();
}

LAS::Parameter LAS::LASFile::step() const
{
    if (haveRead)
        return _well.at("STEP");
    else
        return LAS::Parameter();
}

LAS::Parameter LAS::LASFile::null() const
{
    if (haveRead)
        return _well.at("NULL");
    else
        return LAS::Parameter();
}

LAS::Parameter LAS::LASFile::company() const
{
    if (haveRead && _well.find("COMP") != _well.end())
        return _well.at("COMP");
    else
        return LAS::Parameter();
}

LAS::Parameter LAS::LASFile::well() const
{
    if (haveRead && _well.find("WELL") != _well.end())
        return _well.at("WELL");
    else
        return LAS::Parameter();
}

LAS::Parameter LAS::LASFile::field() const
{
    if (haveRead && _well.find("FLD") != _well.end())
        return _well.at("FLD");
    else
        return LAS::Parameter();
}

LAS::Parameter LAS::LASFile::location() const
{
    if (haveRead && _well.find("LOC") != _well.end())
        return _well.at("LOC");
    else
        return LAS::Parameter();
}

LAS::Parameter LAS::LASFile::province() const
{
    if (haveRead && _well.find("PROV") != _well.end())
        return _well.at("PROV");
    else
        return LAS::Parameter();
}

LAS::Parameter LAS::LASFile::county() const
{
    if (haveRead && _well.find("CNTY") != _well.end())
        return _well.at("CNTY");
    else
        return LAS::Parameter();
}

LAS::Parameter LAS::LASFile::state() const
{
    if (haveRead && _well.find("STAT") != _well.end())
        return _well.at("STAT");
    else
        return LAS::Parameter();
}

LAS::Parameter LAS::LASFile::country() const
{
    if (haveRead && _well.find("CTRY") != _well.end())
        return _well.at("CTRY");
    else
        return LAS::Parameter();
}

LAS::Parameter LAS::LASFile::service() const
{
    if (haveRead && _well.find("SRVC") != _well.end())
        return _well.at("SRVC");
    else
        return LAS::Parameter();
}
LAS::Parameter LAS::LASFile::date() const
{
    if (haveRead && _well.find("DATE") != _well.end())
        return _well.at("DATE");
    else
        return LAS::Parameter();
}

LAS::Parameter LAS::LASFile::uwi() const
{
    if (haveRead && _well.find("UWI") != _well.end())
        return _well.at("UWI");
    else
        return LAS::Parameter();
}

LAS::Parameter LAS::LASFile::api() const
{
    if (haveRead && _well.find("API") != _well.end())
        return _well.at("API");
    else
        return LAS::Parameter();
}

LAS::Parameter LAS::LASFile::licence() const
{
    if (haveRead && _well.find("LIC") != _well.end())
        return _well.at("LIC");
    else
        return LAS::Parameter();
}

std::string LAS::LASFile::other() const
{
    if (haveRead && !_other.empty())
        return _other;
    else
        return "";
}

std::vector<LAS::Parameter> LAS::LASFile::parameters() const
{
    std::vector<LAS::Parameter> out;
    if (haveRead && !_parameter.empty())
    {
        for (auto const& m : _parameter)
            out.push_back(m.second);
        return out;
    }
    else
        return out;
}

std::vector<LAS::Parameter> LAS::LASFile::curves() const
{
    std::vector<LAS::Parameter> out;
    if (haveRead && !_curve.empty())
    {
        for (auto const& m : _curve)
            out.push_back(m.second);
        return out;
    }
    else
        return out;
}

std::vector<float> LAS::LASFile::index() const
{
    std::vector<float> out;
    if (haveRead && !_data.empty())
    {
        for (auto const& m : _data)
            out.push_back(m.first);
        return out;
    }
    else
        return out;
}

std::vector<std::vector<float>> LAS::LASFile::data() const
{
    std::vector<std::vector<float>> out;
    if (haveRead && !_data.empty())
    {
        for (auto const& m : _data)
            out.push_back(m.second);
        return out;
    }
    else
        return out;
}
