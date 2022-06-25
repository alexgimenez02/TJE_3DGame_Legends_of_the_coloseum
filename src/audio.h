#ifndef AUDIO_H
#define AUDIO_H
#include <bass.h>
#include <map>
#include <string>
#include <cassert>
class Audio
{
public:
	static std::map<std::string, Audio*> sLoadedAudios; //para nuestro manager
	HSAMPLE hSample; //aqui guardamos el handler del sample que retorna BASS_SampleLoad
	std::string filename;
	Audio(); //importante poner sample a cero aqui
	~Audio(); //aqui deberiamos liberar el sample con BASS_SampleFree

	HCHANNEL play(float volume, DWORD flags); //lanza el audio y retorna el channel donde suena

	static BOOL Stop(HCHANNEL channel); //para parar un audio necesitamos su channel
	static Audio* Get(const char* filename); //manager de audios
	static Audio* Find(const char* filename);
	static HCHANNEL Play(const char* filename, DWORD flags); //version estática para ir mas rapido
	HSAMPLE LoadSample(const char* filename);
	void setName(const char* name);


};

#endif

