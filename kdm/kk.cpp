#include<iostream>
#include<fstream>
#include<cstring>
#include<cctype>

using namespace std;

// Global variables
char M[100][4];  // Main Memory (RAM)
char IR[4];      // Instruction Register
char R[4];       // General Purpose Register
int IC;          // Instruction Counter
bool C;          // Toggle (Condition Code)
char Buffer[41]; // Input Buffer (40 chars + null terminator)
int m;           // Memory pointer for LOAD

int SI;          // Service Interrupt Flag

// Function declarations
void INIT();
void LOAD(ifstream &infile, ofstream &outfile);
void STARTEXECUTION(ifstream &infile, ofstream &outfile);
void MOS(ifstream &infile, ofstream &outfile);
void READ(ifstream &infile);
void WRITE(ofstream &outfile);
void TERMINATE(ofstream &outfile);
void print_memory();

// Main function
int main() {
    cout << "\n[STEP 1] Starting OS Phase-I...\n";
    
    ifstream infile("input.txt");
    ofstream outfile("output.txt", ios::out); 

    if (!infile.is_open()) {
        cerr << "Cannot open 'input.txt' file. Verification failed.\n";
        return 1;
    }
    if (!outfile.is_open()) {
        cerr << "Cannot create 'output.txt' file.\n";
        return 1;
    }
    
    cout << "Files opened. Initiating Machine Cycle...\n";

    while (!infile.eof()) {
        LOAD(infile, outfile);
        if (infile.eof()) break; 
        STARTEXECUTION(infile, outfile);
    }

    cout << "\n[STEP 7] Executing final memory dump...\n";
    print_memory();

    cout << "[STEP 8] Terminating simulator.\n";
    infile.close();
    outfile.close();
    return 0;
}

// INIT function
void INIT() {
    cout << "\n[INIT] Wiping Main Memory and CPU Registers...\n";
    
    for(int i = 0; i < 4; i++) {
        IR[i] = '*';
        R[i] = '*';
    }
    IC = 0;
    C = false;
    SI = 0;

    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 4; j++) M[i][j] = '*';
    }
    cout << "INIT() completed.\n";
}

// LOAD function
void LOAD(ifstream &infile, ofstream &outfile) {
    while (infile.getline(Buffer, 41)) {
        // Skip empty or whitespace-only lines
        if (strlen(Buffer) == 0 || (strlen(Buffer) == 1 && isspace(Buffer[0]))) {
            continue;
        }
        
        if (strncmp(Buffer, "$AMJ", 4) == 0) {
            cout << "\n[LOAD] Found $AMJ. Starting new job.\n";
            INIT();
            m = 0;
        }
        else if (strncmp(Buffer, "$DTA", 4) == 0) {
            cout << "\n[LOAD] Found $DTA. Program loading finished.\n";
            cout << "[STEP 4] Transitioning to STARTEXECUTION phase...\n";
            break; 
        }
        else if (strncmp(Buffer, "$END", 4) == 0) {
            cout << "\n[LOAD] Found $END. Job Complete.\n";
            continue;
        }
        else {
            cout << "[LOAD] Parsing Program Card at block M[" << m << "]...\n";
            int buffer_len = strlen(Buffer);
            int buffer_index = 0;
            int limit = m + 10; 

            while (m < limit && buffer_index < buffer_len && m < 100) {
                for (int j = 0; j < 4; j++) {
                    if (buffer_index < buffer_len) M[m][j] = Buffer[buffer_index++];
                    else M[m][j] = '*'; // Fill empty bytes with *
                }
                m++;
            }

            // Block Allocation Logic
            if (m % 10 != 0) {
                m = ((m / 10) + 1) * 10;
                cout << "  -> [ALLOCATION] Short card. Pointer 'm' advanced to boundary " << m << "\n";
            } else {
                cout << "  -> [ALLOCATION] Full card. Pointer 'm' resting at boundary " << m << "\n";
            }
        }
    }
}

// STARTEXECUTION
void STARTEXECUTION(ifstream &infile, ofstream &outfile) {
    cout << "\n[STEP 5] Starting STARTEXECUTION Cycle...\n";
    IC = 0;

    while (true) {
        // 1. Fetch Instruction into IR
        for (int i = 0; i < 4; i++) {
            IR[i] = M[IC][i];
        }
        cout << "\n  [CPU] Fetched IR: " << IR[0] << IR[1] << IR[2] << IR[3] 
             << " from IC=" << (IC < 10 ? "0" : "") << IC << "\n";
        
        IC++; 

        char op1 = IR[0];
        char op2 = IR[1];

        // 2. Decode & Execute 
        if (op1 == 'G' && op2 == 'D') {
            SI = 1;
            MOS(infile, outfile);
        }
        else if (op1 == 'P' && op2 == 'D') {
            SI = 2;
            MOS(infile, outfile);
        }
        else if (op1 == 'H') {
            SI = 3;
            MOS(infile, outfile);
            break;
        }
        else {
            int operand = 0;
            if (isdigit(IR[2]) && isdigit(IR[3])) {
                operand = (IR[2] - '0') * 10 + (IR[3] - '0');
            } else {
                cerr << "  -> [CRITICAL] Invalid operand. System Crash.\n";
                break;
            }

            if (op1 == 'L' && op2 == 'R') {
                cout << "  -> [EXEC] LR" << operand << " : M[" << operand << "] -> R\n";
                for (int i = 0; i < 4; i++) R[i] = M[operand][i];
            }
            else if (op1 == 'S' && op2 == 'R') {
                cout << "  -> [EXEC] SR" << operand << " : R -> M[" << operand << "]\n";
                for (int i = 0; i < 4; i++) M[operand][i] = R[i];
            }
            else if (op1 == 'C' && op2 == 'R') {
                cout << "  -> [EXEC] CR" << operand << " : Compare R with M[" << operand << "]\n";
                C = true;
                for (int i = 0; i < 4; i++) {
                    if (R[i] != M[operand][i]) {
                        C = false;
                        break;
                    }
                }
                cout << "     [STATE] Toggle C = " << (C ? "1 (TRUE)" : "0 (FALSE)") << "\n";
            }
            else if (op1 == 'B' && op2 == 'T') {
                cout << "  -> [EXEC] BT" << operand << " : Branch if C=1\n";
                if (C) {
                    IC = operand;
                    cout << "     [STATE] Branch taken. IC <- " << IC << "\n";
                }
            }
            else {
                cerr << "  -> [ERROR] Unrecognized Opcode. System Crash.\n";
                break;
            }
        }
    }
}

// MOS - MASTER/KERNEL MODE
void MOS(ifstream &infile, ofstream &outfile) {
    switch(SI) {
        case 1: READ(infile); break;
        case 2: WRITE(outfile); break;
        case 3: TERMINATE(outfile); break;
        default: break;
    }
}

// KERNEL READ FUNCTION (SI = 1)
void READ(ifstream &infile) {
    int operand = (IR[2] - '0') * 10 + (IR[3] - '0');
    cout << "  -> [MOS] READ : Loading Data Card to M[" << operand << "]\n";
    
    if (infile.getline(Buffer, 41)) {
        int len = strlen(Buffer);
        int b_idx = 0;

        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 4; j++) {
                if (b_idx < len) M[operand + i][j] = Buffer[b_idx++];
                else M[operand + i][j] = '*'; 
            }
        }
    }
}

// KERNEL WRITE FUNCTION (SI = 2)
void WRITE(ofstream &outfile) {
    int operand = (IR[2] - '0') * 10 + (IR[3] - '0');
    cout << "  -> [MOS] WRITE : Outputting M[" << operand << "] to output.txt\n";
    
    int b_idx = 0;
    
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 4; j++) {
            Buffer[b_idx++] = M[operand + i][j];
        }
    }
    
    for (int i = 0; i < 40; i++) {
        if (Buffer[i] != '*') {
            outfile << Buffer[i];
        }
    }
    outfile << "\n";
}

// KERNEL TERMINATE FUNCTION (SI = 3)
void TERMINATE(ofstream &outfile) {
    cout << "  -> [MOS] TERMINATE : Appending blank lines to output.txt\n";
    outfile << "\n\n";
    // NOTE: Instead of calling LOAD() directly from here (which causes recursive stack buildup), 
    // the simulator returns to the main() loop which natively fetches the next job.
}

// PRINT MEMORY FUNCTION
void print_memory() {
    cout << "\n=========================================\n";
    cout << "          MAIN MEMORY DUMP (RAM)         \n";
    cout << "=========================================\n";
    
    for (int i = 0; i < 100; i++) {
        if (i < 10) cout << "M[0" << i << "]: ";
        else cout << "M[" << i << "]: ";

        for (int j = 0; j < 4; j++) cout << M[i][j];
        
        if ((i + 1) % 10 == 0) cout << "\n-------------------------";
        cout << "\n";
    }
}