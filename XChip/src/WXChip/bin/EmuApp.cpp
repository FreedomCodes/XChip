/*

XChip - A chip8 lib and emulator.
Copyright (C) 2016  Rafael Moura

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see http://www.gnu.org/licenses/gpl-3.0.html.

*/

#error "EmuApp / WXChip are not ready for the plugin system"

#if defined(__linux__) || defined(__APPLE__)
#include <csignal>
#elif defined( _WIN32 )
#include <Windows.h>
#endif

// TODO: adapt to plugin system

#include <algorithm>
#include <utility>
#include <fstream>
#include <vector>

#include <XChip/Core/Emulator.h>
#include <XChip/Utility/Log.h>
#include <XChip/Utility/Assert.h>

static xchip::Emulator g_emulator;
void configure_emulator(const std::vector<std::string>& arguments);
/*******************************************************************************************
 *	-RES  window size: WidthxHeight ex: -RES 200x300 and -RES FULLSCREEN for fullscreen
 *	-CFQ  Cpu Frequency in hz ex: -CFQ 600
 *	-SFQ  Sound Tone in hz ex: -SFQ 400
 *	-COL  Color in RGB ex: -COL 100x200x255
 *      -BKG  Background color in RGB ex: -BKG 255x0x0
 *	-FPS  Frame Rate ex: -FPS 30
 *******************************************************************************************/

#if defined(__linux__) || defined(__APPLE__)
void signals_sigint(const int signum);
#elif defined(_WIN32)
bool _stdcall ctrl_handler(DWORD ctrlType);
#endif

/*********************************************************
 * SIGNALS:
 * SIGINT - set g_emulator exitflag
 * CTRL_EVENT: windows ConsoleCtrlEvents...
 *********************************************************/






int main(int argc, char **argv)
{
	using xchip::Emulator;
	using xchip::UniqueRender;
	using xchip::UniqueInput;
	using xchip::UniqueSound;
	using xchip::utility::make_unique;

	
#if defined(__linux__) || defined(__APPLE__) 

	if (signal(SIGINT, signals_sigint) == SIG_ERR)
	{
		std::cerr << "Could not install signal handler!" << std::endl;
		return EXIT_FAILURE;
	}

#elif defined(_WIN32)

	if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)ctrl_handler, true))
	{
		std::cerr << "Could not install Console Ctrl Handler: " << GetLastError() << std::endl;
		return EXIT_FAILURE;
	}

#endif

	if (argc < 2) 
	{
		std::cout << "No game to load..." << std::endl;
		return EXIT_SUCCESS;
	}




	

	if (!g_emulator.LoadRom(argv[1]))
		return EXIT_FAILURE;


	if(argc >= 3)
		configure_emulator(std::vector<std::string>(argv+2, argv+argc));


	while (!g_emulator.GetExitFlag())
	{
		g_emulator.UpdateSystems(); 
		g_emulator.HaltForNextFlag();		
		if (g_emulator.GetInstrFlag()) 			
			g_emulator.ExecuteInstr();
		if (g_emulator.GetDrawFlag())
			g_emulator.Draw();
	}


	return EXIT_SUCCESS;


}





xchip::utility::Color get_arg_rgb(const std::string& arg);
void res_config(const std::string& arg);
void cfq_config(const std::string& arg);
void sfq_config(const std::string& arg);
void col_config(const std::string& arg);
void bkg_config(const std::string& arg);
void fps_config(const std::string& arg);

void configure_emulator(const std::vector<std::string>& arguments)
{
	using xchip::utility::make_unique;

	std::cout << "\n\n\tconfigure_emulator: \n\n";


	using ConfigFunc = void(*)(const std::string&);
	using ConfigPair = std::pair<const char*, ConfigFunc>;

	ConfigPair configTable[] = 
	{
		{"-RES", res_config},
		{"-CFQ", cfq_config},
		{"-SFQ", sfq_config},
		{"-COL", col_config},
		{"-BKG", bkg_config},
		{"-FPS", fps_config}
	};



	const auto begin = arguments.cbegin();
	const auto end = arguments.cend();

	for(auto arg = begin; arg != end; ++arg)
	{
		bool validArg = std::any_of(std::begin(configTable), std::end(configTable),
						[&arg, &end](const ConfigPair& cpair) 
						{
							const auto argSize = (*arg).size();
							const auto cmdSize = strlen(cpair.first);

							if(argSize == cmdSize)
							{
								if(*arg == cpair.first) 
								{
									
									++arg;

									if(arg != end && (*arg)[0] != '-')
										cpair.second(*arg);

									else
										cpair.second(*(--arg));

									return true;
								}
							}

							else if(argSize > cmdSize)
							{
								
								if((*arg).compare(0, cmdSize, cpair.first) == 0)
								{
									cpair.second((*arg).substr(cmdSize, argSize - cmdSize));
									return true;
								}
							}

							return false;
						});


		if(!validArg)
			std::cout << "Unkown argument: " << *arg << std::endl;
	}


	std::cout << "\n\n\tconfigure_emulator done.\n\n";
}







void res_config(const std::string& arg)
{
	using xchip::utility::Vec2i;

	try
	{
		std::cout << "Setting Resolution..." << std::endl;

		if(!g_emulator.GetRender())
			throw std::runtime_error("null Render");


		if(arg == "FULLSCREEN")
		{
			if(!g_emulator.GetRender()->SetFullScreen(true))
				throw std::runtime_error("iRender internal error.");
		}

		else
		{
			const auto separatorIndex = arg.find('x');
		
			if(separatorIndex == std::string::npos)
				throw std::invalid_argument("missing the \'x\' separator for widthxheight");

			const Vec2i res(std::stoi(arg.substr(0, separatorIndex)), 
                                        std::stoi(arg.substr(separatorIndex+1, arg.size())) );

			g_emulator.GetRender()->SetWindowSize(res);
		}

		std::cout << "Render Resolution: " << g_emulator.GetRender()->GetWindowSize() << std::endl; 
		std::cout << "Done." << std::endl;

	}

	catch(std::exception& e)
	{
		std::cerr << "Failed to set Render resolution: " << e.what() << std::endl;
	}


}




void cfq_config(const std::string& arg)
{
	try
	{
		std::cout << "Setting Cpu Frequency..." << std::endl;		
		const auto cfq = std::stoi(arg);
		g_emulator.SetCpuFreq(cfq);
		std::cout << "Cpu Frequency: " << g_emulator.GetCpuFreq() << std::endl;
		std::cout << "Done." << std::endl;
	}
	catch(std::exception& e)
	{
		std::cerr << "Failed to set Cpu Frequency: " << e.what() << std::endl;
	}

}



void sfq_config(const std::string& arg)
{
	try
	{
		std::cout << "Setting Sound Tone..." << std::endl;
		const auto sfq = std::stof(arg);

		if(!g_emulator.GetSound())
			throw std::runtime_error("null Sound");

		g_emulator.GetSound()->SetSoundFreq(sfq);
		std::cout << "Sound Freq: " << g_emulator.GetSound()->GetSoundFreq() << std::endl;
		std::cout << "Done." << std::endl;
	}
	catch(std::exception& e)
	{
		std::cerr << "Failed to set Sound Tone: " << e.what() << std::endl;
	}
	
}






void col_config(const std::string& arg)
{
	try
	{
		std::cout << "Setting Render Color..." << std::endl;
		const auto color = get_arg_rgb(arg);

		if(!g_emulator.GetRender())
			throw std::runtime_error("null Render");

		if(!g_emulator.GetRender()->SetDrawColor(color))
			throw std::runtime_error("iRender internal error");

		std::cout << "Render Color: " << g_emulator.GetRender()->GetDrawColor() << std::endl;
		std::cout << "Done." << std::endl;
		
	}
	catch(std::exception& e)
	{
		std::cerr << "Failed to set Render Color: " << e.what() << std::endl;
	}

}








void bkg_config(const std::string& arg)
{
	try
	{
		std::cout << "Setting Background Color..." << std::endl;
		
		const auto color = get_arg_rgb(arg);

		if(!g_emulator.GetRender())
			throw std::runtime_error("null Render");

		if(!g_emulator.GetRender()->SetBackgroundColor(color))
			throw std::runtime_error("iRender internal error");

		std::cout << "Background Color: " << g_emulator.GetRender()->GetBackgroundColor() << std::endl;
		std::cout << "Done." << std::endl;
		
	}
	catch(std::exception& e)
	{
		std::cerr << "Failed to set Render Color: " << e.what() << std::endl;
	}

}





void fps_config(const std::string& arg)
{
	try
	{
		std::cout << "Setting Emulator FPS..." << std::endl;
		const auto fps = std::stoi(arg);
		g_emulator.SetFps(fps);
		std::cout << "Emulator FPS: " << g_emulator.GetFps() << std::endl;
		std::cout << "Done." << std::endl;
	}
	catch(std::exception& e)
	{
		std::cerr << "Failed to set Emulator FPS: " << e.what() << std::endl;
	}

}


xchip::utility::Color get_arg_rgb(const std::string& arg)
{
	const auto firstSeparator = arg.find('x');

	if (firstSeparator == std::string::npos)
		throw std::invalid_argument("missing the \'x\' separator");

	const auto secondSeparator = arg.find('x', firstSeparator + 1);

	if (secondSeparator == std::string::npos)
		throw std::invalid_argument("missing the second \'x\' separator");

	unsigned long rgb[3] =
	{
		std::stoul(arg.substr(0, firstSeparator)),
		std::stoul(arg.substr(firstSeparator + 1, secondSeparator)),
		std::stoul(arg.substr(secondSeparator + 1, arg.size()))
	};

	for (auto& col : rgb)
	{
		if (col > 255)
			col = 255;
	}

	return { (uint8_t)rgb[0], (uint8_t)rgb[1], (uint8_t)rgb[2] };
}







#if defined(__linux__) || defined(__APPLE__)
void signals_sigint(const int signum)
{
	std::cout << "Received sigint! signum: " << signum << std::endl;
	std::cout << "Closing Application!" << std::endl;
	g_emulator.SetExitFlag(true);
}

#elif defined(_WIN32)
bool _stdcall ctrl_handler(DWORD ctrlType)
{
	std::cout << "Received ctrlType: " << ctrlType << std::endl;
	std::cout << "Closing Application!" << std::endl;
	g_emulator.SetExitFlag(true);
	return true;
}
#endif



