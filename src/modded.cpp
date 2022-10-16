#include "uCCBlibC.h"
#include <iostream>
#include "rs232.h"

using std::cout;
using std::cin;

void RS232_ReadData(int cport_nr, unsigned char* buf)
{
	int n = 0;
	while ((n = RS232_PollComport(cport_nr, buf, 4095)) == 0);

	if (n > 0)
	{
		buf[n] = 0;   /* null termination at the end of a string */

		for (int i = 0; i < n; i++)
		{
			if (buf[i] < 32)  /* replace unreadable control-codes by dots */
			{
				buf[i] = '.';
			}
		}
	}
}

int main(int argc, char * argv[])
{
	uCCBlibC u;
	u.BaudRateSettingCommand(uCCBlibC::BAUD_100k);

	cout << "Set filter for id = 0xFF (filter number and bank number equal 0):\n" <<
		u.AcceptanceFilter(0x0, 0x0, 0xFF) << '\n';

	/* Using RS232 library to send commands via serial to the device */

	u.SetEOL(uCCBlibC::EOL_CR);
	int cport_nr = RS232_GetPortnr("ttyACM1"),		/* argument depends on OS and port with UCCB device */
		bdrate = 115200;						/* 115200 baud */

	unsigned char buf[4096];

	char mode[] = { '8', 'N', '1', 0 };			/* 8 data bits, none parity, 1 stop bit, no handshaking */

	if (RS232_OpenComport(cport_nr, bdrate, mode))
	{
		cout << "Cannot open port\n";
		return -1;
	}

	/* close can communication */
	RS232_cputs(cport_nr, u.CloseCommand().data());
	RS232_ReadData(cport_nr, buf);
	cout << "Received: " << buf << '\n';

	/* get serial number */
	RS232_cputs(cport_nr, u.SerialNumberCommand().data());
	RS232_ReadData(cport_nr, buf);
	cout << "Received: " << buf << '\n';

	/* get hardware number */
	RS232_cputs(cport_nr, u.HardwareVersionCommand().data());
	RS232_ReadData(cport_nr, buf);
	cout << "Received: " << buf << '\n';

	/* open can communication */
	RS232_cputs(cport_nr, u.OpenCommand().data());
	RS232_ReadData(cport_nr, buf);
	cout << "Received: " << buf << '\n';

	/* send data frames */
	auto v = u.PrepareDataFrames(0x1, false, "canExampleMessage");
	for (auto i : v)
	{
		RS232_cputs(cport_nr, u.StringRepresentationOfFrame(i).data());
		RS232_ReadData(cport_nr, buf);
		cout << "Received: " << buf << '\n';
	}

	/* close can communication */
	RS232_cputs(cport_nr, u.CloseCommand().data());
	RS232_ReadData(cport_nr, buf);
	cout << "Received: " << buf << '\n';

	RS232_CloseComport(cport_nr);

	cout << "Press enter to exit...";
	cin.get();
	return 0;
}