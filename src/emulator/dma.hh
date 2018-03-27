#pragma once

namespace GBEmu::Emulator
{

class IO;
class Memory;

class Dma
{
public:
	Dma(IO &io, Memory &memory);
	virtual ~Dma();

	void Tick();

private:

};

}