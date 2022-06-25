#include "audio.h"
#include <iostream>
std::map<std::string, Audio*> Audio::sLoadedAudios;
Audio::Audio()
{
	hSample = 0;
}

Audio::~Audio()
{
	BASS_SampleFree(hSample);
	if (filename.size())
	{
		auto it = sLoadedAudios.find(filename);
		if (it != sLoadedAudios.end())
			sLoadedAudios.erase(it);
	}
}
HCHANNEL Audio::play(float volume, DWORD flags)
{
	//El handler para un canal
	HCHANNEL hSampleChannel;

	//Creamos un canal para el sample
	hSampleChannel = BASS_SampleGetChannel(hSample, flags);

	//Lanzamos un sample
	BASS_ChannelPlay(hSampleChannel, false);
	return hSampleChannel;
}
Audio* Audio::Find(const char* filename)
{
	assert(filename);
	auto it = sLoadedAudios.find(filename);
	if (it != sLoadedAudios.end())
		return it->second;
	return NULL;
}
HCHANNEL Audio::Play(const char* filename, DWORD flags)
{
	Audio* audio = Audio::Get(filename);
	HCHANNEL hSampleChannel = audio->play(1.0f, flags);
	return hSampleChannel;

}
HSAMPLE Audio::LoadSample(const char* filename)
{
	//Cargamos un sample del disco duro (memoria, filename, offset, length, max, flags)
	//use BASS_SAMPLE_LOOP in the last param to have a looped sound
	hSample = BASS_SampleLoad(false, filename, 0, 0, 3, 0);
	setName(filename);
	if (hSample == 0)
	{
		std::cout << "ERROR load " << filename << std::endl;
		return 0;
	}

	std::cout << " + AUDIO load " << filename << std::endl;
	return hSample;

}

void Audio::setName(const char* name)
{

	filename = name;
	sLoadedAudios[filename] = this;

}

BOOL Audio::Stop(HCHANNEL channel)
{
	//BASS_ChannelPause(channel);
	return BASS_ChannelStop(channel);
}

Audio* Audio::Get(const char* filename)
{
	//load it
	Audio* audio = Find(filename);
	if (audio)
		return audio;

	audio = new Audio();
	if (!audio->LoadSample(filename))
	{
		delete audio;
		return NULL;
	}

	return audio;
}
