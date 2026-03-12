#include <iostream>
#include <vector>
#include <unordered_set>
#include <algorithm>

using namespace std;

class UCodeGenerator
{
    static const int inc_pc            = 1 << 0;
    static const int jmp_en            = 1 << 1;
    static const int next_inst         = 1 << 2;
    static const int halt              = 1 << 3;

    static const int wr_a              = 1 << 4;
    static const int wr_b              = 1 << 5;
    static const int wr_c              = 1 << 6;
    static const int wr_dmem           = 1 << 7;

    static const int rd_a              = 1 << 8;
    static const int rd_b              = 1 << 9;
    static const int rd_c              = 1 << 10;
    static const int rd_tmp            = 1 << 11;
    static const int rd_dmem           = 1 << 12;
    static const int rd_pmem           = 1 << 13;

    static const int alu_en            = 1 << 14;
    static const int alu_dis_a         = 1 << 15;
    static const int alu_cin           = 1 << 16;
    static const int alu_inv_b         = 1 << 17;
    static const int alu_sel_adder     = 1 << 18;
    static const int alu_s0            = 1 << 19;
    static const int alu_s1            = 1 << 20;
    static const int alu_shr           = 1 << 21;
    static const int dont_update_flags = 1 << 22;

    static int slice(int value, int high, int low)
    {
        int width = high - low + 1;
        int mask = (1 << width) - 1;
        return (value << low) & mask;
    }

    static bool match(int value, string pattern)
    {
        reverse(pattern.begin(), pattern.end());

        for (int bit = 0; bit < 13; bit++)
        {
            if (pattern[bit] == '_')
                continue;

            if (pattern[bit] == '0' and ((value >> bit) & 1) == 1)
                return false;

            if (pattern[bit] == '1' and ((value >> bit) & 1) == 0)
                return false;
        }

        return true;
    }

    static vector<int> ucode(int cycle, int flags, int inst)
    {
        if (cycle == 0)
            return {inc_pc};

        int dst = slice(inst, 4, 3);
        int src = slice(inst, 1, 0);


        if (match(inst, "000__0__") && dst <= 2)
        {
            int wr = {wr_a, wr_b, wr_c}[dst];
            return {}
        }

        return {};
    }
};