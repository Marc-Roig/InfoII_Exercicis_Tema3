// 2013 - C1
{
    #define K 0.1
    #define REF_VAL 100

    ulong PSensor = 0x200; 
    ulong PSalida = 0x201; 

    double ref    = 0;

    bool program_running = true;

    HANDLE hEvT2;
    HANDLE hEvT3;
    HANDLE hTh2;
    HANDLE hTh3;

    // main Thread
    int main() {

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

// 2014 - C1
{   

    #define PWM_ON  1
    #define PWM_OFF 0
    HANDLE hEv_Tec;
    HANDLE hEv_Pwm;
    HANDLE hEv_Timer;

    HANDLE hTh_Tec;
    HANDLE hTh_Pwm;
    HANDLE hTh_Timer;

    uchar dc = 0;

    bool program_running = true;
    // HANDLE hTh_Timer;

    class cPwm {
    private:
        uchar portAdr;
        uchar portBit;

        // Utilitzo dos variables per al duty cycle. La variable dc es la que s'utilitza
        // dintre AttTim per saber si ha de passar destat ON a OFF. Degut a que es pot modificar
        // el dc directament quan funciona el PWM utilitzo la variable next_dc, que actualitza la
        // variable dc quan s'acaba el periode. Sino podria donarse el seguent cas:
        //      -----------  ---   ----------------
        //      |         |  | |   |              |
        //      |         |  | |   |              |
        //-------         ---- -----              --------
        //      0        DC   DC'  0              DC'
        // D'aquesta manera s'eviten pics inesperats dintre cada periode del PWM quan sintrodueix un
        // valor de dc més gran que lanterior durant el periode OFF.
        // Aixo domes afecta en el periode OFF del PWM. Ja que si durant el periode ON sintrodueix
        // un valor mes petit o més gran de dc no genera aquesta problematica.
        uchar dc = 0;
        uchar next_dc = 0;
        uchar state = 0;
        uchar count = 0;

        bool ready = false;

    public:
        // cPwm(ushort portAdr, uchar portBit) {}
        uchar cPwm(ushort prtAdr, uchar  prtBit) {
            portAdr = prtAdr;
            portBit = prtBit;
            ready = true;
            return 0;
        }

        uchar SetDutyCycle(uchar pct) {
            next_dc = pct;
            return 0;
        }

        void AttTim() {

            switch(state) {

                case (PWM_ON):
                    // If the dc value is 99 (100%) the output will never turn LOW: DC = 100%.
                    if (next_dc == 99 && count >= 100) {count = 0;}
                    // If the counter is greater than the dc value, turn off the otuput
                    else if (count >= new_dc) {

                        state = OFF;
                        uchar portStatus = ReadPort(portAdr);
                        WritePort(portAdr, portStatus & ~(1 << prtBit));

                    }

                    break;

                case (PWM_OFF):

                    // If the counter gets to 100 and the dc value is greater than 0
                    // Turn on the output. In case the dc value is 0 , the output will
                    // never turn HIGH: DC = 0%
                    if (next_dc == 0 && count >= 100) {count = 0;} 
                    else if (count >= 100) {

                        state = ON;
                        count = 0;
                        dc = next_dc;

                        uchar portStatus = ReadPort(portAdr);
                        WritePort(portAdr, portStatus | (1 << prtBit));

                    }

                    break;

                default:
                    break;

            }

            count++;
        }


    }

    int main () {


        hTh_Tec = CreateThread(
                NULL, 0, (LPTHREAD_START_ROUTINE) Th2,
                NULL, CREATE_SUSPENDED, NULL);
        if (hTh_Tec == NULL) ErrorExit("Err. creacion thread\n");
        
        hTh_Pwm = CreateThread(
                NULL, 0, (LPTHREAD_START_ROUTINE) Th3,
                NULL, CREATE_SUSPENDED, NULL);
        if (hTh_Pwm == NULL) ErrorExit("Err. creacion thread\n");

        ResumeThread(hTh_Tec);
        ResumeThread(hTh_Pwm);

        hTh_Timer = SetTimPer(1 , AttSysTim);

        WaitForSingleObject(hTh_Tec, INFINITE);
        WaitForSingleObject(hTh_Pwm, INFINITE);
        KillTimer(hTh_Timer);

        return 0;

    }

    void CALLBACK AttSysTim(INT param) {

        static uint count = 0;

        setEvent(hEv_Pwm);    // Call pwm every 1 ms
 
        if (count++ >= 200) { // Call keyboard every 200 ms
            setEvent(hEv_Tec);
            count = 0;
        }
        
    }



    DWORD WINAPI Th_Tec(DWORD pParam) {
        
        while (program_running) {

            WaitForSingleObject(hEv_Tec, INFINITE);
            if (kbhit()) {
                a = getch();

                switch (a) {
                    case '+':
                        obj_Pwm.SetDutyCycle(++dc);
                        break;

                    case '-':
                        obj_Pwm.SetDutyCycle(--dc);
                        break;

                    case 0:
                        obj_Pwm.SetDutyCycle(0);
                        break;

                    case 27:
                        program_running = false;
                        break;

                    default:
                        ungetch(a);
                        break;
                }
            }
        }

    }

    DWORD WINAPI Th_Pwm(DWORD pParam) {

        while (program_running) {

            WaitForSingleObject(hEv_Pwm, INFINITE);

            obj_Pwm.AttTim();

        }

    }

}