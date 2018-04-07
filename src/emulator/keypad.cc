#include "keypad.hh"
#include "io.hh"

namespace GBEmu::Emulator
{

Keypad::Keypad(IO &io)
	:keys_(),
	buttonKeys_(false),
	directionKeys_(false)
{
	// Joypad
	io.Register("JOYP", 0x00, [&]() {
		uint8_t r = 0xF;

		if (buttonKeys_)
		{
			if (keys_[Keys::Start]) r &= ~8u;
			if (keys_[Keys::Select]) r &= ~4u;
			if (keys_[Keys::B]) r &= ~2u;
			if (keys_[Keys::A]) r &= ~1u;
		}
		if (directionKeys_)
		{
			if (keys_[Keys::Down]) r &= ~8u;
			if (keys_[Keys::Up]) r &= ~4u;
			if (keys_[Keys::Left]) r &= ~2u;
			if (keys_[Keys::Right]) r &= ~1u;
		}

		return r;
	}, [&](uint8_t v) {

		if (!(v & 0x20)) buttonKeys_ = true; else buttonKeys_ = false;
		if (!(v & 0x10)) directionKeys_ = true; else directionKeys_ = false;
	});
}

void Keypad::SetKeys(const KeypadKeys &keys)
{
	for (int i = 0; i < keys_.size(); i++)
		keys_[i] = keys[i];
}

}