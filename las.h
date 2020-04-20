#ifndef LAS_H
#define LAS_H


#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <math.h>


struct Parameter;
struct LASFile;


struct Parameter
{
    Parameter();
    Parameter(bool r);

    std::string mnemonic;
    std::string unit;
    std::string value;
    std::string description;

private:
    int order = 0;
    bool set = false;
    bool required = false;
    friend struct LASFile;
};

struct LASFile
{

public:

    LASFile();
    LASFile(const std::string& path);

    // Interface
    void read(const std::string& path);
    void info() const;
    std::string version() const;
    Parameter created() const;
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

    // State
    bool have_read = false;

    // Current section in file
    enum class Section {
        VERSION,     WELL,      CURVE,       PARAMETERS,
        OTHER,       ASCII,     COMMENT,     UNDEFINED
    };

    Section section = Section::UNDEFINED;

    int current_order;

    // Utility functions
    void parse_parameter(const std::string&, std::map<std::string, Parameter>&);
    void parse_other(const std::string&);
    void parse_data(const std::string&);
    std::string prepare(std::string) const; // not using rvalue reference as parameter, because it should return "" sometimes
    bool correct_extension(const std::string& path) const;
    bool consistent_index() const;
    bool wrapped() const;
    void validate() const;


    // ~Version Information
    std::map<std::string, Parameter> _version = {
        {"VERS", Parameter(true)}, {"WRAP", Parameter(true)}
    };

    // ~Well Information
    std::map<std::string, Parameter> _well = {
        {"STRT", Parameter(true)}, {"STOP", Parameter(true)},
        {"STEP", Parameter(true)}, {"NULL", Parameter(true)}
    };

    // ~Curve Information
    std::map<std::string, Parameter> _curve;

    // ~Parameter Information
    std::map<std::string, Parameter> _parameter;

    // ~Other Information
    std::string _other;

    // ~ASCII Log Data
    std::map<float, std::vector<float>> _data;
    constexpr static const float INDEX_PRECISION = 0.001f;
    //float CURRENT_INDEX; // for reading in wrapped mode

};


#endif // LAS_H
