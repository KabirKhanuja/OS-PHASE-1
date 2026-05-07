#include <iostream>
#include <fstream>
#include <cstring>
#include <cctype>
#include <iomanip>
#include <string>

using namespace std;

// ================= GLOBAL VARIABLES =================
char M[100][4];
char IR[4];
char R[4];

int IC;
bool C;
char Buffer[41];
int m;
int SI;

// ================= FUNCTION DECLARATIONS =================
void INIT();
void LOAD(ifstream &infile, ofstream &outfile);
void STARTEXECUTION(ifstream &infile, ofstream &outfile);
void MOS(ifstream &infile, ofstream &outfile);
void READ(ifstream &infile);
void WRITE(ofstream &outfile);
void TERMINATE(ofstream &outfile);
void print_memory();

int toInt(char a[4]);
void toCharArray(int num, char a[4]);

// ================= MAIN =================
int main()
{
    cout << "\n=================================================\n";
    cout << "           MOS PHASE-I SIMULATOR STARTED         \n";
    cout << "=================================================\n";

    ifstream infile("input.txt"); // Add the input file path here
    ofstream outfile("output.txt", ios::app);

    if (!infile.is_open())
    {
        cerr << "ERROR: Cannot open input.txt\n";
        return 1;
    }

    if (!outfile.is_open())
    {
        cerr << "ERROR: Cannot create output.txt\n";
        return 1;
    }

    while (!infile.eof())
    {
        LOAD(infile, outfile);

        if (infile.eof())
            break;

        STARTEXECUTION(infile, outfile);
    }

    print_memory();

    cout << "\n=================================================\n";
    cout << "         EXECUTION COMPLETED SUCCESSFULLY        \n";
    cout << "=================================================\n";

    infile.close();
    outfile.close();

    return 0;
}

// ================= INIT =================
void INIT()
{
    cout << "\n[INIT] Resetting Memory and Registers...\n";

    for (int i = 0; i < 4; i++)
    {
        IR[i] = '*';
        R[i] = '*';
    }

    IC = 0;
    C = false;
    SI = 0;

    for (int i = 0; i < 100; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            M[i][j] = '*';
        }
    }
}

// ================= CHAR ARRAY TO INT =================
int toInt(char a[4])
{
    string s = "";

    for (int i = 0; i < 4; i++)
    {
        if (isdigit(a[i]))
            s += a[i];
    }

    if (s == "")
        return 0;

    return stoi(s);
}

// ================= INT TO CHAR ARRAY =================
void toCharArray(int num, char a[4])
{
    string s = to_string(num);

    while (s.length() < 4)
        s = "0" + s;

    if (s.length() > 4)
        s = s.substr(s.length() - 4);

    for (int i = 0; i < 4; i++)
        a[i] = s[i];
}

// ================= LOAD =================
void LOAD(ifstream &infile, ofstream &outfile)
{
    while (infile.getline(Buffer, 41))
    {

        if (strncmp(Buffer, "$AMJ", 4) == 0)
        {
            cout << "\n[LOADER] New Job Started ($AMJ)\n";
            INIT();
            m = 0;
        }

        else if (strncmp(Buffer, "$DTA", 4) == 0)
        {
            cout << "[LOADER] Program Loading Complete ($DTA)\n";
            break;
        }

        else if (strncmp(Buffer, "$END", 4) == 0)
        {
            cout << "[LOADER] End of Job ($END)\n";
            continue;
        }

        else
        {
            cout << "[LOADER] Loading Program Card at Memory Block " << m << "\n";

            int buffer_len = strlen(Buffer);
            int buffer_index = 0;
            int limit = m + 10;

            while (m < limit && buffer_index < buffer_len && m < 100)
            {
                for (int j = 0; j < 4; j++)
                {
                    if (buffer_index < buffer_len)
                        M[m][j] = Buffer[buffer_index++];
                    else
                        M[m][j] = '*';
                }

                m++;
            }

            if (m % 10 != 0)
            {
                m = ((m / 10) + 1) * 10;
            }
        }
    }
}

// ================= START EXECUTION =================
void STARTEXECUTION(ifstream &infile, ofstream &outfile)
{
    cout << "\n=================================================\n";
    cout << "            USER PROGRAM EXECUTION START         \n";
    cout << "=================================================\n";

    IC = 0;

    while (true)
    {

        for (int i = 0; i < 4; i++)
        {
            IR[i] = M[IC][i];
        }

        cout << "\n[CPU] IC = "
             << setw(2) << setfill('0') << IC
             << " | IR = "
             << IR[0] << IR[1] << IR[2] << IR[3] << "\n";

        IC++;

        char op1 = IR[0];
        char op2 = IR[1];

        if (op1 == 'G' && op2 == 'D')
        {
            SI = 1;
            MOS(infile, outfile);
        }

        else if (op1 == 'P' && op2 == 'D')
        {
            SI = 2;
            MOS(infile, outfile);
        }

        else if (op1 == 'H')
        {
            SI = 3;
            MOS(infile, outfile);
            break;
        }

        else
        {
            int operand = 0;

            if (isdigit(IR[2]) && isdigit(IR[3]))
            {
                operand = (IR[2] - '0') * 10 + (IR[3] - '0');
            }
            else
            {
                cerr << "ERROR: Invalid Operand\n";
                break;
            }

            // LR
            if (op1 == 'L' && op2 == 'R')
            {
                cout << "[EXEC] LR " << operand << "\n";

                for (int i = 0; i < 4; i++)
                    R[i] = M[operand][i];
            }

            // SR
            else if (op1 == 'S' && op2 == 'R')
            {
                cout << "[EXEC] SR " << operand << "\n";

                for (int i = 0; i < 4; i++)
                    M[operand][i] = R[i];
            }

            // CR
            else if (op1 == 'C' && op2 == 'R')
            {
                cout << "[EXEC] CR " << operand << "\n";

                C = true;

                for (int i = 0; i < 4; i++)
                {
                    if (R[i] != M[operand][i])
                    {
                        C = false;
                        break;
                    }
                }

                cout << "[STATE] C = " << C << "\n";
            }

            // BT
            else if (op1 == 'B' && op2 == 'T')
            {
                cout << "[EXEC] BT " << operand << "\n";

                if (C)
                {
                    IC = operand;
                    cout << "[STATE] Branch Taken -> IC = " << IC << "\n";
                }
            }

            // AD
            else if (op1 == 'A' && op2 == 'D')
            {
                cout << "[EXEC] AD " << operand << "\n";

                int r = toInt(R);
                int mem = toInt(M[operand]);

                int result = r + mem;

                toCharArray(result, R);
            }

            // SB
            else if (op1 == 'S' && op2 == 'B')
            {
                cout << "[EXEC] SB " << operand << "\n";

                int r = toInt(R);
                int mem = toInt(M[operand]);

                int result = r - mem;

                toCharArray(result, R);
            }

            // ML
            else if (op1 == 'M' && op2 == 'L')
            {
                cout << "[EXEC] ML " << operand << "\n";

                int r = toInt(R);
                int mem = toInt(M[operand]);

                int result = r * mem;

                toCharArray(result, R);
            }

            // DV
            else if (op1 == 'D' && op2 == 'V')
            {
                cout << "[EXEC] DV " << operand << "\n";

                int r = toInt(R);
                int mem = toInt(M[operand]);

                if (mem == 0)
                {
                    cerr << "ERROR: Divide By Zero\n";
                    break;
                }

                int result = r / mem;

                toCharArray(result, R);
            }

            else
            {
                cerr << "ERROR: Invalid Opcode\n";
                break;
            }
        }
    }
}

// ================= MOS =================
void MOS(ifstream &infile, ofstream &outfile)
{
    switch (SI)
    {
    case 1:
        READ(infile);
        break;

    case 2:
        WRITE(outfile);
        break;

    case 3:
        TERMINATE(outfile);
        break;
    }
}

// ================= READ =================
void READ(ifstream &infile)
{
    int operand = (IR[2] - '0') * 10 + (IR[3] - '0');

    cout << "[MOS] READ -> Loading Data into Memory[" << operand
         << "-" << operand + 9 << "]\n";

    if (infile.getline(Buffer, 41))
    {
        int len = strlen(Buffer);
        int idx = 0;

        for (int i = 0; i < 10; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                if (idx < len)
                    M[operand + i][j] = Buffer[idx++];
                else
                    M[operand + i][j] = '*';
            }
        }
    }
}

// ================= WRITE =================
void WRITE(ofstream &outfile)
{
    int operand = (IR[2] - '0') * 10 + (IR[3] - '0');

    cout << "[MOS] WRITE -> Outputting Memory[" << operand
         << "-" << operand + 9 << "]\n";

    int idx = 0;

    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            Buffer[idx++] = M[operand + i][j];
        }
    }

    for (int i = 0; i < 40; i++)
    {
        if (Buffer[i] != '*')
            outfile << Buffer[i];
    }

    outfile << "\n";
}

// ================= TERMINATE =================
void TERMINATE(ofstream &outfile)
{
    cout << "[MOS] TERMINATE -> Job Finished\n";

    outfile << "\nPROGRAM TERMINATED\n";
    outfile << "-----------------------------\n\n";
}

// ================= MEMORY DUMP =================
void print_memory()
{
    cout << "\n\n";
    cout << "===============================================================\n";
    cout << "                     MAIN MEMORY DUMP                          \n";
    cout << "===============================================================\n";

    for (int row = 0; row < 10; row++)
    {

        cout << "+---------------------------------------------------------------------------------------------------+\n";

        for (int col = 0; col < 10; col++)
        {
            int idx = row * 10 + col;

            cout << "| "
                 << setw(2) << setfill('0') << idx
                 << ":";

            for (int j = 0; j < 4; j++)
            {
                cout << M[idx][j];
            }

            cout << " ";
        }

        cout << "|\n";
    }

    cout << "+---------------------------------------------------------------------------------------------------+\n";
}