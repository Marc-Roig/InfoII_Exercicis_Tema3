#define KP 100

bool end = false;

HANDLE hTh_ADC;
HANDLE hTh_Rx;
HANDLE hTh_Tx;
HANDLE hTh_Alarma;
HANDLE hTh_PID;
HANDLE hTh_PWM;

HANDLE hEv_ADC;
HANDLE hEv_Rx;
HANDLE hEv_Tx;
HANDLE hEv_Alarma;
HANDLE hEv_PID;
HANDLE hEv_PWM;

CRITICAL_SECTION cs_vel;
CRITICAL_SECTION cs_dc;

struct Velocitat {

    uchar real;
    uchar ref;

    uchar values_updated = true;
};
Velocitat vel;

uchar DutyCycle;

void main() {

    InitializeCriticalSection(&cs_vel);
    InitializeCriticalSection(&cs_dc );

    hTh_ADC    = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE) Th_ADC   , NULL, CREATE_SUSPENDED, NULL);
    hTh_Rx     = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE) Th_Com_Rx, NULL, CREATE_SUSPENDED, NULL);
    hTh_Tx     = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE) Th_Com_Tx, NULL, CREATE_SUSPENDED, NULL);
    hTh_Alarma = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE) Th_Alarma, NULL, CREATE_SUSPENDED, NULL);
    hTh_PID    = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE) Th_PID   , NULL, CREATE_SUSPENDED, NULL);
    hTh_PWM    = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE) Th_PWM   , NULL, CREATE_SUSPENDED, NULL);

    hEv_ADC    = CreateEvent( NULL, FALSE, FALSE, NULL );
    hEv_Rx     = CreateEvent( NULL, FALSE, FALSE, NULL );
    hEv_Tx     = CreateEvent( NULL, FALSE, FALSE, NULL );
    hEv_Tx_2s  = CreateEvent( NULL, FALSE, FALSE, NULL );
    hEv_Alarma = CreateEvent( NULL, FALSE, FALSE, NULL );
    hEv_PID    = CreateEvent( NULL, FALSE, FALSE, NULL );
    hEv_PWM    = CreateEvent( NULL, FALSE, FALSE, NULL );

    timeSetEvent (2000, 0, hEv_Tx_2s , 0, TIME_PERIODIC | TIME_CALLBACK_EVENT_SET);
    timeSetEvent ( 500, 0, hEv_Alarma, 0, TIME_PERIODIC | TIME_CALLBACK_EVENT_SET);
    timeSetEvent ( 100, 0, hEv_PID   , 0, TIME_PERIODIC | TIME_CALLBACK_EVENT_SET);
    timeSetEvent (  10, 0, hEv_PWM   , 0, TIME_PERIODIC | TIME_CALLBACK_EVENT_SET);

    SetThreadPriority( hTh_Tx     , PRIOR_2);
    SetThreadPriority( hTh_Rx     , PRIOR_3);
    SetThreadPriority( hTh_ADC    , PRIOR_3);
    SetThreadPriority( hTh_Alarma , PRIOR_4);
    SetThreadPriority( hTh_PID    , PRIOR_5);
    SetThreadPriority( hTh_PWM    , PRIOR_6);

    ResumeThread(hTh_ADC);
    ResumeThread(hTh_Rx);
    ResumeThread(hTh_Tx);
    ResumeThread(hTh_Alarma);
    ResumeThread(hTh_PID);
    ResumeThread(hTh_PWM);

    // Esperar a que tots els threads acabin
    HANDLE threads[5] = { hTh_Rx, hTh_Tx, hTh_Alarma, hTh_PID, hTh_PWM };
    WaitForMultipleObjects( 5, threads, true, INFINITE );

    DeleteCriticalSection(&cs_vel); 
    DeleteCriticalSection(&cs_dc);

}

DWORD WINAPI Th_ADC(DWORD pParam) {

    while (!end) {

        WaitForSingleObject(hEv_ADC, INFINITE);

        EnterCriticalSection(&cs_vel);
        vel.real = RdAdc();
        LeaveCriticalSection(&cs_vel);

    }

}

DWORD WINAPI Th_Com_Rx(DWORD pParam) {

    uchar byte;

    uchar vel = 0;
    uchar chkSum = 1;

    uchar idx = 0;
    while (!end) {

        WaitForSingleObject(hEv_Rx, INFINITE);

        byte = rec232();

        switch (idx) {

            case 0:
                if (byte == 1) idx++;
                break;

            case 1:
                vel += (byte - 0x30)*100;
                chkSum += byte;
                idx++;
                break;

            case 2:
                vel += (byte - 0x30)*10;
                chkSum += byte;
                idx++;
                break;

            case 3:
                vel += (byte - 0x30);
                chkSum += byte;
                idx++;
                break;

            default:
                if (byte == chkSum) {
                    EnterCriticalSection(&cs_vel);
                    vel.ref = vel;
                    val.values_updated = true;
                    LeaveCriticalSection(&cs_vel);
                }
                break;


        }



    }

}

DWORD WINAPI Th_Com_Tx(DWORD pParam) {

    HANDLE events[2] = {hEv_Tx, hEv_Tx_2s}; 
    ulong resp;

    // Trama
    uchar trama[5] = {1,0,0,0,0};
    char idx = 0;

    while (!end) {

        WaitForMultipleObjects(2, events, false, INFINITE);

        switch (resp) {

            // Continuar enviant trama
            case WAIT_OBJECT_0:

                if (idx < 5) snd232(trama[idx]);
                idx++;

                break;

            case WAIT_OBJECT_1:

                idx = 0;

                EnterCriticalSection(&cs_vel);
                uchar vel_real = vel.real;
                LeaveCriticalSection(&cs_vel);

                trama[1] = 0x30 + (vel_real / 100)      ;  
                trama[2] = 0x30 + (vel_real / 10 ) % 10 ;  
                trama[3] = 0x30 + (vel_real      ) % 10 ;
                trama[4] = 1 + trama[1] + trama[2] + trama[3];  

                snd232(0x01);

                break;

            default:
                continue;
                break;

        }

    }

}

DWORD WINAPI Th_Alarma(DWORD pParam) {

    uchar last_vel_read;
    bool error = false;

    while (!end) {

        WaitForSingleObject(hEv_Alarma, INFINITE);

        EnterCriticalSection(&cs_vel);
        if (!vel.values_updated) error = true;
        LeaveCriticalSection(&cs_vel);

        if (error) {
            EnterCriticalSection(&cs_dc);
            DutyCycle = 0;
            LeaveCriticalSection(&cs_dc);
        }

    }

}

DWORD WINAPI Th_PID(DWORD pParam) {

    while (!end) {
        WaitForSingleObject(hEv_PID, INFINITE);

        // Aplicar control proporcional
        EnterCriticalSection(&cs_vel);

        uchar out = (vel.ref - vel.real)*KP;
        out = (out < 0  ) ? 0   : out;
        out = (out > 255) ? 255 : out;

        LeaveCriticalSection(&cs_vel);

        EnterCriticalSection(&cs_dc);
        DutyCycle = out;
        LeaveCriticalSection(&cs_dc);

        StartADC(); // Cridar conversor AD per actualitzar valor de velocitat

    }

}

DWORD WINAPI Th_PWM(DWORD pParam) {

    uchar idx = 0;

    while (!end) {

        // No es dona informacio per implementar-ho

    }

}