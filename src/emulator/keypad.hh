#pragma once

namespace GBEmu::Emulator
{

class IO;

class Keypad
{
public:
	Keypad(IO &io);
	virtual ~Keypad();

	void SetKeys(bool keys[8]);

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
	bool keys_[8];
	bool buttonKeys_, directionKeys_;
};

}