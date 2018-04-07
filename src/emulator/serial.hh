#pragma once

#include <string>

namespace GBEmu::Emulator
{

class Log;
class IO;

class Serial
{
public:
	Serial(Log &log, IO &io);

	void Tick();

private:
	Log & log_;
	std::string s_;

};

}