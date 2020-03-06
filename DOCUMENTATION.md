# DOCUMENTATION

LASReader contains one header file LAS.hpp and one implementation file LAS.cpp. Two structs declared in LAS namespace:
  * Parameter
  * LASFile

**Parameter** struct holds textual information of any single mandatory or optional parameter that has been read from input file, 
such as VERS, STRT or LIC.

**LASFile** struct holds all the information that could be read from input file.


## Example

The following example programm creates a `LAS::LASFile` pointer through a constructor that takes 
one argument `std::string path` and automatically reads the input file. Then the pointer is used to access
data (value and unit) that correspond to 'STEP' mnemonic in the file, other information and index values.
```c++
int main()
{
    std::string path = ".../.../*.las";
    
    LAS::LASFile* lasFile = new LAS::LASFile(path);
    
    std::cout << lasFile->step().value << std::endl;
    
    std::cout << lasFile->step().unit << std::endl;
    
    std::cout << lasFile->other() std::endl;
    
    for (auto const& x : lasFile->index())
        std::cout << x << std::endl;
    
    delete lasFile;
    
    return 0;
}
```



## struct LAS::Parameter

### Public properties
#### *std::string* name
- Holds mnemonic of the parameter.
#### *std::string* unit
- Holds unit, associated with the mnemonic.
#### *std::string* value
- Holds value, associated with the mnemonic.
#### *std::string* description
- Holds description, associated with the mnemonic.
 



## struct LAS::LASFile

### Constructors

#### LASFile()
Creates empty `LAS::LASFile` object with mandatory fields specified.
#### LASFile(const *std::string* path)
Same as `LASFile()`, but takes the path of input file.

### Read file, get its header info and version
#### *void* read(const *std::string* path)
- Reads the contents of file specified by `path` into `LAS::LASFile` object on which it was called.
#### *void* info()
- Outputs the header of las file into console.
#### *std::string* version()
- Returns the value of `Parameter` with mnemonic 'VERS'.

### Access the mandatory and optional parameters from section '~Well Information'
These functions take no arguments and return value of type is LAS::Parameter, whose fields can be accessed via '.' 
operator like this: `lasFile.srart().unit` or `lasFile.null().value`, where `lasFile` is an object of type `LAS::LASFile`.
#### *LAS::Parameter* start()
#### *LAS::Parameter* stop()
#### *LAS::Parameter* step()
#### *LAS::Parameter* null()
#### *LAS::Parameter* company()
#### *LAS::Parameter* well()
#### *LAS::Parameter* field()
#### *LAS::Parameter* location()
#### *LAS::Parameter* province()
#### *LAS::Parameter* county()
#### *LAS::Parameter* state()
#### *LAS::Parameter* country()
#### *LAS::Parameter* service()
#### *LAS::Parameter* date()
#### *LAS::Parameter* uwi()
#### *LAS::Parameter* api()
#### *LAS::Parameter* licence()

### Access data from '~Other Information' section
#### *std::string* other()
- Returns textual value from '~Other Information' section.

### Access the optional parameters from sections '~Curve Information' and '~Parameter Information'
#### *std::vector\<LAS::Parameter>* parameters()
- Returns the list of all parameters that were read from '~Parameter Information' section.
#### *std::vector\<LAS::Parameter>* curves()
- Returns the list of all parameters that were read from '~Curve Information' section.

### Access the indices and corresponding data from section '~ASCII Log Data'
#### *std::vector\<float>* index()
- Returns the list of index values.
#### *std::vector\<std::vector\<float>>* data()
- Returns the 2D list of all data values corresponfing to their index values.  Size of first dimention of **data** is the same as size of **index**.
