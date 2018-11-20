/// MARC ROIG CAMPOS

/// ------------------------------ ///
/// EXERCICIS INFORMATICA II C/C++ ///
/// ------------------------------ ///

// EXERCICI 1
{
	struct est1 {

		char   var1;
		short* var2;
		short  var3[50];

	}

	int main(void) {

		est1  miestruct;
		short buf[30];

		miestruct.var1 = 3;
		for (int i = 0; i < 50; i++) var3[i] = i + 10;
		miestruct.var2 = buf;


	}
}

// EXERCICI 2
{
	struct est1 {

		char   var1;
		short* var2;
		short  var3[50];

	}

	int main(void) {

		est1  arr[10];
		short buf[10];

		for (int i = 0; i < 10; i++) {
			arr[i].var1 = 0x20 + i;
			arr[i].var2 = &buf[i]; // També es podria ficar (buf+i);
			for (int j = 0; j < 50; j++) arr[i].var3[j] = 0x30;
		}

		
	}
}

// EXERCICI 3
{
	typedef struct {
		char a;
		char b;
	} str;

	str* ptarray[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	int numstr = 0;

	void fun1(){

		for (int i = 0; i < 10; i++) ptarray[i] = new str;

	}

	void fun2() {

		for (int i = 0; i < 10; i++) {

			if (ptarray[i]) delete ptarray[i];
			ptarray[i] = 0;

		}

	}
}

// EXERCICI 4
{
    class Clase_ex4 {

    public:

        int (*var1)(char);

        Clase_ex4 (int (*param)(char)) {
            var1 = param;
        }

        void execute(char param) {
            var1(param);
        }

    };

    int main(void) {
        Clase_ex4* clase_ex4 = new Clase_ex4(exe1);
        clase_ex4->execute(0x30);
    }
}

// EXERCICI 5
{
    int a;
    int b = 1, c = 7, d = 5;
    
    // A int can hold a number between -32767 and 32767. Given the worst
    // case scenario where b, c and d equal 100, the biggest possible result
    // is 2025. The values are multiplied by 10 to get a decimal place,
    // then the operation is computed and the result divided again by 10. 
    // The int variable can still hold the maximum value (20250)
    // and the 'truncation' effects are minimized

    a = b*10 + (c*10)*d/4;
    
    // If decimal place is bigger or equal to 5 round up
    // if its smaller it will be truncated int the /10 division;
    if ((a % 10) >= 5) a = a - (a%10) + 5;

    a = (a >=  1000) ?  1000: a;
    a = (a <= -1000) ? -1000: a;
    
    a /= 10;

}

// EXERCICI 6 
{   

    int main(void) {

        int  *file  = 0x1000;
        char *video = 0x5000;

        for (int row = 0; row < 32; row++) {

            int row_offset = row*64;

            for (int col = 0; col < 64; col++) {

                // Get pixel value from video array
                char video_pixel = *(video + row_offset + col);

                unsigned int file_pixel = *(file + row_offset + col);

                // Get RGB colors
                char B =  file_pixel        & 0b00011111;
                char G = (file_pixel >> 5 ) & 0b00001111;
                char R = (file_pixel >> 11) & 0b00011111;

                if (B > 0b11 ) B = 0b11;
                if (G > 0b111) G = 0b111;
                if (R > 0b111) R = 0b111;

                // Encode the RGB values into the file format
                *(file + row_offset + col) = B | (G << 2) | (R << 5);

            }
        }
    }

}

// EXERCICI 7 
{

    void CpyImg(uchar* pImgBuf, char szx, char szy, uchar* pPantBuf, char pant_x, char pant_y) {

        // Check if image to copy is inside the 64x32 image
        if (pant_x < 0) pant_x = 0;
        if (pant_y < 0) pant_y = 0;

        if (pant_x > 63) pant_x = 63;
        if (pant_y > 31) pant_y = 31;

        if (pant_x + szx > 63) szx = 63-pant_x;
        if (pant_y + szy > 31) szy = 31-pant_y;


        for (int row = 0; row < szy; row++) {

            int row_offset_img  =  row*szx;
            int row_offset_pant = (row+pant_y)*64;

            for (int col = 0; col < szx; col++) {

                *(pPantBuf + row_offset_pant + pant_x + col) = *(pImgBuf + row_offset_img + col);

            }

        }

    }

    int main(void) {

        uchar* img_buff;
        ulong* pszx, pszy;

        for (int i = 0; i < 20; i++) {
            RdFile(i+1, img_buff, pszx, pszy);
            CpyImg(img_buff, *pszx, *pszy, Dpant, array[i].x, array[i].y);
        }

    }
    
}

// EXERCICI 8
{

    class Buffer {
    public:

        u_char* arr;

        u_long p1 = 0, p2 = 0;
        u_long buff_length;

        bool is_empty = true;
        bool is_full  = false;

        Buffer(int buffer_length) {

            arr = new u_char[buffer_length];
            buff_length = buffer_length;

        }

        ~Buffer() {
            delete[] arr;
        }

        void WrVal(char a) {

            if (is_full) return;

            arr[p2] = a;

            p2 = (p2 + 1) % buff_length;

            is_empty = false;
            if (p2 == p1) is_full = true;

        }

        char RdVal() {

            if (is_empty) return NULL;

            u_char return_value = arr[p1];

            p1 = (p1 + 1) % buff_length;

            is_full = false;
            if (p1 == p2) is_empty = true;

            return return_value;

        }

        ulong GetFilledBytes() {

            if (is_full) return buff_length;
            int filledBytes = p2-p1;
            if (filledBytes < 0) filledBytes = -filledBytes;

            return filledBytes;
        }

    };

}

// EXERCICI PL1
{
    char data[100];
    char data_index = 0;

    void AttRx(char rxbyte) {

        switch (rxbyte) {

            // Start of frame
            case 1: 

                if (data_index == 0) data[data_index++] = 1;
                break;

            // End of frame
            case 2: 

                data[data_index++] = 2;

                if (is_data_correct()) AppPtr();

                data_index = 0; // Restart data array

                break;

            default:

                data_index[data_index++] = rxbyte;

        }

        if (data_index >= 100) data_index = 0; //Reset data[], el missatge es massa llarg per ser valid

    }

    bool is_data_correct() {

        
        // Com a minim ha de tenir el caracter dinici, el node de desti, 
        // longitut de dades, checksum i caracter de fi de trama
        if (data_index < 5) return false;

        // El primer byte ha de ser el dinici
        if (data[0] != 1)   return false;

        // Els possibles valors de node estan entre [0..15]
        unsigned char node = data[1] - 0x10;
        if (node < 0 || node > 15) return false;

        // Els possibles valors de dlen estan entre [0..30]
        unsigned char data_length = data[2] - 0x10;
        if (data_length < 0 || data_length > 30) return false;

        // Els valors de data han de ser caracters ASCII
        for (int i = 3; i < data_length+3; i++) {
            if (data[i] < '0' || data[i] > 'Z') return false;
        }

        unsigned char checksum = 0; // Variable per a comprovar si el checksum es correcte 
        for (int i = 0; i < data_index-1; i++) checksum += data[i];
        if (checksum != data[data_index-1]-0x10) return false;
    
        return true;
    }

    void SendResponse(char destination, char datalen, char* databody) {

        char TxBuf[100];

        u_char buff_len = 5 + datalen; // Byte inici + node + datalen + data + Checksum + Byte final 
        if (buff_len > 100) return;

        // Byte inici
        TxBuf[0] = 1;

        // Node Desti
        TxBuf[1] = destination + 0x10;

        // Datalen
        TxBuf[2] = data_length + 0x10;

        // Data
        for (int i = 0; i < datalen; i++) {
            TxBuf[i+3] = databody[i];
        }

        // Checksum (7 bits??)
        u_char checksum = 0; 
        for (int i = 0; i < datalen + 3; i++) {
            checksum += TxBuf[i];
        }

        TxBuf[buff_len - 2] = (checksum + 0x10) & 0b01111111; // Elimino el bit mes significatiu per a que sigui de 7 bits (????)

        TxBuf[buff_len - 1] = 2;
        SendFrame(TxBuf);

    }

}

// EXERCICI PL2
{
    char keyboard[4][4] = {
        {0, 1 , 2 ,'-'},
        {3, 4 , 5 ,'-'},
        {6, 7 , 8 ,'-'},
        {9,'.','R','-'}
    };

    char Tref = 0;
    short duty = 0;

    char* portS = 0x100;
    char* portE = 0x101;
    char* portP = 0x102; //PWM

    void att_timer_10hz() {

        static char comptadorADC = 0; // Comptador crida per actualitzar valor de temperatura
        char valor; // Utilitzat per a lectura del teclat

        if (comptadorADC++ >= 100) { // Actualitzar el valor de temperatura cada 10 segons
            comptadorADC = 0;
            IniConvADC(0, updateTsens);
        }

        // Read keyboard
        for (int i = 0; i < 4; i++) {

            WritePort8bits(portE, 0b1000>>i); //Activate Row
            ReadPort8bits(portS, &valor); // Read Columns

            if      (valor & 0x1) store_key_pressed(keyboard[i][3]);
            else if (valor & 0x2) store_key_pressed(keyboard[i][2]);
            else if (valor & 0x3) store_key_pressed(keyboard[i][1]);
            else if (valor & 0x4) store_key_pressed(keyboard[i][0]);
            else continue; // If none of the conditions are true end this loop and go for the next one;

            break; //One of the ifs conditions was true, break the for loop. The pressed key has been found.

        }

    }

    void store_key_pressed(char key) {

        static char sequence_index = 0;
        static char data[2];

        // La sequencia esta formada per dos numeros i la tecla Return. Així doncs si aquesta sequencia no es compleix
        // es reseteja i si es compleix s'actualitza el valor Trej.
        switch (sequence_index) {

            case 0:
                data[sequence_index++] = key;
                break;

            case 1:
                data[sequence_index++] = key;
                break;

            case 2:

                sequence_index = 0;
                if (key != 'R') return;

                Tref = data[0]*10 + data[1];

                break;

        }

    }

    short P(short kp, short error) {

        float result = kp*error;

        if (result >  100) result =  100;
        if (result < -100) result = -100;

        return (short)result;

    }

    void updateTsens(char ADCVal) {

        char error = Tref-ADCVal;
        if (error < 0) error = 0;

        duty = (char)P(1.5, error);

    }

    void controlPWM() {

        static int counts = 0;
        uchar val; 

        if (counts == 0) {
            ReadPort8bits(portP, &val);
            val = val | 0x1;
            WritePort8bits(portP, &val);
        }
        else if (counts >= duty) {
            ReadPort8bits(portP, &val);
            val = val & 0xFE;
            WritePort8bits(portP, &val);
        }
        
        counts++;
        if (counts >= 100) counts = 0;

    }

    int main(void) {

        InicializarPorts();
        InicializarTimer();
        InicializarADC();

        Set_periodic_timer(100, att_timer_10hz);
        Set_periodic_timer(10 , controlPWM);

        // PWM OUTPUT TO 0
        uchar val; 
        ReadPort8bits(portP, &val); // No tinc clar si fa falta llegir primer el que hi ha i aplicar la mascara
        val = val & 0xFE;
        WritePort8bits(portP, &val);

    }
}

// PL3
// https://www.researchgate.net/post/How_to_convert_Ti_integral_time_and_Td_derivative_time_to_I_and_D_parameter_in_function_block_parameter_for_PID_controller_SIMULINK
{
    short PI(short kp, short Ti, short error, short error_ant) {

        float result = kp*error + (kp/Ti)*(error+error_ant);

        result = (result >  100) ?  100 : result;
        result = (result < -100) ? -100 : result;

        return (short)result;

    }

    void updateTsens(char ADCVal) {

        static char last_error = 0;
        char error = Tref - ADCVal;
        if (error < 0) error = 0;

        duty = PI(1.5, 100, error, last_error);

        last_error = error;

    }
}
