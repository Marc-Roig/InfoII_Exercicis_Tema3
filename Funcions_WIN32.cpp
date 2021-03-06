// RESUM FUNCIONS MULTITASKING WIN32

// RECURSOS COMPARTITS

    // Seccio critica
        CRITICAL_SECTION cs;
        InitializeCriticalSection(&cs);

        EnterCriticalSection(&cs);
        LeaveCriticalSection(&cs);

        DelecteCriticalSection(&cs);

    // Mutex
        HANDLE hMu;
        hMu = CreateMutex ( 
                            NULL , // default security attributes,
                            NULL , // initially not owned
                            NULL   // unnamed
                          );

        WaitForSingleObject ( hMu, INFINITE ); // Wait infinite time for mutex 
        ReleaseMutex (hMu);

        CloseHandle (hMu);


    // Semaphore
        HANDLE hSem;

        hSem = CreateSemaphore(
                                NULL, // no atrib. seguridad
                                10  , // valor inicial
                                10  , // valor maximo
                                NULL, // sin nombre
                              );



        WaitForSingleObject ( hSem, INFINITE );
        // ... 
        ReleaseSemaphore ( hSem, 1, 0 );


// THREADS

    HANDLE hTh;

    hTh = CreateThread ( NULL, 0, (LPTHREAD_START_ROUTINE) Th_function_callback, NULL, CREATE_SUSPENDED, NULL);

    SetThreadPriority ( hTh, /*...*/ );
    ResumeThread ( hTh );


// PRIORITATS

    HANDLE hTh // handle thread
    SetThreadPriority ( hTh , THREAD_PRIORITY_LOWEST ); // LOWEST, BELOW_NORMAL, NORMAL, IDLE, ABOVE_NORMAL, HIGHEST, TIME_CRITICAL

    // Prioridades de proceso
    // REALTIME_PRIORITY_CLASS (24), HIGH_PRIORITY_CLASS (13), NORMAL_PRIORITY_CLASS (7/8), IDLE_PRIORITY_CLASS (4)
    
    // Handle de la propia funció
    HANDLE hProcess = GetCurrentProcess();
    // Ajustar prioridad del proceso
    SetPriorityClass(hProcess , HIGH_PRIORITY_CLASS);

// EVENTS

    // MANUAL EVENT

        HANDLE hEv;

        hEv = CreateEvent ( NULL , // no atrib. seguridad
                            FALSE, // reset automatico en el wait for object
                            FALSE, // estado inicial
                            NULL); // sin nombre

        setEvent(hEv);

        WaitForSingleObject(hEv, INFINITE);

    // TIME EVENT -- CALLBACK FUNCTION

        HANDLE hTEv;

        hTEv = timeSetEvent (   5000    ,     // period (ms)
                                0       ,     // max resolution
                                Th_Timer,     // Callback function
                                0       ,
                                TIME_ONESHOT, //modo disparo unico, TIME_PERIODIC para repetir
                            );

        WaitForSingleObject(hTEv, INFINITE);

    // TIME EVENT -- CALLBACK EVENT

        HANDLE hTEv;

        hTEv = timeSetEvent (   5000    ,     // period (ms)
                                0       ,     // max resolution
                                hEv_Timer,    // Callback event handle
                                0       ,
                                TIME_ONESHOT | TIME_CALLBACK_EVENT_SET, //modo disparo unico, TIME_PERIODIC para repetir
                            );

        WaitForSingleObject(hTEv, INFINITE);

// ALTRES

    // Funció callback timer
        void CALLBACK Th_timer (UINT uId, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);


    // Esperar a multiples objectes
        HANDLE arrEv [2] = { hEv_ex1, hEv_ex2 };
        ulong resp;

        resp = WaitForMultipleObjects ( 
                                        n_objects        ,
                                        arrEv            , // array de handlers
                                        false            , // esperarse a tots
                                        INFINITE 
                                       );

        switch (resp) {

            case WAIT_OBJECT_0 : 
                // ..
                break;
            case WAIT_OBJECT_0 + 1:
                // ..
                break;
            //..

        }



