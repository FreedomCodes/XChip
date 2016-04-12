#ifndef _XCHIP_ISOUND_H_
#define _XCHIP_ISOUND_H_
#include "iMediaInterface.h"
#include "../Utility/StdintDef.h"

 
namespace xchip {



class iSound : public iMediaInterface
{
public:
	virtual bool Initialize() noexcept = 0;
	virtual bool IsPlaying() const = 0;
	virtual void SetCountdownFreq(const float hz) = 0;
	virtual void Play(const uint8_t soundTimer) = 0;
	virtual void Stop() = 0;


};









}










#endif
