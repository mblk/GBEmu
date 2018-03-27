#include "dma.hh"
#include "io.hh"
#include "memory.hh"

namespace GBEmu::Emulator
{

Dma::Dma(IO &io, Memory &memory)
{
	// DMA
	io.Register("DMA", 0x46, []() {
		return 0;
	}, [&](uint8_t v) {

		uint16_t sourceAddr = v << 8;
		uint16_t destAddr = 0xFE00;
		uint16_t bytes = 160;

		/* Source:      XX00-XX9F   ;XX in range from 00-F1h
			Destination: FE00-FE9F*/

		for (uint16_t i = 0; i < bytes; i++)
		{
			uint8_t data = memory.Read(sourceAddr + i);
			memory.Write(destAddr + i, data);
		}

	});
}

Dma::~Dma()
{

}

void Dma::Tick()
{

}

}