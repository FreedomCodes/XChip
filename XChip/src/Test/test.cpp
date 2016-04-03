#include <XChip/Emulator.h>
#include <XChip/Utility/Log.h>
#include <XChip/SDL_MEDIA/SdlRender.h>
#include <XChip/SDL_MEDIA/SdlInput.h>
#include <XChip/SDL_MEDIA/SdlSound.h>


int main(int argc, char** argv)
{

	if (argc < 2) 
	{
		xchip::utility::LOG("No game to load...");
		return 0;
	}

	std::atexit([]()
	{
		xchip::utility::LOG("PRESS ANY KEY TO EXIT");
		std::cin.ignore();
	});

	static xchip::Emulator emulator;
	
	if (!emulator.Initialize(new(std::nothrow) xchip::SdlRender(), 
                             new(std::nothrow) xchip::SdlInput(),
                             new(std::nothrow) xchip::SdlSound()))
	{
		return 1;
	}

	else if (!emulator.LoadRom(argv[1]))
	{
		return 1;
	}

	while (! emulator.GetExitFlag())
	{
		emulator.HaltForNextFlag();
		emulator.UpdateSystems();
		
		if (emulator.GetInstrFlag())
			emulator.ExecuteInstr();
		
		if (emulator.GetDrawFlag())
			emulator.Draw();
	}

	return 0;


}


