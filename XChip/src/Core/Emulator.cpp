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

#include <XChip/Core/Emulator.h>
#include <Utix/Log.h>
#include <Utix/ScopeExit.h>




namespace xchip {

using namespace utix;


// local functions declarations
inline void init_emu_timers(Timer& instrTimer, Timer& frameTimer, Timer& chDelayTimer);
inline bool init_cpu_manager(CpuManager& m_manager);






Emulator::Emulator() noexcept
{
	Log("Creating XChip Emulator object...");
}



Emulator::~Emulator()
{
	if(m_initialized) 
		this->Dispose();

	Log("Destroying XChip Emulator object...");
}





bool Emulator::Initialize() noexcept
{
	if (m_initialized) 
		this->Dispose();

	const auto cleanup = MakeScopeExit([this]() noexcept {
		if (!this->m_initialized)
			this->Dispose();
	});

	init_emu_timers(m_instrTimer, m_frameTimer, m_chDelayTimer);

	if(init_cpu_manager(m_manager))
	{
		CleanFlags();
		m_initialized = true;
		return true;
	}

	return false;
}




bool Emulator::Initialize(UniqueRender&& render, UniqueInput&& input, UniqueSound&& sound) noexcept
{
	using std::move;

	if (m_initialized) 
		this->Dispose();

	const auto cleanup = MakeScopeExit([this]() noexcept {
		if (!this->m_initialized)
			this->Dispose();
	});

	m_renderPlugin = move(render);
	m_inputPlugin = move(input);
	m_soundPlugin = move(sound);

	init_emu_timers(m_instrTimer, m_frameTimer, m_chDelayTimer);

	if(init_cpu_manager(m_manager))
	{
		// try to init all interfaces before returning something...
		if (( InitRender() & InitInput() & InitSound()) ) 
		{
			CleanFlags();
			m_initialized = true;
			return true;
		}
	}
	
	return false;
}





void Emulator::Dispose() noexcept
{
	m_manager.Dispose();
	m_initialized = false;
}





void Emulator::HaltForNextFlag() const
{
	if (! m_manager.GetFlags(Cpu::DRAW | Cpu::INSTR))
	{
		const auto instrRemain = m_instrTimer.GetRemain();
		const auto frameRemain = m_frameTimer.GetRemain();
		utix::Sleep((instrRemain < frameRemain) ? instrRemain : frameRemain);
	}
}



void Emulator::UpdateTimers()
{
	if (!m_manager.GetFlags(Cpu::INSTR) && m_instrTimer.Finished())
	{
		m_manager.SetFlags(Cpu::INSTR);
		m_instrTimer.Start();
	}

	if (!m_manager.GetFlags(Cpu::DRAW) && m_frameTimer.Finished())
	{
		m_manager.SetFlags(Cpu::DRAW);
		m_frameTimer.Start();
	}

	

	if (m_chDelayTimer.Finished())
	{
		auto& delayTimer = m_manager.GetCpu().delayTimer;
		if (delayTimer)
			--delayTimer;

		m_chDelayTimer.Start();
	}
}


 
void Emulator::UpdateSystems()
{
	ASSERT_MSG(!m_manager.GetFlags(Cpu::BAD_RENDER), "BAD RENDER");
	ASSERT_MSG(!m_manager.GetFlags(Cpu::BAD_INPUT),  "BAD INPUT");
	m_manager.GetRender()->UpdateEvents();
	m_manager.GetInput()->UpdateKeys();
	this->UpdateTimers();
}




void Emulator::CleanFlags()
{
	// clean flags but keep bad flags.
	const auto badFlags = m_manager.GetFlags(Cpu::BAD_RENDER | Cpu::BAD_INPUT | Cpu::BAD_SOUND);
	m_manager.CleanFlags();
	m_manager.SetFlags(badFlags);
}



void Emulator::Reset()
{
	if(!m_manager.GetFlags(Cpu::BAD_SOUND))
		m_soundPlugin->Stop();

	CleanFlags();
	m_manager.CleanGfx();
	m_manager.CleanStack();
	m_manager.CleanRegisters();
	m_manager.SetPC(0x200);
}





bool Emulator::SetRender(UniqueRender rend) 
{ 
	Log("Setting new iRender...");
	m_renderPlugin = std::move(rend);
	return InitRender();
}




bool Emulator::SetInput(UniqueInput input) 
{
	Log("Setting new iInput...");
	m_inputPlugin = std::move(input);
	return InitInput();
}




bool Emulator::SetSound(UniqueSound sound) 
{
	Log("Setting new iSound...");
	m_soundPlugin = std::move(sound);
	return InitSound();
}




UniqueRender Emulator::SwapRender(UniqueRender rend) 
{ 
	if (rend.get()) 
	{
		Log("Swapping iRender...");
		m_renderPlugin.Swap(rend);
		InitRender();
		return rend;
	}

	Log("Swapping iRender to nullptr...");
	m_manager.SetRender(nullptr);
	return std::move(m_renderPlugin);
}




UniqueInput Emulator::SwapInput(UniqueInput input) 
{ 
	if (input.get())
	{
		Log("Swapping iInput...");
		m_inputPlugin.Swap(input);
		InitInput();
		return input;
	}

	Log("Swapping iInput to nullptr...");
	m_manager.SetInput(nullptr);
	return std::move(m_inputPlugin);
}




UniqueSound Emulator::SwapSound(UniqueSound sound) 
{ 
	if (sound.get())
	{
		Log("Swapping iSound...");
		m_soundPlugin.Swap(sound);
		InitSound();
		return sound;
	}

	Log("Swapping iSound to nullptr...");
	m_manager.SetSound(nullptr);
	return std::move(m_soundPlugin);
}










bool Emulator::InitRender()
{
	iRender* const rend = m_renderPlugin.get();
	const auto set_render = MakeScopeExit([this]() noexcept { 
		m_manager.SetRender(m_renderPlugin.get());
	});


	if (!rend) {
		LogError("Cannot Initialize iRender: nullptr");
		return false;
	} 
	else if (rend->IsInitialized()) {
		return true;
	} 
	else if (!rend->Initialize({512, 256}, m_manager.GetGfxRes())) {
		return false;
	}

	rend->SetBuffer(m_manager.GetGfx());
	rend->SetWinCloseCallback(&m_manager, [](const void* man) { ((CpuManager*)man)->SetFlags(Cpu::EXIT); });
	return true;
}




bool Emulator::InitInput()
{
	iInput* const input = m_inputPlugin.get();
	const auto set_input = MakeScopeExit([this]() noexcept {
		m_manager.SetInput(m_inputPlugin.get());
	});


	if (!input) {
		LogError("Cannot Initialize iInput: nullptr");
		return false;
	}
	else if (input->IsInitialized()) {
		return true;
	}
	else if (!input->Initialize()) {
		return false;
	}


	input->SetEscapeKeyCallback(&m_manager, [](const void* man){ ((CpuManager*)man)->SetFlags(Cpu::EXIT); });
	input->SetResetKeyCallback(this, [](const void* _this) { ((Emulator*)_this)->Reset(); });
	input->SetWaitKeyCallback(this, [](const void* g_emulator)
	{
		auto* const emulator = (Emulator*) g_emulator;
		
		do
		{		
			emulator->GetRender()->UpdateEvents();
			emulator->UpdateTimers();
			
			if (emulator->GetExitFlag())
				return false;

			if (emulator->GetDrawFlag())
				emulator->Draw();


			emulator->HaltForNextFlag();

		}while(!emulator->GetInstrFlag());


		return true;
	});

	return true;
}





bool Emulator::InitSound()
{
	iSound* const sound = m_soundPlugin.get();
	const auto set_sound = MakeScopeExit([this]() noexcept {
		m_manager.SetSound(m_soundPlugin.get());
	});

	if (!sound)  {
		LogError("Cannot Initialize iSound: nullptr");
		return false;
	}
	else if (sound->IsInitialized()) {
		return true;
	}
	else if (!sound->Initialize()) {
		return false;
	}

	sound->SetCountdownFreq(60);
	return true;
}













// local functions definitions
inline void init_emu_timers(Timer& instrTimer, Timer& frameTimer, Timer& chDelayTimer)
{	
	using namespace utix::literals;

	instrTimer.SetTargetTime(380_hz);
	frameTimer.SetTargetTime(60_hz);
	chDelayTimer.SetTargetTime(60_hz);
}



inline bool init_cpu_manager(CpuManager& manager)
{
	// init the CPU
	if (manager.SetMemory(0xFFFF)
		&& manager.SetRegisters(0x10)
		&& manager.SetStack(0x10)
		&& manager.SetGfxRes(64, 32))
	{
		manager.SetPC(0x200);
		manager.LoadDefaultFont();
		manager.LoadHiResFont();
		return true;
	}

	return false;
}










}

