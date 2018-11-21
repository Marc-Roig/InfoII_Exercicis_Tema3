struct str_LLC_resp {
	uchar datalen;
	uchar* data;
};

// CAPA 2A - MAC
class MAC() {
public:

	void SendFrm(uchar datalen, uchar* data);
	bool isResponseRecievedOK();
	void ReadRecievedFrm(uchar* datalen, uchar* data);

};

volatile MAC mac = MAC();

// CAPA 2B - LLC
class LLC() {

public:

	bool busy, ResponseAvailable, ResponseFailed;
	str_LLC_resp messageToSend, response;
	char nIntentos;

	LLC() {

		busy = false;
		ResponseFailed = false;
		ResponseAvailable = false;
		nIntentos = 0;

		// Seria millor cridarla cada cop que s'ha enviat un missatge sense ser periodica
		SetTim(100, AttTimer, 1); // Cada 100ms, crida AttTimer, repeteix per sempre.

	}

	void SendFrame(uchar datalen, uchar* data, uchar num_reintentos) {

		if (busy) return;

		nIntentos = num_reintentos % 10 + 1; // Un intent mes tots els altres reintents 
		messageToSend.datalen = datalen % 10;
		messageToSend.data = data;

		busy = true;

	}

	uchar FrameResponse_GetStatus() {

		if (busy || !ResponseAvailable) return 0;
		else if (ResponseAvailable) return 1;
		else return 2; // ResponseFailed = true

	}

	void FrameResponse_GetFrm(str_LLC_resp * r) {

		if (!ResponseAvailable || ResponseFailed) return;

		r->datalen = response.datalen;
		for (int i = 0; i < response.datalen; i++) {
			r->data[i] = response.data[i];
		}

		ResponseAvailable = false;

	}

};

volatile LLC llc = LLC();

void AttTimer() {

	if (!llc.busy || llc.ResponseFailed) return;

	if (mac.isResponseOK()) {

		mac.ReadRecievedFrm(llc.messageToSend.datalen, llc.messageToSend.data);
		llc.ResponseFailed = false;
		llc.ResponseAvailable = true;
		llc.busy = false;
		llc.nIntentos = 0;

	} else {

		llc.nIntentos--;
		if (llc.nIntentos <= 0) llc.ResponseFailed = true; 

	}

}
