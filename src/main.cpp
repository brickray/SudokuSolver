#include <iostream>
#include <algorithm>
#include <array>
#include <vector>

class SudokuSolver
{
public:
    using Uint = unsigned int;
    using SudokuBoard = std::array<std::array<Uint, 9>, 9>;

public:
    SudokuSolver() {}
    ~SudokuSolver() {}

    // return false if the input data has invalid number. i.e. each number of input data must be in the range of zero to nine [0, 9], and be -1 if that position is empty.
    bool SetBoard(const SudokuBoard& data)
    {
        board = data;
        return CheckIfDataValid();
    }

    bool Solver(SudokuBoard& result)
    {
        while (FindUniquePlace());
        GatherEmptySpace();
        bool found = SolveRecursely();

        result = board;

        return found;
    }

private:
    bool CheckIfDataValid() const
    {
        for (Uint i = 0; i < 9; ++i)
        {
            for (Uint j = 0; j < 9; ++j)
            {
                Uint v = board[i][j];
                if (v < empty_flag || v > 9)
                {
                    std::cout << "invalid digits at[" << i << "," << j << "]" << std::endl;
                    return false;
                }
            }
        }

        return true;
    }

    bool CheckDataContainsOne2NineUniquely(std::array<Uint, 9>& data) const
    {
        for (Uint i = 1; i <= 9; ++i)
        {
            if (i != data[i - 1])
                return false;
        }

        return true;
    }

    bool CheckIfThereAreEmptyPlaces(const SudokuBoard& new_try) const
    {
        for (auto& row : new_try)
        {
            for (auto v : row)
            {
                if (v == empty_flag)
                    return true;
            }
        }

        return false;
    }

    std::array<Uint, 9> GetSpecificRow(const SudokuBoard& new_try, Uint row) const
    {
        std::array<Uint, 9> data;
        memcpy(data.data(), new_try[row].data(), 9 * sizeof(Uint));

        return data;
    }

    std::array<Uint, 9> GetSpecificColumn(const SudokuBoard& new_try, Uint col) const
    {
        std::array<Uint, 9> data;
        for (Uint i = 0; i < 9; ++i)
        {
            data[i] = new_try[i][col];
        }

        return data;
    }

    std::array<Uint, 9> GetSpecificBlock(const SudokuBoard& new_try, Uint block) const
    {
        std::array<Uint, 9> data;
        Uint base_x = (block / 3) * 3;
        Uint base_y = (block % 3) * 3;
        for (Uint i = 0; i < 3; ++i)
        {
            for (Uint j = 0; j < 3; ++j)
            {
                data[i * 3 + j] = new_try[base_x + i][base_y + j];
            }
        }

        return data;
    }

    bool CheckIfSpecificRowSatisfied(const SudokuBoard& new_try, Uint row) const
    {
        std::array<Uint, 9> data = GetSpecificRow(new_try, row);
        std::sort(data.begin(), data.end());
        return CheckDataContainsOne2NineUniquely(data);
    }

    bool CheckIfSpecificColumnSatisfied(const SudokuBoard& new_try, Uint col) const
    {
        std::array<Uint, 9> data = GetSpecificColumn(new_try, col);
        std::sort(data.begin(), data.end());
        return CheckDataContainsOne2NineUniquely(data);
    }

    bool CheckIfSpecificBlockSatisfied(const SudokuBoard& new_try, Uint block) const
    {
        std::array<Uint, 9> data = GetSpecificBlock(new_try, block);
        std::sort(data.begin(), data.end());
        return CheckDataContainsOne2NineUniquely(data);
    }

    bool CheckIfWin(const SudokuBoard& new_try) const
    {
        if (CheckIfThereAreEmptyPlaces(new_try))
            return false;

        for (Uint i = 0; i < 9; ++i)
        {
            for (Uint j = 0; j < 9; ++j)
            {
                if (!CheckIfSpecificRowSatisfied(new_try, i)) return false;
                if (!CheckIfSpecificColumnSatisfied(new_try, j)) return false;
                if (!CheckIfSpecificBlockSatisfied(new_try, (i / 3) * 3 + j / 3)) return false;
            }
        }

        return true;
    }

    bool CheckIfPlacementValid(const SudokuBoard& new_try, Uint i, Uint j)
    {
        std::array<Uint, 9> row = GetSpecificRow(new_try, i);
        std::array<Uint, 9> col = GetSpecificColumn(new_try, j);
        std::array<Uint, 9> blk = GetSpecificBlock(new_try, (i / 3) * 3 + j / 3);

        auto Stat = [=](std::array<Uint, 9>& data)->bool
        {
            std::array<Uint, 10> statistics;
            statistics.fill(0);
            for (Uint i = 0; i < 9; ++i)
            {
                if (data[i] != empty_flag)
                {
                    statistics[data[i]]++;
                }
            }

            for (Uint i = 1; i <= 9; ++i)
            {
                if (statistics[i] > 1)
                    return false;
            }

            return true;
        };
        if (!Stat(row)) return false;
        if (!Stat(col)) return false;
        if (!Stat(blk)) return false;

        return true;
    }

    void GatherWhatDigitsCanBePlaced(std::vector<Uint>& digits, Uint i, Uint j)
    {
        std::array<Uint, 9> row = GetSpecificRow(board, i);
        std::array<Uint, 9> col = GetSpecificColumn(board, j);
        std::array<Uint, 9> blk = GetSpecificBlock(board, (i / 3) * 3 + j / 3);

        std::array<Uint, 10> statistics;
        statistics.fill(0);
        auto Stat = [&](std::array<Uint, 9>& data)
        {
            for (Uint i = 0; i < 9; ++i)
            {
                if (data[i] != empty_flag)
                {
                    statistics[data[i]]++;
                }
            }
        };
        Stat(row);
        Stat(col);
        Stat(blk);

        for (Uint i = 1; i <= 9; ++i)
        {
            if (statistics[i] == 0)
            {
                digits.push_back(i);
            }
        }
    }

    // find the place that it can be placed by only one digit, and place that number on that place
    bool FindUniquePlace()
    {
        bool found = false;
        for (Uint i = 0; i < 9; ++i)
        {
            for (Uint j = 0; j < 9; ++j)
            {
                if (board[i][j] != empty_flag)
                    continue;

                std::vector<Uint> digits;
                GatherWhatDigitsCanBePlaced(digits, i, j);
                if (digits.size() == 1)
                {
                    board[i][j] = digits[0];
                    found = true;
                }
            }
        }

        return found;
    }

    void GatherEmptySpace()
    {
        for (Uint i = 0; i < 9; ++i)
        {
            for (Uint j = 0; j < 9; ++j)
            {
                if (board[i][j] != empty_flag)
                    continue;

                empty_places.push_back(EmptyPlace(i, j));
                GatherWhatDigitsCanBePlaced(empty_places.back().array_of_digits_can_be_placed, i, j);
            }
        }
    }

    bool PutDigitRecuresely(SudokuBoard& new_try, Uint index)
    {
        if (index == empty_places.size())
        {
            bool found_correct_one = CheckIfWin(new_try);
            if (found_correct_one)
            {
                board = new_try;
            }
            return found_correct_one;
        }

        EmptyPlace& ep = empty_places[index];
        for (Uint i = 0; i < ep.array_of_digits_can_be_placed.size(); ++i)
        {
            new_try[ep.i][ep.j] = ep.array_of_digits_can_be_placed[i];
            if (!CheckIfPlacementValid(new_try, ep.i, ep.j)) continue;
            bool found = PutDigitRecuresely(new_try, index + 1);
            if (found)
                return true;
        }


        new_try[ep.i][ep.j] = empty_flag;
        return false;
    }

    bool SolveRecursely()
    {
        SudokuBoard new_try = board;
        return PutDigitRecuresely(new_try, 0);
    }

private:
    class EmptyPlace
    {
    public:
        EmptyPlace(Uint x, Uint y)
        {
            i = x;
            j = y;
        }

        Uint i; // coordinate
        Uint j; // coordinate

        // all the digits can be placed on this place
        std::vector<Uint> array_of_digits_can_be_placed;
    };

    SudokuBoard board;
    std::vector<EmptyPlace> empty_places;
    const Uint empty_flag = 0;
};

int main()
{
    SudokuSolver::SudokuBoard board;
    /*board[0][0] = 0; board[0][1] = 8; board[0][2] = 0; board[0][3] = 7; board[0][4] = 0; board[0][5] = 0; board[0][6] = 3; board[0][7] = 0; board[0][8] = 0;
    board[1][0] = 0; board[1][1] = 6; board[1][2] = 9; board[1][3] = 8; board[1][4] = 0; board[1][5] = 0; board[1][6] = 0; board[1][7] = 0; board[1][8] = 0;
    board[2][0] = 0; board[2][1] = 0; board[2][2] = 0; board[2][3] = 1; board[2][4] = 0; board[2][5] = 0; board[2][6] = 2; board[2][7] = 0; board[2][8] = 4;
    board[3][0] = 0; board[3][1] = 1; board[3][2] = 0; board[3][3] = 0; board[3][4] = 0; board[3][5] = 4; board[3][6] = 0; board[3][7] = 0; board[3][8] = 2;
    board[4][0] = 0; board[4][1] = 0; board[4][2] = 0; board[4][3] = 0; board[4][4] = 5; board[4][5] = 7; board[4][6] = 0; board[4][7] = 3; board[4][8] = 0;
    board[5][0] = 8; board[5][1] = 0; board[5][2] = 0; board[5][3] = 6; board[5][4] = 0; board[5][5] = 0; board[5][6] = 0; board[5][7] = 0; board[5][8] = 0;
    board[6][0] = 3; board[6][1] = 4; board[6][2] = 0; board[6][3] = 0; board[6][4] = 6; board[6][5] = 0; board[6][6] = 0; board[6][7] = 0; board[6][8] = 0;
    board[7][0] = 0; board[7][1] = 0; board[7][2] = 0; board[7][3] = 0; board[7][4] = 0; board[7][5] = 0; board[7][6] = 7; board[7][7] = 6; board[7][8] = 0;
    board[8][0] = 0; board[8][1] = 0; board[8][2] = 0; board[8][3] = 0; board[8][4] = 0; board[8][5] = 0; board[8][6] = 0; board[8][7] = 1; board[8][8] = 0;*/

    board[0][0] = 0; board[0][1] = 2; board[0][2] = 0; board[0][3] = 0; board[0][4] = 1; board[0][5] = 0; board[0][6] = 3; board[0][7] = 0; board[0][8] = 0;
    board[1][0] = 0; board[1][1] = 4; board[1][2] = 0; board[1][3] = 0; board[1][4] = 0; board[1][5] = 2; board[1][6] = 0; board[1][7] = 0; board[1][8] = 0;
    board[2][0] = 5; board[2][1] = 0; board[2][2] = 0; board[2][3] = 3; board[2][4] = 0; board[2][5] = 4; board[2][6] = 0; board[2][7] = 0; board[2][8] = 2;
    board[3][0] = 0; board[3][1] = 0; board[3][2] = 0; board[3][3] = 2; board[3][4] = 0; board[3][5] = 5; board[3][6] = 1; board[3][7] = 0; board[3][8] = 0;
    board[4][0] = 0; board[4][1] = 6; board[4][2] = 9; board[4][3] = 0; board[4][4] = 0; board[4][5] = 0; board[4][6] = 0; board[4][7] = 0; board[4][8] = 0;
    board[5][0] = 8; board[5][1] = 0; board[5][2] = 0; board[5][3] = 0; board[5][4] = 0; board[5][5] = 0; board[5][6] = 4; board[5][7] = 0; board[5][8] = 0;
    board[6][0] = 0; board[6][1] = 5; board[6][2] = 0; board[6][3] = 0; board[6][4] = 7; board[6][5] = 0; board[6][6] = 0; board[6][7] = 6; board[6][8] = 0;
    board[7][0] = 0; board[7][1] = 0; board[7][2] = 7; board[7][3] = 4; board[7][4] = 5; board[7][5] = 0; board[7][6] = 0; board[7][7] = 0; board[7][8] = 0;
    board[8][0] = 1; board[8][1] = 0; board[8][2] = 0; board[8][3] = 0; board[8][4] = 0; board[8][5] = 0; board[8][6] = 0; board[8][7] = 3; board[8][8] = 0;
    SudokuSolver solver;
    bool is_valid = solver.SetBoard(board);
    if (!is_valid)
    {
        std::cout << "the input data contains invalid digits, please checck and fix that" << std::endl;
        return -1;
    }

    SudokuSolver::SudokuBoard result;
    bool found = solver.Solver(result);
    if (!found)
    {
        std::cout << "the solver can not find the correct solution, maybe something wrong in input data" << std::endl;
        return -1;
    }

    for (SudokuSolver::Uint i = 0; i < 9; ++i)
    {
        for (SudokuSolver::Uint j = 0; j < 9; ++j)
        {
            std::cout << result[i][j] << "  ";
        }

        std::cout << "\n";
    }

    return 0;
}
