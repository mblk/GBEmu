#pragma once

#include <array>

namespace GBEmu::Emulator
{

class IO;

using KeypadKeys = std::array<bool, 8>;

class Keypad
{
public:
	Keypad(IO &io);

	void SetKeys(const KeypadKeys &keys);

	enum Keys
	{
		Start = 0,
		Select = 1,
		B = 2,
		A = 3,
		Down = 4,
		Up = 5,
		Left = 6,
		Right = 7
	};

private:
	KeypadKeys keys_;
	bool buttonKeys_, directionKeys_;
};

}