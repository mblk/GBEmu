#pragma once

#include <cstdint>

namespace GBEmu::Emulator
{

class Log;
class IO;
class Pic;

class Timer
{
public:
	Timer(Log &log, IO &io, Pic &pic);
	virtual ~Timer();

	void Tick(uint32_t ticks);

private:
	Log & log_;
	IO & io_;
	Pic & pic_;

	uint8_t divValue_;
	uint8_t timaValue_;
	uint8_t tmaValue_;
	uint8_t tacValue_;

	uint32_t divTicks_;
	uint32_t timaTicks_;

};

}