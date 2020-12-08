#include <iostream>
#include <bitset>
#include <sstream>
#include <vector>

/////////////////
//  returns final string to assemmbly
/////////////////
std::string Translate(const char *data);
void TranslateAll(const char *data, size_t size, std::vector<std::string> *out);

enum class Type
{
    R_type,
    I_type_ADD,
    I_type_LW,
    S_type,
    U_type,
    J,
    JR,
    B,
    ERROR
};

char *FromLittle(const char *data, int k);

Type GetType(const std::string &data);
std::string GetRegistr(const std::string &reg);
std::string GetNum(const std::string &num);
std::string GetNumUnsigned(const std::string &num, int reg = 10);

std::string GetBits(const char *data, int left, int right);

std::string ADDI(const char *data);
std::string ADD(const char *data);
std::string MUL(const char *data);
std::string SW(const char *data);
std::string LW(const char *data);
std::string JAL(const char *data);
std::string JR(const char *data);
std::string BGE(const char *data);