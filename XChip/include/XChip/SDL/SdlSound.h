#include <cstdint>
#include <XChip/Interfaces/iSound.h>
#include "SdlMedia.h"

struct SDL_AudioSpec;
typedef uint32_t SDL_AudioDeviceID;

namespace xchip {



class SdlSound final : private SdlMedia, public iSound
{
public:
	SdlSound() noexcept;
	~SdlSound();
	bool Initialize() noexcept override;
	void Dispose() noexcept override;

	bool IsInitialized() override { return _initialized; }
	bool IsPlaying() override { return _audioLen > 0;  };
	void Play() override;
	void Stop() override;

private:
	static void audio_callback(void* sdlSound, uint8_t* const stream, int len);

private:
	SDL_AudioSpec* _want; 
	SDL_AudioSpec* _have;
	SDL_AudioDeviceID _dev;
	unsigned int _audioPos; /* which sample we are up to */
	int _audioLen;          /* how many samples left to play, stops when <= 0 */
	float _audioFreq;       /* audio frequency in cycles per sample */
	float _audioVol;        /* audio volume, 0 - ~32000 */
	bool _initialized;

};
















}
