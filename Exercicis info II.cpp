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

    CRITICAL_SECTION cs;

    // main Thread
    int main() {

        InitializeCriticalSection(&cs);

        hTh2 = CreateThread(
                NULL, 0, (LPTHREAD_START_ROUTINE) Th2,
                NULL, CREATE_SUSPENDED, NULL);
        if (hTh2 == NULL) ErrorExit("Err. creacion thread\n");
        
        hTh3 = CreateThread(
                NULL, 0, (LPTHREAD_START_ROUTINE) Th3,
                NULL, CREATE_SUSPENDED, NULL);
        if (hTh3 == NULL) ErrorExit("Err. creacion thread\n");

        hEvT2 = CreateEvent(NULL, FALSE, FALSE, NULL);
        hEvT3 = CreateEvent(NULL, FALSE, FALSE, NULL);

        ResumeThread(hTh2);
        ResumeThread(hTh3);

        SetTimerPeriod(100 , hEvT2);
        SetTimerPeriod(0.05, hEvT3); // ulong no pot tenir decimals, el professor diu que es un error del enunciat

        WaitForSingleObject(hTh2, INFINITE);
        KillTimer(hEvT2);
        WaitForSingleObject(hTh3, INFINITE);
        KillTimer(hEvT3);

        DeleteCriticalSection(&cs);

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
                        ref = REF_VAL; // Carregar valor de referencia
                        break;

                    case 27:
                        EnterCriticalSection(&cs);
                        WritePort(PSalida, 0)
                        LeaveCriticalSection(&cs);
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
    
    CRITICAL_SECTION cs;
    uchar dc = 0;

    bool program_running = true;
    // HANDLE hTh_Timer;
    cPwm obj_Pwm;

    class cPwm 
    {
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
        uchar cPwm(ushort prtAdr, uchar  prtBit) 
        {
            portAdr = prtAdr;
            portBit = prtBit;
            ready = true;
            return 0;
        }

        uchar SetDutyCycle(uchar pct) 
        {
            next_dc = pct;
            return 0;
        }

        void AttTim() 
        {

            switch(state) 
            {

                case (PWM_ON):
                    // If the dc value is 99 (100%) the output will never turn LOW: DC = 100%.
                    if (next_dc == 99 && count >= 100) {count = 0;}
                    // If the counter is greater than the dc value, turn off the otuput
                    else if (count >= new_dc) 
                    {

                        state = PWM_OFF;
                        uchar portStatus = ReadPort(portAdr);
                        WritePort(portAdr, portStatus & ~(1 << prtBit));

                    }

                    break;

                case (PWM_OFF):

                    // If the counter gets to 100 and the dc value is greater than 0
                    // Turn on the output. In case the dc value is 0 , the output will
                    // never turn HIGH: DC = 0%
                    if (next_dc == 0 && count >= 100) {count = 0;} 
                    else if (count >= 100) 
                    {

                        state = PWM_ON;
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

    int main () 
    {


        hTh_Tec = CreateThread(
                NULL, 0, (LPTHREAD_START_ROUTINE) Th2,
                NULL, CREATE_SUSPENDED, NULL);
        if (hTh_Tec == NULL) ErrorExit("Err. creacion thread\n");
        
        hTh_Pwm = CreateThread(
                NULL, 0, (LPTHREAD_START_ROUTINE) Th3,
                NULL, CREATE_SUSPENDED, NULL);
        if (hTh_Pwm == NULL) ErrorExit("Err. creacion thread\n");

        hEv_Tec = CreateEvent(NULL, FALSE, FALSE, NULL);
        hEv_Pwm = CreateEvent(NULL, FALSE, FALSE, NULL);

        ResumeThread(hTh_Tec);
        ResumeThread(hTh_Pwm);

        hTh_Timer = SetTimPer(1 , AttSysTim);

        WaitForSingleObject(hTh_Tec, INFINITE);
        WaitForSingleObject(hTh_Pwm, INFINITE);
        KillTimer(hTh_Timer);

        return 0;

    }

    void CALLBACK AttSysTim(INT param) 
    {

        static uint count = 0;

        setEvent(hEv_Pwm);    // Call pwm every 1 ms
 
        if (count++ >= 200)  // Call keyboard every 200 ms
        { 
            setEvent(hEv_Tec);
            count = 0;
        }
        
    }



    DWORD WINAPI Th_Tec(DWORD pParam) 
    {
        
        while (program_running) 
        {

            WaitForSingleObject(hEv_Tec, INFINITE);
            if (kbhit()) 
            {
                a = getch();
                EnterCriticalSection(&cs);
                switch (a) 
                {
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
                LeaveCriticalSection(&cs);
            }
        }

    }

    DWORD WINAPI Th_Pwm(DWORD pParam) 
    {

        while (program_running) 
        {

            WaitForSingleObject(hEv_Pwm, INFINITE);
            // PSO
            EnterCriticalSection(&cs);
            obj_Pwm.AttTim();
            LeaveCriticalSection(&cs);

        }

    }

}

// 2014 - C3
// Aquest es dels divertits... L'he fet rapid i no esta revisat!!!! 

// PORT DE COMMUNICACIO
// - Obrir el port amb  funcio Com_Open(...)
// - Els events RX i TX son els que s'activaran en les tasques Th_Tx i Th_Rx

// CAS GENERAL DE FUNCIONAMENT
// Quan es rebi un missatge, el flag evRX s'activara i el thread Th_Rx sen encarregara
// de guardar el caracter en la variable global response_message. En el cas de que shagin enviat 
// tots els caracters correctament es cridara la funcio ReadAdcCh(). 
// Quan la conversio del ADC shagui acabat comensara la sequencia per enviar tots els caracters
// de resposta. Aquesta sequencia es:
// - Enviar un primer caracter amb la funcio Com_SendByte(...)
// - Quan shagui enviat sactivara levent evTx, continuara lexecucio del Th_Tx i si queden caracters
//   per enviar (si sen han enviat 7 o el caracter es el de finalitzacio de trama 0x04) es tornara
//   a enviar el seguent caracter amb la funcio Com_SendByte(...).

// CAS ESPECIAL
// Quan sha acabat de rebre un missatge, la conversio anterior del ADC no sha acabat.
// En aquest cas es rebutjara la lectura del ADC i directament senvia el codi derror amb la mateixa 
// sequencia que abans.
{ 
    #define PORT_COM 2
    #define BPS_SPEED 9600

    HANDLE hCom;
    HANDLE evRx;
    HANDLE evTx;
    HANDLE hEv_Timer;

    uchar response_message[7]  = {0x01, '' , '', '', '' , '', 0x04};
    bool Com_Tx_busy  = false;
    bool ADC_busy     = false;

    // Seccio critica per accedir al missatge i varaibles Com_Tx_busy i ADC_busy
    CRITICAL_SECTION cs_message;

    int main() {

        InitializeCriticalSection(&cs_message); 

        hTh_Rx = CreateThread(
                NULL, 0, (LPTHREAD_START_ROUTINE) Th2,
                NULL, CREATE_SUSPENDED, NULL);
        if (hTh_Rx == NULL) ErrorExit("Err. creacion thread\n");
        
        hTh_Tx = CreateThread(
                NULL, 0, (LPTHREAD_START_ROUTINE) Th3,
                NULL, CREATE_SUSPENDED, NULL);
        if (hTh_Tx == NULL) ErrorExit("Err. creacion thread\n");

        evRx = CreateEvent(NULL, FALSE, FALSE, NULL);
        evTx = CreateEvent(NULL, FALSE, FALSE, NULL);

        hCom = Com_Open(PORT_COM, BPS_SPEED, evRx, evTx);

        ResumeThread(hTh_Rx);
        ResumeThread(hTh_Tx);

        WaitForSingleObject(hTh_Rx, INFINITE);
        WaitForSingleObject(hTh_Tx, INFINITE);

        DeleteCriticalSection(&cs_message);

        return 0;

    }

    // Cada cop que es rebi un caracter s'activa levent evRx
    // i sexecuta un loop del while.
    DWORD WINAPI Th_Rx(DWROD pParam) {
        
        uchar idx = 0;
        uchar message[7];

        while (1) {

            WaitForSingleObject(evRx, INFINITE);
            // Llegir caracter que s'ha rebut.
            uchar byte_in = Com_ReadRxByte(hCom);
            switch(idx) 
            {
                case 0: // El primer caracter ha de ser un 1
                    if (byte_in != 0x01) idx = 0;
                    else message[0] = 0x01;
                    break;

                case 4: // L'ultim caracter ha de ser un 4
                    if (byte_in == 0x04) {

                        EnterCriticalSection(&cs_message);
                        // Si sesta fent una conversio ADC, es descartara la mesura
                        // i senvia el missatge derror amb el format especificat al
                        // enunciat.
                        if (ADC_busy) 
                        {
                            response_message[1] = 0x0;
                            response_message[2] = message[2];
                            response_message[3] = message[3];
                            response_message[4] = 0x04;
                            Com_Tx_busy = true;       // Es comensara a enviar la trama
                            Com_SendByte(hCom, 0x01); // Comensar sequencia amb el primer caracter

                        } else if (!Com_Tx_busy) {
                            
                            response_message[1] = 0x01;
                            response_message[4] = message[2];
                            response_message[5] = message[3];  
                            ADC_busy = true;          // Es comensa la conversio ADC
                            ReadAdcCh(response_message[1], AdcCharacter);
                        }
                        // If Com Tx is busy discard message
                        LeaveCriticalSection(&cs_message);
                    }
                    idx = 0;
                    break;

                default:
                    message[idx++] = byte_in;
                    break;
            }


        }
    }

    // El flag evTx sactiva quan sha acabat denviar un caracter.
    // La funcio del thread es continuar enviant els caracters.
    // idx=-1 representa que lultim caracter enviat era el ultim 
    // de la trama a enviar. D'aquesta manera es para la sequencia.

    DWORD WINAPI Th_Tx(DWROD pParam) {

        uchar idx = 1;

        while (1) {

            WaitForSingleObject(evTx, INFINITE);

            // Si lultim caracter enviat era lultim de la trama o 
            // sha superat el tamany maxim de la trama no fer res.
            if ( idx == -1 || idx >= 7 ) {idx = 1; continue;}

            EnterCriticalSection(&cs_message);

            // Continuar la sequencia enviant el seguent bit.
            Com_SendByte(hCom, response_message[idx]);

            // Si el caracter enviat es el caracter de final de trama
            if ( response_message[idx] != 4 ) {
                Com_Tx_busy = false; // S'ha acabat la sequencia
                idx = -1;            // S'ha enviat lultim caracter
            } else idx++;

            LeaveCriticalSection(&cs_message);
        }

    }


    void AdcCharacter(uchar valorAdc) {

        EnterCriticalSection(&cs_message);
        ADC_busy = false;          // ADC conversion ended
        if (Com_Tx_busy) return;   // Discard value if Com Tx is sending message

        // Codificar el valor del ADC tal i com fica al enunciat.
        response_message[2] = (valorAdc >> 4  ) + 0x30;
        response_message[3] = (valorAdc & 0x0f) + 0x30;

        Com_Tx_busy = true;        // Es comensara a enviar la trama 
        LeaveCriticalSection(&cs_message);

        // Comensar sequencia denviar trama.
        Com_SendByte(hCom, 0x01);

    }

}