#include "translation.hpp"

// int main()
// {
//     std::cout << Translate("00110011000001011010010100000010") << std::endl;
// }

void TranslateAll(const char *data, size_t size, std::vector<std::string> *out)
{
    for (size_t i = 0; i < size / 4; ++i)
    {
        std::string str = std::move(Translate(data + 4 * i));
        if (str == "j\t0x")
        {
            char *temp_str = new char[10];
            sprintf(temp_str, "%lx", i * 4);
            str.append(temp_str);
            delete (temp_str);
        }
        if (str[0] == 'b' && str[1] == 'g' && str[2] == 'e')
        {
            char *temp_str = new char[10];
            sprintf(temp_str, "%lx", i * 4);
            str.append(temp_str);
            delete (temp_str);
        }

        (*out)[i].append(str);

        std::cout << (*out)[i] << std::endl;
    }
}

std::string Translate(const char *data)
{
    char *new_data = FromLittle(data, 4);
    std::string out;
    //std::cout << new_data << std::endl;

    switch (GetType(GetBits(new_data, 25, 32)))
    {
    case Type::R_type:
        if (GetBits(new_data, 17, 20) == "000")
        {
            if (GetBits(new_data, 0, 7) == "0000000")
                out = std::move(ADD(new_data));
            if (GetBits(new_data, 0, 7) == "0000001")
                out = std::move(MUL(new_data));
        }

        if (out.empty())
            out = "Not R_Type!";
        break;

    case Type::I_type_ADD:
        if (GetBits(new_data, 17, 20) == "000")
            out = ADDI(new_data);

        if (out.empty())
            out = "Not I_Type!";
        break;

    case Type::S_type:
        if (GetBits(new_data, 17, 20) == "010")
            out = SW(new_data);

        if (out.empty())
            out = "Not S_Type!\n";
        break;

    case Type::I_type_LW:
        if (GetBits(new_data, 17, 20) == "010")
            out = LW(new_data);

        if (out.empty())
            out = "Not I_Type!\n";
        break;

    case Type::J:
        out = JAL(new_data);
        break;

    case Type::JR:
        if (GetBits(new_data, 17, 20) == "000")
            out = JR(new_data);
        if (out.empty())
            out = "Not JR type!";
        break;

    case Type::B:
        if (GetBits(new_data, 17, 20) == "101")
            out = BGE(new_data);

        if (out.empty())
            out = "Not B format!";
        break;

    default:
        out = "Not appropriate instruction!";
        break;
    }

    delete new_data;
    return out;
}

Type GetType(const std::string &opcode)
{
    //std::cout << opcode;
    if (opcode == "0010011") //ADDI
        return Type::I_type_ADD;
    if (opcode == "0000011") //LW
        return Type::I_type_LW;

    if (opcode == "0110011") //ADD || MUL
        return Type::R_type;

    if (opcode == "0100011") //SW
        return Type::S_type;

    if (opcode == "1101111") //JAL
        return Type::J;

    if (opcode == "1100111")
        return Type::JR;

    if (opcode == "1100011")
        return Type::B;

    return Type::ERROR;
}

std::string GetBits(const char *data, int left, int right)
{
    return std::string(data + left, data + right);
}

std::string GetRegistr(const std::string &reg)
{
    int out = 0;
    for (int i = 0; i < reg.size(); ++i)
    {
        if (reg[i] == '1')
        {
            out += (1 << (reg.size() - i - 1));
        }
    }

    std::stringstream ss;
    ss << out;
    return std::string{"x"} + std::string{ss.str()};
}

std::string GetNum(const std::string &num)
{
    int out = 0;
    for (int i = 1; i < num.size(); ++i)
    {
        if (num[i] == '1')
        {
            out += (1 << (num.size() - i - 1));
        }
    }
    if (num[0] == '1')
        out -= (1 << (num.size() - 1));

    std::stringstream ss;
    ss << out;
    return std::string{ss.str()};
}
std::string GetNumUnsigned(const std::string &num, int reg /*= 10*/)
{
    int out = 0;
    for (int i = 0; i < num.size(); ++i)
    {
        if (num[i] == '1')
        {
            out += (1 << (num.size() - i - 1));
        }
    }

    std::stringstream ss;
    ss << out;
    return std::string{ss.str()};
}

char *FromLittle(const char *data, int k)
{
    //std::cout << data << std::endl;
    char *out = new char[32];
    for (int j = 0; j < k; ++j)
    {
        for (int i = 0; i < 8; ++i)
            out[8 * j + i] = ((data[3 - j] & (1 << (7 - i))) >> (7 - i)) + '0';
    }
    //std::cout << out;
    return out;
}

std::string ADDI(const char *data)
{
    std::string out = "addi\t";
    out += GetRegistr(GetBits(data, 20, 25)) + ", ";
    out += GetRegistr(GetBits(data, 12, 17)) + ", ";
    out += GetNum(GetBits(data, 0, 12));
    return out;
}
std::string ADD(const char *data)
{
    std::string out = "add\t";
    out += GetRegistr(GetBits(data, 20, 25)) + ", ";
    out += GetRegistr(GetBits(data, 12, 17)) + ", ";
    out += GetRegistr(GetBits(data, 7, 12));
    return out;
}
std::string SW(const char *data)
{
    std::string out = "sw\t";
    out += GetRegistr(GetBits(data, 12, 17)) + ", ";
    out += GetNumUnsigned(GetBits(data, 20, 25)) + "(";
    out += GetRegistr(GetBits(data, 7, 12)) + ")";
    return out;
}
std::string LW(const char *data)
{
    std::string out = "lw\t";
    out += GetRegistr(GetBits(data, 20, 25)) + ", ";
    out += GetNum(GetBits(data, 0, 12)) + "(";
    out += GetRegistr(GetBits(data, 12, 17)) + ")";
    return out;
}
std::string MUL(const char *data)
{
    std::string out = "mul\t";
    out += GetRegistr(GetBits(data, 20, 25)) + ", ";
    out += GetRegistr(GetBits(data, 12, 17)) + ", ";
    out += GetRegistr(GetBits(data, 7, 12));
    return out;
}
std::string BGE(const char *data)
{
    std::string out = "bge\t";
    out += GetRegistr(GetBits(data, 12, 17)) + ", ";
    out += GetRegistr(GetBits(data, 7, 12)) + ", 0x";
    return out;
}
std::string JAL(const char *data)
{
    std::string out = "j\t0x";
    return out;
}
std::string JR(const char *data)
{
    std::string out = "ret";
    return out;
}