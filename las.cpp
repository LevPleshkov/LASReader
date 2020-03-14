#include "las.h"

//
//  LAS.cpp
//  las reader
//
//  Created by Lev Pleshkov on 27.02.2020.
//

#include "las.h"


LAS::Parameter::Parameter()                     { }
LAS::Parameter::Parameter(bool r) : required(r) { }


LAS::LASFile::LASFile()                         {             }
LAS::LASFile::LASFile(const std::string& path)  { read(path); }


// reads file line by line, passing it each time to corresponding parsing function
// depending on in what file section current line is
void LAS::LASFile::read(const std::string& path)
{
    if (!correct_extension(path))
    {
        std::cerr << "Wrong file extension." << std::endl;
        exit(1);
    }
    else
    {
        std::ifstream file(path);

        if (!file)
        {
            std::cerr << "File could not be opened." << std::endl;
            exit(2);
        }

        std::string line;

        while (std::getline(file, line))
        {
            while (std::isspace(line.front()))
                line.erase(line.begin());

            if      (line.find("#" ) == 0) ; //section = COMMENT;
            else if (line.find("~V") == 0) section = Section::VERSION;
            else if (line.find("~W") == 0) section = Section::WELL;
            else if (line.find("~C") == 0) section = Section::CURVE;
            else if (line.find("~P") == 0) section = Section::PARAMETERS;
            else if (line.find("~O") == 0) section = Section::OTHER;
            else if (line.find("~A") == 0) section = Section::ASCII;
            else    // continue with current section
                    // and try to parse the line

            switch (section)
            {
                case Section::COMMENT:                                       break;
                case Section::VERSION:    parse_parameter(line, _version);   break;
                case Section::WELL:       parse_parameter(line, _well);      break;
                case Section::CURVE:      parse_parameter(line, _curve);     break;
                case Section::PARAMETERS: parse_parameter(line, _parameter); break;
                case Section::OTHER:      parse_other(line);                 break;
                case Section::ASCII:      parse_data(line);                  break;
                case Section::UNDEFINED:
                                          std::cerr << "Program has come to unexpected state." << std::endl;
                                          exit(3);                           break;
                default: break;
            }
        }

        // self-validate
        validate();

        // set the state
        have_read = true;
    }
}

// parses line for similar sections ~V, ~W, ~C, ~P
void LAS::LASFile::parse_parameter(const std::string& l, std::map<std::string, Parameter>& m)
{
    // locate delimiters in line
    size_t dot = l.find_first_of('.');                     // first occurence of '.'
    size_t ws  = l.substr(dot).find_first_of(' ') + dot;   // first occurense of ' ' after first '.'
    size_t col = l.find_last_of(':');                      // last  occurence of ':'

    // retrieve fields
    std::string mn = prepare(l.substr(0, dot));
    std::string un = prepare(l.substr(dot + 1, ws - dot));
    std::string va = prepare(l.substr(ws, col - ws));
    std::string de = prepare(l.substr(col + 1));

    // set name only for optional mnemonics
    if (!m[mn].required)
        m[mn].mnemonic = mn;

    m[mn].unit = un;
    m[mn].value = va;
    m[mn].description = de;

    m[mn].set = true;
}

// parses line for section ~O
void LAS::LASFile::parse_other(const std::string& l) { _other.append(l); }

// parses line for section ~A
void LAS::LASFile::parse_data(const std::string& l)
{
    std::stringstream stream(l);
    float first; // index value
    float next;  // each next value after index

    stream >> first;

    if (!wrapped())
    {
        // retrieve all values after index value
        // if there is no value at all, row of _data will contain corrupt values
        // if there is a value other than number, it is converted to 0
        for (u_long i = 0; i < _curve.size() - 1; i++)
        {
            stream >> next;
            _data[first].push_back(next);
        }
    }
    else
    {
        std::cerr << "Reading in wrapped mode is not implemented yet." << std::endl;
        exit(5);
    }
}

// crops colon and whitespaces of string
std::string LAS::LASFile::prepare(std::string str) const
{
    if (str.size() > 0)
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

// determines file mode
bool LAS::LASFile::wrapped() const
{
    if      (_version.at("WRAP").value == "NO" ) return false;
    else if (_version.at("WRAP").value == "YES") return true;
    else
    {
            std::cerr << "Could not determine wrap mode: WRAP value should be YES or NO." << std::endl;
            exit(4);
    }
}

// performs check and gives warnings, doesn't exit the program
void LAS::LASFile::validate() const
{
    // requred fields are in place
    for (auto const& m : _version)
        if (m.second.required && !m.second.set)
            std::cerr << "Required field " << m.first << " could not be read from file." << std::endl;

    for (auto const& m : _well)
        if (m.second.required && !m.second.set)
            std::cerr << "Required field " << m.first << " could not be read from file." << std::endl;

    // start and stop fields correspond to first and last index
    if (abs(std::stof(_well.at("STRT").value) - _data.begin()->first) > INDEX_PRECISION)
        std::cerr << "First index value " << _data.begin()->first
                  << " is not equal to STRT value " << _well.at("STRT").value << "." << std::endl;

    if (abs(std::stof(_well.at("STOP").value) - _data.rbegin()->first) > INDEX_PRECISION)
        std::cerr << "Last index value " << _data.rbegin()->first
                  << " is not equal to STOP value " << _well.at("STOP").value << "." << std::endl;

    // step field is 0 for inconsistent index
    if (!consistent_index() && std::stof(_well.at("STEP").value) != 0)
        std::cerr << "STEP must have a value of 0 for inconsistent increment of index." << std::endl;

    // step field corresponds to actual step of index values
    const float step = std::next(_data.begin())->first - _data.begin()->first;
    if (abs(std::stof(_well.at("STEP").value) - step) > INDEX_PRECISION && consistent_index())
        std::cerr << "Actual index step " << step
                  << " is not equal to STEP value " << _well.at("STEP").value << "." << std::endl;

}

// ckecks if input file's extension is "las" (case insensitive)
bool LAS::LASFile::correct_extension(const std::string& path) const
{
    std::string ext = path.substr(path.rfind('.') + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c){ return std::tolower(c); }
                  );

    return (ext.substr(0, 3) == "las") ? true : false;
}

// ckecks if the input values difference is constant
bool LAS::LASFile::consistent_index() const
{
    const float step = std::next(_data.begin())->first - _data.begin()->first;
    // whoo-hoo..
    for (auto i = ++(_data.begin()); i != --(_data.end()); i++)
        if (abs(step - ((++i)->first - (--i)->first)) > INDEX_PRECISION)
        {
            std::cerr << step << ":" << i->first << std::endl;
            return false;
        }
    return true;
}

void LAS::LASFile::info() const
{
    // ...
}


// Interface functions

// if file is read...
// - return version value if there is a key "VERS", otherwise return empty string
std::string    LAS::LASFile::version()  const { return (have_read && _version.find("VERS") != _version.end()) ? _version.at("VERS").value : ""; }
// - return Parameter value if there is such key, otherwise return empty parameter
LAS::Parameter LAS::LASFile::created()  const { return (have_read && _version.find("CREA") != _version.end()) ? _version.at("CREA") : LAS::Parameter(); }
LAS::Parameter LAS::LASFile::start()    const { return (have_read && _well.find("STRT") != _well.end()) ? _well.at("STRT") : LAS::Parameter(); }
LAS::Parameter LAS::LASFile::stop()     const { return (have_read && _well.find("STOP") != _well.end()) ? _well.at("STOP") : LAS::Parameter(); }
LAS::Parameter LAS::LASFile::step()     const { return (have_read && _well.find("STEP") != _well.end()) ? _well.at("STEP") : LAS::Parameter(); }
LAS::Parameter LAS::LASFile::null()     const { return (have_read && _well.find("NULL") != _well.end()) ? _well.at("NULL") : LAS::Parameter(); }
LAS::Parameter LAS::LASFile::company()  const { return (have_read && _well.find("COMP") != _well.end()) ? _well.at("COMP") : LAS::Parameter(); }
LAS::Parameter LAS::LASFile::well()     const { return (have_read && _well.find("WELL") != _well.end()) ? _well.at("WELL") : LAS::Parameter(); }
LAS::Parameter LAS::LASFile::field()    const { return (have_read && _well.find("FLD" ) != _well.end()) ? _well.at("FLD" ) : LAS::Parameter(); }
LAS::Parameter LAS::LASFile::location() const { return (have_read && _well.find("LOC" ) != _well.end()) ? _well.at("LOC" ) : LAS::Parameter(); }
LAS::Parameter LAS::LASFile::province() const { return (have_read && _well.find("PROV") != _well.end()) ? _well.at("PROV") : LAS::Parameter(); }
LAS::Parameter LAS::LASFile::county()   const { return (have_read && _well.find("CNTY") != _well.end()) ? _well.at("CNTY") : LAS::Parameter(); }
LAS::Parameter LAS::LASFile::state()    const { return (have_read && _well.find("STAT") != _well.end()) ? _well.at("STAT") : LAS::Parameter(); }
LAS::Parameter LAS::LASFile::country()  const { return (have_read && _well.find("CTRY") != _well.end()) ? _well.at("CTRY") : LAS::Parameter(); }
LAS::Parameter LAS::LASFile::service()  const { return (have_read && _well.find("SRVC") != _well.end()) ? _well.at("SRVC") : LAS::Parameter(); }
LAS::Parameter LAS::LASFile::date()     const { return (have_read && _well.find("DATE") != _well.end()) ? _well.at("DATE") : LAS::Parameter(); }
LAS::Parameter LAS::LASFile::uwi()      const { return (have_read && _well.find("UWI" ) != _well.end()) ? _well.at("UWI" ) : LAS::Parameter(); }
LAS::Parameter LAS::LASFile::api()      const { return (have_read && _well.find("API" ) != _well.end()) ? _well.at("API" ) : LAS::Parameter(); }
LAS::Parameter LAS::LASFile::licence()  const { return (have_read && _well.find("LIC" ) != _well.end()) ? _well.at("LIC" ) : LAS::Parameter(); }
// - return other information text if it exists, otherwise return empty string
std::string    LAS::LASFile::other()    const { return (have_read && !_other.empty()) ? _other : ""; }
// - return vector of Parameters if it is not empty, otherwise return empty vector
std::vector<LAS::Parameter> LAS::LASFile::parameters() const
{
    std::vector<LAS::Parameter> out;
    if (have_read && !_parameter.empty())
    {
        for (auto const& m : _parameter) out.push_back(m.second);
        return out;
    }
    else
        return out;
}
// - return vector of Parameters if it is not empty, otherwise return empty vector
std::vector<LAS::Parameter> LAS::LASFile::curves() const
{
    std::vector<LAS::Parameter> out;
    if (have_read && !_curve.empty())
    {
        for (auto const& m : _curve)
            out.push_back(m.second);
        return out;
    }
    else
        return out;
}
// - return vector of index values if they exist, otherwise return empty vector
std::vector<float> LAS::LASFile::index() const
{
    std::vector<float> out;
    if (have_read && !_data.empty())
    {
        for (auto const& m : _data)
            out.push_back(m.first);
        return out;
    }
    else
        return out;
}
// - return 2D vector of data values if they exist, otherwise return empty 2D vector
std::vector<std::vector<float>> LAS::LASFile::data() const
{
    std::vector<std::vector<float>> out;
    if (have_read && !_data.empty())
    {
        for (auto const& m : _data)
            out.push_back(m.second);
        return out;
    }
    else
        return out;
}
