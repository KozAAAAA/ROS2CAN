#include "uCCBlibC.h"
#include <iostream>
#include "rs232.h"
#include <filesystem>

namespace fs = std::filesystem;

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

bool checkIfUsbCan(uCCBlibC& u, const int cport_nr, const int bdrate, const char (&mode)[4], unsigned char (&buf)[4096])
{
		if (RS232_OpenComport(cport_nr, bdrate, mode))
		{
			std::cout << "Cannot open port\n";
			return false;
		}
		RS232_cputs(cport_nr, u.SerialNumberCommand().data());
		RS232_ReadData(cport_nr, buf);
		RS232_CloseComport(cport_nr);
		if(buf[0]=='N')
		{
			return true;
		}
		else 
		{
			return false;
		}
}

int main(int argc, char * argv[])
{
	int cport_nr;
	int bdrate = 115200;
	const char mode[] = { '8', 'N', '1', 0 };
	unsigned char buf[4096];

	uCCBlibC u;
	u.BaudRateSettingCommand(uCCBlibC::BAUD_100k);
	u.SetEOL(uCCBlibC::EOL_CR);

	std::string devPath = "/dev";
	std::string acmFileName = "ttyACM";

	if(fs::exists(devPath))
	{
		for (const auto & fileEntry : fs::directory_iterator(devPath))
		{
			auto fileName = fileEntry.path().filename().string();
			if(fileName.find(acmFileName) != std::string::npos)
			{	
				if(checkIfUsbCan(u, RS232_GetPortnr(fileName.c_str()), bdrate, mode, buf))
				{
					std::cout<<fileName<<" is the USB-CAN\n";
					acmFileName = fileName;
					break;
				}
				else
				{
					std::cout<<fileName<<" isn't the USB-CAN\n";
				}
			}
		}
	}
	else
	{
		std::cout<<devPath<<" does not exist\n";
	}

	cport_nr = RS232_GetPortnr(acmFileName.c_str());

	if (RS232_OpenComport(cport_nr, bdrate, mode))
	{
		std::cout << "Cannot open port\n";
		return -1;
	}

	/* close can communication */
	RS232_cputs(cport_nr, u.CloseCommand().data());
	RS232_ReadData(cport_nr, buf);
	std::cout << "Received: " << buf << '\n';

	/* get serial number */
	RS232_cputs(cport_nr, u.SerialNumberCommand().data());
	RS232_ReadData(cport_nr, buf);
	std::cout << "Received: " << buf << '\n';

	/* get hardware number */
	RS232_cputs(cport_nr, u.HardwareVersionCommand().data());
	RS232_ReadData(cport_nr, buf);
	std::cout << "Received: " << buf << '\n';

	/* open can communication */
	RS232_cputs(cport_nr, u.OpenCommand().data());
	RS232_ReadData(cport_nr, buf);
	std::cout << "Received: " << buf << '\n';

	/* send data frames */
	auto v = u.PrepareDataFrames(0x1, false, "canExampleMessage");
	for (auto i : v)
	{
		RS232_cputs(cport_nr, u.StringRepresentationOfFrame(i).data());
		RS232_ReadData(cport_nr, buf);
		std::cout << "Received: " << buf << '\n';
	}

	/* close can communication */
	RS232_cputs(cport_nr, u.CloseCommand().data());
	RS232_ReadData(cport_nr, buf);
	std::cout << "Received: " << buf << '\n';

	RS232_CloseComport(cport_nr);

	std::cout << "Press enter to exit...";
	std::cin.get();
	return 0;
}