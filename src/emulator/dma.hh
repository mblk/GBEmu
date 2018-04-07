#pragma once

namespace GBEmu::Emulator
{

class IO;
class Memory;

class Dma
{
public:
	Dma(IO &io, Memory &memory);

	void Tick();

private:

};

}