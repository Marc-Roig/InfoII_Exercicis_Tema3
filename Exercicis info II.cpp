// 2013 - C1
{
    #define K 0.1
    #define REF_VAL 100

    ulong PSensor = 0x200; 
    ulong PSalida = 0x201; 

    double ref    = 0;
    double sensor = 0;

    bool program_running = true;

    HANDLE hEvT2;
    HANDLE hEvT3;
    HANDLE hTh2;
    HANDLE hTh3;

    // main Thread
    int main() {

        // hEv1 = CreateEvent()

        hTh2 = CreateThread(
                NULL, 0, (LPTHREAD_START_ROUTINE) Th2,
                NULL, CREATE_SUSPENDED, NULL);
        if (hTh2 == NULL) ErrorExit("Err. creacion thread\n");
        
        hTh3 = CreateThread(
                NULL, 0, (LPTHREAD_START_ROUTINE) Th3,
                NULL, CREATE_SUSPENDED, NULL);
        if (hTh3 == NULL) ErrorExit("Err. creacion thread\n");

        ResumeThread(hTh2);
        ResumeThread(hTh3);

        SetTimerPeriod(100 , hEvT2);
        SetTimerPeriod(0.05, hEvT3); // ??? ulong is integer, how to set event every 0.05 ms

        WaitForSingleObject(hTh2, INFINITE);
        KillTimer(hEvT2);
        WaitForSingleObject(hTh3, INFINITE);
        KillTimer(hEvT3);

        return 0;

    }

    DWORD WINAPI Th2(DWORD pParam) {

        static int cnt_uslots = 0;
        char a;

        while (program_running) {

            WaitForSingleObject(hEvT2, INFINITE);
            if (kbhit()) {
                a = getch();

                switch (a) {
                    case 'R':
                        ref = REF_VAL;
                        break;

                    case 27:
                        WritePort(PSalida, 0);
                        program_running = false;
                        break;

                    default:
                        ungetch(a);
                        break;
                }
            }
        }
    }


    DWORD WINAPI TH3(DWORD pParam) {

        while (program_running) {

            WaitForSingleObject(hEvT3, INFINITE);

            uchar sens = ReadPort(PSensor);
            uchar out = PControl(sens);

            WritePort(PSalida, out);

        }

    }

    char PControl(char sens) {

        double result = K*((double)ref - (double)sens);
        return (char)result;
    }
}