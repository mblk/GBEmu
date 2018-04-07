#include "cpu.hh"
#include "memory.hh"
#include "io.hh"
#include "pic.hh"
#include "log.hh"

#include <cstdio>
#include <cassert>
#include <array>

namespace GBEmu::Emulator
{

Cpu::Cpu(Log &log, Memory &memory, IO &io, Pic &pic)
	:log_(log),
	memory_(memory),
	io_(io),
	pic_(pic),
	instructions_({}),
	instructionsCB_({}),
	instructions10_({})
{
	Reset();

	Registers &r = regs_;
	Memory &m = memory_;

	auto set = [&](uint8_t opcode, std::string name, uint8_t operands,
		uint8_t cycles, InstructionHandler handler)
	{
		assert(!instructions_[opcode].handler);
		instructions_[opcode] = { name, operands, cycles, handler, nullptr };
	};

	auto setCB = [&](uint8_t opcode, std::string name, uint8_t operands,
		uint8_t cycles, InstructionHandler handler)
	{
		assert(!instructionsCB_[opcode].handler);
		instructionsCB_[opcode] = { name, operands, cycles, handler, nullptr };
	};

	auto setCB2 = [&](uint8_t opcode, std::string name, uint8_t operands,
		uint8_t cycles, InstructionHandler2 handler)
	{
		assert(!instructionsCB_[opcode].handler);
		instructionsCB_[opcode] = { name, operands, cycles, nullptr, handler };
	};

	auto set10 = [&](uint8_t opcode, std::string name, uint8_t operands,
		uint8_t cycles, InstructionHandler handler)
	{
		assert(!instructions10_[opcode].handler);
		instructions10_[opcode] = { name, operands, cycles, handler, nullptr };
	};

	// xxx
	auto push8 = [&](uint8_t v)
	{
		r.sp--;
		m.Write(r.sp, v);
	};
	auto push16 = [&](uint16_t v)
	{
		uint8_t l = v & 0xFF;
		uint8_t h = v >> 8;
		r.sp -= 2;
		m.Write(r.sp + 0, l);
		m.Write(r.sp + 1, h);
	};
	auto pop8 = [&]() -> uint8_t
	{
		uint8_t v = m.Read(r.sp);
		r.sp++;
		return v;
	};
	auto pop16 = [&]() -> uint16_t
	{
		uint8_t l = m.Read(r.sp + 0);
		uint8_t h = m.Read(r.sp + 1);
		r.sp += 2;
		return l | (h << 8);
	};

	// 3.3.1 8-Bit Loads
	// 1. LD nn,n
	set(0x06, "LD B,n", 1, 8, [&](auto p) { r.b = p[0]; });
	set(0x0E, "LD C,n", 1, 8, [&](auto p) { r.c = p[0]; });
	set(0x16, "LD D,n", 1, 8, [&](auto p) { r.d = p[0]; });
	set(0x1E, "LD E,n", 1, 8, [&](auto p) { r.e = p[0]; });
	set(0x26, "LD H,n", 1, 8, [&](auto p) { r.h = p[0]; });
	set(0x2E, "LD L,n", 1, 8, [&](auto p) { r.l = p[0]; });
	// 2. LD r1,r2
	set(0x7F, "LD A,A", 0, 4, [&](auto p) { r.a = r.a; });
	set(0x78, "LD A,B", 0, 4, [&](auto p) { r.a = r.b; });
	set(0x79, "LD A,C", 0, 4, [&](auto p) { r.a = r.c; });
	set(0x7A, "LD A,D", 0, 4, [&](auto p) { r.a = r.d; });
	set(0x7B, "LD A,E", 0, 4, [&](auto p) { r.a = r.e; });
	set(0x7C, "LD A,H", 0, 4, [&](auto p) { r.a = r.h; });
	set(0x7D, "LD A,L", 0, 4, [&](auto p) { r.a = r.l; });
	set(0x7E, "LD A,(HL)", 0, 8, [&](auto p) { r.a = m.Read(r.hl); });
	set(0x40, "LD B,B", 0, 4, [&](auto p) { r.b = r.b; });
	set(0x41, "LD B,C", 0, 4, [&](auto p) { r.b = r.c; });
	set(0x42, "LD B,D", 0, 4, [&](auto p) { r.b = r.d; });
	set(0x43, "LD B,E", 0, 4, [&](auto p) { r.b = r.e; });
	set(0x44, "LD B,H", 0, 4, [&](auto p) { r.b = r.h; });
	set(0x45, "LD B,L", 0, 4, [&](auto p) { r.b = r.l; });
	set(0x46, "LD B,(HL)", 0, 8, [&](auto p) { r.b = m.Read(r.hl); });
	set(0x48, "LD C,B", 0, 4, [&](auto p) { r.c = r.b; });
	set(0x49, "LD C,C", 0, 4, [&](auto p) { r.c = r.c; });
	set(0x4A, "LD C,D", 0, 4, [&](auto p) { r.c = r.d; });
	set(0x4B, "LD C,E", 0, 4, [&](auto p) { r.c = r.e; });
	set(0x4C, "LD C,H", 0, 4, [&](auto p) { r.c = r.h; });
	set(0x4D, "LD C,L", 0, 4, [&](auto p) { r.c = r.l; });
	set(0x4E, "LD C,(HL)", 0, 8, [&](auto p) { r.c = m.Read(r.hl); });
	set(0x50, "LD D,B", 0, 4, [&](auto p) { r.d = r.b; });
	set(0x51, "LD D,C", 0, 4, [&](auto p) { r.d = r.c; });
	set(0x52, "LD D,D", 0, 4, [&](auto p) { r.d = r.d; });
	set(0x53, "LD D,E", 0, 4, [&](auto p) { r.d = r.e; });
	set(0x54, "LD D,H", 0, 4, [&](auto p) { r.d = r.h; });
	set(0x55, "LD D,L", 0, 4, [&](auto p) { r.d = r.l; });
	set(0x56, "LD D,(HL)", 0, 8, [&](auto p) { r.d = m.Read(r.hl); });
	set(0x58, "LD E,B", 0, 4, [&](auto p) { r.e = r.b; });
	set(0x59, "LD E,C", 0, 4, [&](auto p) { r.e = r.c; });
	set(0x5A, "LD E,D", 0, 4, [&](auto p) { r.e = r.d; });
	set(0x5B, "LD E,E", 0, 4, [&](auto p) { r.e = r.e; });
	set(0x5C, "LD E,H", 0, 4, [&](auto p) { r.e = r.h; });
	set(0x5D, "LD E,L", 0, 4, [&](auto p) { r.e = r.l; });
	set(0x5E, "LD E,(HL)", 0, 8, [&](auto p) { r.e = m.Read(r.hl); });
	set(0x60, "LD H,B", 0, 4, [&](auto p) { r.h = r.b; });
	set(0x61, "LD H,C", 0, 4, [&](auto p) { r.h = r.c; });
	set(0x62, "LD H,D", 0, 4, [&](auto p) { r.h = r.d; });
	set(0x63, "LD H,E", 0, 4, [&](auto p) { r.h = r.e; });
	set(0x64, "LD H,H", 0, 4, [&](auto p) { r.h = r.h; });
	set(0x65, "LD H,L", 0, 4, [&](auto p) { r.h = r.l; });
	set(0x66, "LD H,(HL)", 0, 8, [&](auto p) { r.h = m.Read(r.hl); });
	set(0x68, "LD L,B", 0, 4, [&](auto p) { r.l = r.b; });
	set(0x69, "LD L,C", 0, 4, [&](auto p) { r.l = r.c; });
	set(0x6A, "LD L,D", 0, 4, [&](auto p) { r.l = r.d; });
	set(0x6B, "LD L,E", 0, 4, [&](auto p) { r.l = r.e; });
	set(0x6C, "LD L,H", 0, 4, [&](auto p) { r.l = r.h; });
	set(0x6D, "LD L,L", 0, 4, [&](auto p) { r.l = r.l; });
	set(0x6E, "LD L,(HL)", 0, 8, [&](auto p) { r.l = m.Read(r.hl); });
	set(0x70, "LD (HL),B", 0, 8, [&](auto p) { m.Write(r.hl, r.b); });
	set(0x71, "LD (HL),C", 0, 8, [&](auto p) { m.Write(r.hl, r.c); });
	set(0x72, "LD (HL),D", 0, 8, [&](auto p) { m.Write(r.hl, r.d); });
	set(0x73, "LD (HL),E", 0, 8, [&](auto p) { m.Write(r.hl, r.e); });
	set(0x74, "LD (HL),H", 0, 8, [&](auto p) { m.Write(r.hl, r.h); });
	set(0x75, "LD (HL),L", 0, 8, [&](auto p) { m.Write(r.hl, r.l); });
	set(0x36, "LD (HL),n", 1, 12, [&](auto p) { m.Write(r.hl, p[0]); });
	// 3. LD A,n
	//set(0x7F, "LD A,A", 0, 4, [&](auto p) { r.a = r.a; });
	//set(0x78, "LD A,B", 0, 4, [&](auto p) { r.a = r.b; });
	//set(0x79, "LD A,C", 0, 4, [&](auto p) { r.a = r.c; });
	//set(0x7A, "LD A,D", 0, 4, [&](auto p) { r.a = r.d; });
	//set(0x7B, "LD A,E", 0, 4, [&](auto p) { r.a = r.e; });
	//set(0x7C, "LD A,H", 0, 4, [&](auto p) { r.a = r.h; });
	//set(0x7D, "LD A,L", 0, 4, [&](auto p) { r.a = r.l; });
	set(0x0A, "LD A,(BC)", 0, 8, [&](auto p) { r.a = m.Read(r.bc); });
	set(0x1A, "LD A,(DE)", 0, 8, [&](auto p) { r.a = m.Read(r.de); });
	//set(0x7E, "LD A,(HL)", 0, 8, [&](auto p) { r.a = m.Read(r.hl); });
	set(0xFA, "LD A,(nn)", 2, 16, [&](auto p) { r.a = m.Read(p[0] | (p[1] << 8)); });
	set(0x3E, "LD A,#", 1, 8, [&](auto p) { r.a = p[0]; });
	// 4. LD n,A
	//set(0x7F, "LD A,A", 0, 4, [&](auto p) { r.a = r.a; });
	set(0x47, "LD B,A", 0, 4, [&](auto p) { r.b = r.a; });
	set(0x4F, "LD C,A", 0, 4, [&](auto p) { r.c = r.a; });
	set(0x57, "LD D,A", 0, 4, [&](auto p) { r.d = r.a; });
	set(0x5F, "LD E,A", 0, 4, [&](auto p) { r.e = r.a; });
	set(0x67, "LD H,A", 0, 4, [&](auto p) { r.h = r.a; });
	set(0x6F, "LD L,A", 0, 4, [&](auto p) { r.l = r.a; });
	set(0x02, "LD (BC),A", 0, 8, [&](auto p) { m.Write(r.bc, r.a); });
	set(0x12, "LD (DE),A", 0, 8, [&](auto p) { m.Write(r.de, r.a); });
	set(0x77, "LD (HL),A", 0, 8, [&](auto p) { m.Write(r.hl, r.a); });
	set(0xEA, "LD (nn),A", 2, 16, [&](auto p) { m.Write(p[0] | (p[1] << 8), r.a); });
	// 5. LD A,(C)
	set(0xF2, "LD A,(C)", 0, 8, [&](auto p) { r.a = m.Read(0xFF00 + r.c); });
	// 6. LD (C),A
	set(0xE2, "LD (C),A", 0, 8, [&](auto p) { m.Write(0xFF00 + r.c, r.a); });
	// 7. LD A,(HLD) -> same as LDD A,(HL)
	// 8. LD A,(HL-) -> same as LDD A,(HL)
	// 9. LDD A,(HL)
	set(0x3A, "LDD A,(HL)", 0, 8, [&](auto p) {
		r.a = m.Read(r.hl);
		r.hl--;
	});
	// 10. LD (HLD),A -> same as LDD (HL),A
	// 11. LD (HL-),A -> same as LDD (HL),A
	// 12. LDD (HL),A
	set(0x32, "LDD (HL),A", 0, 8, [&](auto p) {
		m.Write(r.hl, r.a);
		r.hl--;
	});
	// 13. LD A,(HLI) -> same as LDI A,(HL)
	// 14. LD A,(HL+) -> same as LDI A,(HL)
	// 15. LDI A,(HL)
	set(0x2A, "LDI A,(HL)", 0, 8, [&](auto p) {
		r.a = m.Read(r.hl);
		r.hl++;
	});
	// 16. LD (HLI),A -> same as LDI (HL),A
	// 17. LD (HL+),A -> same as LDI (HL),A
	// 18. LDI (HL),A
	set(0x22, "LDI (HL),A", 0, 8, [&](auto p) {
		m.Write(r.hl, r.a);
		r.hl++;
	});
	// 19. LDH (n),A
	set(0xE0, "LDH (n),A", 1, 12, [&](auto p) { m.Write(0xFF00 + p[0], r.a); });
	// 20. LDH A,(n)
	set(0xF0, "LDH A,(n)", 1, 12, [&](auto p) { r.a = m.Read(0xFF00 + p[0]); });

	// 3.3.2 16-Bit Loads
	// 1. LD n,nn
	set(0x01, "LD BC,nn", 2, 12, [&](auto p) { r.bc = p[0] | (p[1] << 8); });
	set(0x11, "LD DE,nn", 2, 12, [&](auto p) { r.de = p[0] | (p[1] << 8); });
	set(0x21, "LD HL,nn", 2, 12, [&](auto p) { r.hl = p[0] | (p[1] << 8); });
	set(0x31, "LD SP,nn", 2, 12, [&](auto p) { r.sp = p[0] | (p[1] << 8); });
	// 2. LD SP,HL
	set(0xF9, "LD SP,HL", 0, 8, [&](auto p) { r.sp = r.hl; });
	// 3. LD HL,SP+n -> same as LDHL SP,n
	// 4. LDHL SP,n
	set(0xF8, "LDHL SP,n", 1, 12, [&](auto p) { // TODO verify
		int8_t n = reinterpret_cast<int8_t&>(p[0]);
		r.hl = r.sp + n;
		r.flagZ = 0;
		r.flagN = 0;
		if (n > 0)
		{
			r.flagH = (r.hl & 0xF) < (r.sp & 0xF);
			r.flagC = r.hl < r.sp;
		}
		else
		{
			r.flagH = (r.hl & 0xF) > (r.sp & 0xF);
			r.flagC = r.hl > r.sp;
		}
	});
	// 5. LD (nn),SP
	set(0x08, "LD (nn),SP", 2, 20, [&](auto p) { // TODO verify
		uint16_t addr = p[0] | (p[1] << 8);
		m.Write(addr + 0, r.spl);
		m.Write(addr + 1, r.sph);
	});
	// 6. PUSH nn
	set(0xF5, "PUSH AF", 0, 16, [&](auto p) {
		r.sp -= 2;
		m.Write(r.sp + 0, r.f);
		m.Write(r.sp + 1, r.a);
	});
	set(0xC5, "PUSH BC", 0, 16, [&](auto p) {
		r.sp -= 2;
		m.Write(r.sp + 0, r.c);
		m.Write(r.sp + 1, r.b);
	});
	set(0xD5, "PUSH DE", 0, 16, [&](auto p) {
		r.sp -= 2;
		m.Write(r.sp + 0, r.e);
		m.Write(r.sp + 1, r.d);
	});
	set(0xE5, "PUSH HL", 0, 16, [&](auto p) {
		r.sp -= 2;
		m.Write(r.sp + 0, r.l);
		m.Write(r.sp + 1, r.h);
	});
	// 7. POP nn
	set(0xF1, "POP AF", 0, 12, [&](auto p) {
		r.f = m.Read(r.sp + 0);
		r.a = m.Read(r.sp + 1);
		r.sp += 2;
	});
	set(0xC1, "POP BC", 0, 12, [&](auto p) {
		r.c = m.Read(r.sp + 0);
		r.b = m.Read(r.sp + 1);
		r.sp += 2;
	});
	set(0xD1, "POP DE", 0, 12, [&](auto p) {
		r.e = m.Read(r.sp + 0);
		r.d = m.Read(r.sp + 1);
		r.sp += 2;
	});
	set(0xE1, "POP HL", 0, 12, [&](auto p) {
		r.l = m.Read(r.sp + 0);
		r.h = m.Read(r.sp + 1);
		r.sp += 2;
	});

	// 3.3.3 8-Bit ALU
	// 1. ADD A,n
	auto addAn = [&](uint8_t n) // TODO verify & optimize
	{
		uint8_t a0 = r.a;
		uint8_t a1 = r.a + n;

		r.a = a1;
		r.flagH = (a1 & 0xF) < (a0 & 0xF);
		r.flagC = a1 < a0;
		r.flagZ = !a1;
		r.flagN = 0;
	};
	set(0x87, "ADD A,A", 0, 4, [&,addAn](auto p) { addAn(r.a); });
	set(0x80, "ADD A,B", 0, 4, [&,addAn](auto p) { addAn(r.b); });
	set(0x81, "ADD A,C", 0, 4, [&,addAn](auto p) { addAn(r.c); });
	set(0x82, "ADD A,D", 0, 4, [&,addAn](auto p) { addAn(r.d); });
	set(0x83, "ADD A,E", 0, 4, [&,addAn](auto p) { addAn(r.e); });
	set(0x84, "ADD A,H", 0, 4, [&,addAn](auto p) { addAn(r.h); });
	set(0x85, "ADD A,L", 0, 4, [&,addAn](auto p) { addAn(r.l); });
	set(0x86, "ADD A,(HL)", 0, 8, [&,addAn](auto p) { addAn(m.Read(r.hl)); });
	set(0xC6, "ADD A,#", 1, 8, [&,addAn](auto p) { addAn(p[0]); });
	// 2. ADC A,n
	auto adcAn = [&](uint8_t n) // TODO verify & optimize
	{
		uint8_t a0 = r.a;
		uint8_t a1 = r.a + r.flagC + n;

		r.a = a1;
		r.flagH = (a1 & 0xF) < (a0 & 0xF);
		r.flagC = a1 < a0;
		r.flagZ = !a1;
		r.flagN = 0;
	};
	set(0x8F, "ADC A,A", 0, 4, [&,adcAn](auto p) { adcAn(r.a); });
	set(0x88, "ADC A,B", 0, 4, [&,adcAn](auto p) { adcAn(r.b); });
	set(0x89, "ADC A,C", 0, 4, [&,adcAn](auto p) { adcAn(r.c); });
	set(0x8A, "ADC A,D", 0, 4, [&,adcAn](auto p) { adcAn(r.d); });
	set(0x8B, "ADC A,E", 0, 4, [&,adcAn](auto p) { adcAn(r.e); });
	set(0x8C, "ADC A,H", 0, 4, [&,adcAn](auto p) { adcAn(r.h); });
	set(0x8D, "ADC A,L", 0, 4, [&,adcAn](auto p) { adcAn(r.l); });
	set(0x8E, "ADC A,(HL)", 0, 8, [&,adcAn](auto p) { adcAn(m.Read(r.hl)); });
	set(0xCE, "ADC A,#", 1, 8, [&,adcAn](auto p) { adcAn(p[0]); });
	// 3. SUB n
	auto subAn = [&](uint8_t n) // TODO verify & optimize
	{
		uint8_t a0 = r.a;
		uint8_t a1 = r.a - n;

		//r.flagH = ((r.a - n) & 0xF) > (r.a & 0xF); // TODO invert?
		//r.flagC = (r.a - n) > r.a; // TODO invert?

		r.a = a1;

		r.flagH = (a0 & 0xF) < (n & 0xF);
		r.flagC = a0 < n;

		r.flagZ = !a1;
		r.flagN = 1;
	};
	set(0x97, "SUB A,A", 0, 4, [&,subAn](auto p) { subAn(r.a); });
	set(0x90, "SUB A,B", 0, 4, [&,subAn](auto p) { subAn(r.b); });
	set(0x91, "SUB A,C", 0, 4, [&,subAn](auto p) { subAn(r.c); });
	set(0x92, "SUB A,D", 0, 4, [&,subAn](auto p) { subAn(r.d); });
	set(0x93, "SUB A,E", 0, 4, [&,subAn](auto p) { subAn(r.e); });
	set(0x94, "SUB A,H", 0, 4, [&,subAn](auto p) { subAn(r.h); });
	set(0x95, "SUB A,L", 0, 4, [&,subAn](auto p) { subAn(r.l); });
	set(0x96, "SUB A,(HL)", 0, 8, [&,subAn](auto p) { subAn(m.Read(r.hl)); });
	set(0xD6, "SUB A,#", 1, 8, [&,subAn](auto p) { subAn(p[0]); });
	// 4. SBC A,n
	auto sbcAn = [&](uint8_t n) // TODO verify & optimize
	{
		uint8_t a0 = r.a;
		uint8_t a1 = r.a - r.flagC - n;

		//r.flagH = ((r.a - r.flagC - n) & 0xF) > (r.a & 0xF); // TODO invert?
		//r.flagC = (r.a - r.flagC - n) > r.a; // TODO invert?

		r.a = a1;

		r.flagH = (a0 & 0xF) < ((n + r.flagC) & 0xF);
		r.flagC = a0 < (n + r.flagC);

		//r.a -= r.flagC + n;
		r.flagZ = !a1;
		r.flagN = 1;
	};
	set(0x9F, "SBC A,A", 0, 4, [&, sbcAn](auto p) { sbcAn(r.a); });
	set(0x98, "SBC A,B", 0, 4, [&, sbcAn](auto p) { sbcAn(r.b); });
	set(0x99, "SBC A,C", 0, 4, [&, sbcAn](auto p) { sbcAn(r.c); });
	set(0x9A, "SBC A,D", 0, 4, [&, sbcAn](auto p) { sbcAn(r.d); });
	set(0x9B, "SBC A,E", 0, 4, [&, sbcAn](auto p) { sbcAn(r.e); });
	set(0x9C, "SBC A,H", 0, 4, [&, sbcAn](auto p) { sbcAn(r.h); });
	set(0x9D, "SBC A,L", 0, 4, [&, sbcAn](auto p) { sbcAn(r.l); });
	set(0x9E, "SBC A,(HL)", 0, 8, [&, sbcAn](auto p) { sbcAn(m.Read(r.hl)); });
	set(0xDE, "SBC A,#", 1, 8, [&, sbcAn](auto p) { sbcAn(p[0]); });
	// 5. AND n
	auto andAn = [&](uint8_t n)
	{
		r.a &= n;
		r.flagZ = !r.a;
		r.flagN = 0;
		r.flagH = 1;
		r.flagC = 0;
	};
	set(0xA7, "AND A,A", 0, 4, [&, andAn](auto p) {
		andAn(r.a);
	});
	set(0xA0, "AND A,B", 0, 4, [&, andAn](auto p) { andAn(r.b); });
	set(0xA1, "AND A,C", 0, 4, [&, andAn](auto p) { andAn(r.c); });
	set(0xA2, "AND A,D", 0, 4, [&, andAn](auto p) { andAn(r.d); });
	set(0xA3, "AND A,E", 0, 4, [&, andAn](auto p) { andAn(r.e); });
	set(0xA4, "AND A,H", 0, 4, [&, andAn](auto p) { andAn(r.h); });
	set(0xA5, "AND A,L", 0, 4, [&, andAn](auto p) { andAn(r.l); });
	set(0xA6, "AND A,(HL)", 0, 8, [&, andAn](auto p) { andAn(m.Read(r.hl)); });
	set(0xE6, "AND A,#", 1, 8, [&, andAn](auto p) { andAn(p[0]); });
	// 6. OR n
	auto orAn = [&](uint8_t n)
	{
		r.a |= n;
		r.flagZ = !r.a;
		r.flagN = 0;
		r.flagH = 0;
		r.flagC = 0;
	};
	set(0xB7, "OR A,A", 0, 4, [&, orAn](auto p) { orAn(r.a); });
	set(0xB0, "OR A,B", 0, 4, [&, orAn](auto p) { orAn(r.b); });
	set(0xB1, "OR A,C", 0, 4, [&, orAn](auto p) { orAn(r.c); });
	set(0xB2, "OR A,D", 0, 4, [&, orAn](auto p) { orAn(r.d); });
	set(0xB3, "OR A,E", 0, 4, [&, orAn](auto p) { orAn(r.e); });
	set(0xB4, "OR A,H", 0, 4, [&, orAn](auto p) { orAn(r.h); });
	set(0xB5, "OR A,L", 0, 4, [&, orAn](auto p) { orAn(r.l); });
	set(0xB6, "OR A,(HL)", 0, 8, [&, orAn](auto p) { orAn(m.Read(r.hl)); });
	set(0xF6, "OR A,#", 1, 8, [&, orAn](auto p) { orAn(p[0]); });
	// 7. XOR n
	auto xorAn = [&](uint8_t n)
	{
		r.a ^= n;
		r.flagZ = !r.a;
		r.flagN = 0;
		r.flagH = 0;
		r.flagC = 0;
	};
	set(0xAF, "XOR A,A", 0, 4, [&,xorAn](auto p) { xorAn(r.a); });
	set(0xA8, "XOR A,B", 0, 4, [&,xorAn](auto p) { xorAn(r.b); });
	set(0xA9, "XOR A,C", 0, 4, [&,xorAn](auto p) { xorAn(r.c); });
	set(0xAA, "XOR A,D", 0, 4, [&,xorAn](auto p) { xorAn(r.d); });
	set(0xAB, "XOR A,E", 0, 4, [&,xorAn](auto p) { xorAn(r.e); });
	set(0xAC, "XOR A,H", 0, 4, [&,xorAn](auto p) { xorAn(r.h); });
	set(0xAD, "XOR A,L", 0, 4, [&,xorAn](auto p) { xorAn(r.l); });
	set(0xAE, "XOR A,(HL)", 0, 8, [&,xorAn](auto p) { xorAn(m.Read(r.hl)); });
	set(0xEE, "XOR A,#", 1, 8, [&,xorAn](auto p) { xorAn(p[0]); });
	// 8. CP n
	auto cpAn = [&](uint8_t n)
	{
		r.flagZ = r.a == n;
		r.flagN = 1;
		r.flagH = (r.a & 0xF) < (n & 0xF); // TODO verify
		r.flagC = r.a < n; // TODO verify
	};
	set(0xBF, "CP A,A", 0, 4, [&, cpAn](auto p) { cpAn(r.a); });
	set(0xB8, "CP A,B", 0, 4, [&, cpAn](auto p) { cpAn(r.b); });
	set(0xB9, "CP A,C", 0, 4, [&, cpAn](auto p) { cpAn(r.c); });
	set(0xBA, "CP A,D", 0, 4, [&, cpAn](auto p) { cpAn(r.d); });
	set(0xBB, "CP A,E", 0, 4, [&, cpAn](auto p) { cpAn(r.e); });
	set(0xBC, "CP A,H", 0, 4, [&, cpAn](auto p) { cpAn(r.h); });
	set(0xBD, "CP A,L", 0, 4, [&, cpAn](auto p) { cpAn(r.l); });
	set(0xBE, "CP A,(HL)", 0, 8, [&, cpAn](auto p) { cpAn(m.Read(r.hl)); });
	set(0xFE, "CP A,#", 1, 8, [&, cpAn](auto p) { cpAn(p[0]); });
	// 9. INC n
	auto incnFlags = [&](uint8_t value)
	{
		r.flagZ = !value;
		r.flagN = 0;
		r.flagH = !(value & 0xF);
	};
	set(0x3C, "INC A", 0, 4, [&, incnFlags](auto p) {
		r.a++;
		incnFlags(r.a);
	});
	set(0x04, "INC B", 0, 4, [&, incnFlags](auto p) {
		r.b++;
		incnFlags(r.b);
	});
	set(0x0C, "INC C", 0, 4, [&, incnFlags](auto p) {
		r.c++;
		incnFlags(r.c);
	});
	set(0x14, "INC D", 0, 4, [&, incnFlags](auto p) {
		r.d++;
		incnFlags(r.d);
	});
	set(0x1C, "INC E", 0, 4, [&, incnFlags](auto p) {
		r.e++;
		incnFlags(r.e);
	});
	set(0x24, "INC H", 0, 4, [&, incnFlags](auto p) {
		r.h++;
		incnFlags(r.h);
	});
	set(0x2C, "INC L", 0, 4, [&, incnFlags](auto p) {
		r.l++;
		incnFlags(r.l);
	});
	set(0x34, "INC (HL)", 0, 12, [&, incnFlags](auto p) {
		uint8_t v = m.Read(r.hl);
		v++;
		m.Write(r.hl, v);
		incnFlags(v);
	});
	// 10. DEC n
	auto decnFlags = [&](uint8_t value)
	{
		r.flagZ = !value;
		r.flagN = 1;
		r.flagH = (value & 0xF) == 0xF;
	};
	set(0x3D, "DEC A", 0, 4, [&, decnFlags](auto p) {
		r.a--;
		decnFlags(r.a);
	});
	set(0x05, "DEC B", 0, 4, [&, decnFlags](auto p) {
		r.b--;
		decnFlags(r.b);
	});
	set(0x0D, "DEC C", 0, 4, [&, decnFlags](auto p) {
		r.c--;
		decnFlags(r.c);
	});
	set(0x15, "DEC D", 0, 4, [&, decnFlags](auto p) {
		r.d--;
		decnFlags(r.d);
	});
	set(0x1D, "DEC E", 0, 4, [&, decnFlags](auto p) {
		r.e--;
		decnFlags(r.e);
	});
	set(0x25, "DEC H", 0, 4, [&, decnFlags](auto p) {
		r.h--;
		decnFlags(r.h);
	});
	set(0x2D, "DEC L", 0, 4, [&, decnFlags](auto p) {
		r.l--;
		decnFlags(r.l);
	});
	set(0x35, "DEC (HL)", 0, 12, [&, decnFlags](auto p) {
		uint8_t v = m.Read(r.hl);
		v--;
		m.Write(r.hl, v);
		decnFlags(v);
	});

	// 3.3.4 16-Bit Arithmetic
	// 1. ADD HL,n
	auto addHLn = [&](uint16_t hl0, uint16_t hl1)
	{
		r.hl = hl1;
		r.flagN = 0;
		r.flagH = (hl1 & 0xFFF) < (hl0 & 0xFFF);
		r.flagC = hl1 < hl0;
	};
	set(0x09, "ADD HL,BC", 0, 8, [&, addHLn](auto p) { addHLn(r.hl, r.hl + r.bc); });
	set(0x19, "ADD HL,DE", 0, 8, [&, addHLn](auto p) { addHLn(r.hl, r.hl + r.de); });
	set(0x29, "ADD HL,HL", 0, 8, [&, addHLn](auto p) { addHLn(r.hl, r.hl + r.hl); });
	set(0x39, "ADD HL,SP", 0, 8, [&, addHLn](auto p) { addHLn(r.hl, r.hl + r.sp); });
	// 2. ADD SP,n
	set(0xE8, "ADD SP,n", 1, 16, [&](auto p)
	{
		int8_t n = reinterpret_cast<int8_t&>(p[0]);
		uint16_t sp0 = r.sp;
		uint16_t sp1 = sp0 + n;
		
		r.sp = sp1;
		r.flagZ = 0;
		r.flagN = 0;

		if (n > 0) // TODO verify
		{
			r.flagH = (sp1 & 0xF) < (sp0 & 0xF);
			r.flagC = sp1 < sp0;
		}
		else
		{
			r.flagH = (sp1 & 0xF) > (sp0 & 0xF);
			r.flagC = sp1 > sp0;
		}
	});
	// 3. INC nn
	set(0x03, "INC BC", 0, 8, [&](auto p) { r.bc++; });
	set(0x13, "INC DE", 0, 8, [&](auto p) { r.de++; });
	set(0x23, "INC HL", 0, 8, [&](auto p) { r.hl++; });
	set(0x33, "INC SP", 0, 8, [&](auto p) { r.sp++; });
	// 4. DEC nn
	set(0x0B, "DEC BC", 0, 8, [&](auto p) { r.bc--; });
	set(0x1B, "DEC DE", 0, 8, [&](auto p) { r.de--; });
	set(0x2B, "DEC HL", 0, 8, [&](auto p) { r.hl--; });
	set(0x3B, "DEC SP", 0, 8, [&](auto p) { r.sp--; });

	// 3.3.5 Miscellaneous
	// 1. SWAP n
	auto swapn = [&](uint8_t v) -> uint8_t
	{
		uint8_t res = (v >> 4) | (v << 4);
		r.flagZ = !res;
		r.flagN = 0;
		r.flagH = 0;
		r.flagC = 0;
		return res;
	};
	setCB(0x37, "SWAP A", 0, 8, [&, swapn](auto p) { r.a = swapn(r.a); });
	setCB(0x30, "SWAP B", 0, 8, [&, swapn](auto p) { r.b = swapn(r.b); });
	setCB(0x31, "SWAP C", 0, 8, [&, swapn](auto p) { r.c = swapn(r.c); });
	setCB(0x32, "SWAP D", 0, 8, [&, swapn](auto p) { r.d = swapn(r.d); });
	setCB(0x33, "SWAP E", 0, 8, [&, swapn](auto p) { r.e = swapn(r.e); });
	setCB(0x34, "SWAP H", 0, 8, [&, swapn](auto p) { r.h = swapn(r.h); });
	setCB(0x35, "SWAP L", 0, 8, [&, swapn](auto p) { r.l = swapn(r.l); });
	setCB(0x36, "SWAP (HL)", 0, 16, [&, swapn](auto p) {
		uint8_t v = m.Read(r.hl);
		v = swapn(v);
		m.Write(r.hl, v);
	});
	// 2. DAA
	set(0x27, "DAA", 0, 27, [&](auto p)
	{
		// http://www.z80.info/z80syntx.htm#DAA

		/*
		--------------------------------------------------------------------------------
		|           | C Flag  | HEX value in | H Flag | HEX value in | Number  | C flag|
		| Operation | Before  | upper digit  | Before | lower digit  | added   | After |
		|           | DAA     | (bit 7-4)    | DAA    | (bit 3-0)    | to byte | DAA   |
		|------------------------------------------------------------------------------|
		|           |    0    |     0-9      |   0    |     0-9      |   00    |   0   |
		|   ADD     |    0    |     0-8      |   0    |     A-F      |   06    |   0   |
		|           |    0    |     0-9      |   1    |     0-3      |   06    |   0   |
		|   ADC     |    0    |     A-F      |   0    |     0-9      |   60    |   1   |
		|           |    0    |     9-F      |   0    |     A-F      |   66    |   1   |
		|   INC     |    0    |     A-F      |   1    |     0-3      |   66    |   1   |
		|           |    1    |     0-2      |   0    |     0-9      |   60    |   1   |
		|           |    1    |     0-2      |   0    |     A-F      |   66    |   1   |
		|           |    1    |     0-3      |   1    |     0-3      |   66    |   1   |
		|------------------------------------------------------------------------------|
		|   SUB     |    0    |     0-9      |   0    |     0-9      |   00    |   0   |
		|   SBC     |    0    |     0-8      |   1    |     6-F      |   FA    |   0   |
		|   DEC     |    1    |     7-F      |   0    |     0-9      |   A0    |   1   |
		|   NEG     |    1    |     6-F      |   1    |     6-F      |   9A    |   1   |
		|------------------------------------------------------------------------------|
		*/
		/*
		uint8_t a = r.a;
		if ((r.a & 0xF) > 9 || r.flagH) a += 0x06;
		if ((r.a >> 4) > 9 || r.flagC) { a += 0x60; r.flagC = 1; }
		else { r.flagC = 0; }

		r.a = a;
		r.flagZ = !r.a;
		r.flagH = 0;
		*/

		uint16_t a = r.a;

		if (!r.flagN)
		{
			if (r.flagH || (a & 0xF) > 9)
				a += 0x06;
			if (r.flagC || a > 0x9F)
				a += 0x60;
		}
		else
		{
			if (r.flagH)
				a = (a - 6) & 0xFF;
			if (r.flagC)
				a -= 0x60;
		}

		r.flagH = 0;
		r.flagZ = 0;

		if ((a & 0x100) == 0x100)
			r.flagC = 1;

		a &= 0xFF;

		if (a == 0)
			r.flagZ = 1;

		r.a = uint8_t(a);
	});
	// 3. DPL
	set(0x2F, "CPL", 0, 4, [&](auto p) {
		r.a = ~r.a;
		r.flagN = 1;
		r.flagH = 1;
	});
	// 4. CCF
	set(0x3F, "CCF", 0, 4, [&](auto p) {
		r.flagN = 0;
		r.flagH = 0;
		r.flagC = !r.flagC;
	});
	// 5. SCF
	set(0x37, "SCF", 0, 4, [&](auto p) {
		r.flagN = 0;
		r.flagH = 0;
		r.flagC = 1;
	});
	// 6. NOP
	set(0x00, "NOP", 0, 4, [&](auto p) { });
	// 7. HALT
	set(0x76, "HALT", 0, 4, [&](auto p) { halted_ = true; });
	// 8. STOP
	set10(0x00, "STOP", 0, 4, [&](auto p) { });
	// 9. DI
	set(0xF3, "DI", 0, 4, [&](auto p) { interruptsEnabled_ = false; });
	// 10. EI
	set(0xFB, "EI", 0, 4, [&](auto p) { interruptsEnabled_ = true; });

	// 3.3.6 Rotates & Shifts
	// 1. RLCA
	set(0x07, "RLCA", 0, 4, [&](auto p) {
		uint8_t a0 = r.a;
		r.a <<= 1;
		r.flagZ = !r.a;
		r.flagN = 0;
		r.flagH = 0;
		r.flagC = (a0 & 0x80) == 0x80;
	});
	// 2. RLA
	set(0x17, "RLA", 0, 4, [&](auto p) {
		uint8_t a0 = r.a;
		r.a = (r.a << 1) | r.flagC;
		r.flagZ = !r.a;
		r.flagN = 0;
		r.flagH = 0;
		r.flagC = (a0 & 0x80) == 0x80;
	});
	// 3. RRCA
	set(0x0F, "RRCA", 0, 4, [&](auto p) {
		uint8_t a0 = r.a;
		r.a >>= 1;
		r.flagZ = !r.a;
		r.flagN = 0;
		r.flagH = 0;
		r.flagC = a0 & 1;
	});
	// 4. RRA
	set(0x1F, "RRA", 0, 4, [&](auto p) {
		uint8_t a0 = r.a;
		r.a = (r.a >> 1) | ((uint8_t)r.flagC << 7); // TODO verify
		r.flagZ = !r.a;
		r.flagN = 0;
		r.flagH = 0;
		r.flagC = a0 & 1;
	});
	// 5. RLC n
	auto rlcn = [&](uint8_t n) -> uint8_t {
		uint8_t n0 = n;
		uint8_t n1 = n << 1;
		r.flagZ = !n1;
		r.flagN = 0;
		r.flagH = 0;
		r.flagC = (n0 & 0x80) == 0x80;
		return n1;
	};
	setCB(0x07, "RLC A", 0, 8, [&, rlcn](auto p) { r.a = rlcn(r.a); });
	setCB(0x00, "RLC B", 0, 8, [&, rlcn](auto p) { r.b = rlcn(r.b); });
	setCB(0x01, "RLC C", 0, 8, [&, rlcn](auto p) { r.c = rlcn(r.c); });
	setCB(0x02, "RLC D", 0, 8, [&, rlcn](auto p) { r.d = rlcn(r.d); });
	setCB(0x03, "RLC E", 0, 8, [&, rlcn](auto p) { r.e = rlcn(r.e); });
	setCB(0x04, "RLC H", 0, 8, [&, rlcn](auto p) { r.h = rlcn(r.h); });
	setCB(0x05, "RLC L", 0, 8, [&, rlcn](auto p) { r.l = rlcn(r.l); });
	setCB(0x06, "RLC (HL)", 0, 16, [&, rlcn](auto p) {
		uint8_t value = m.Read(r.hl);
		value = rlcn(value);
		m.Write(r.hl, value);
	});
	// 6. RL n
	auto rln = [&](uint8_t n) -> uint8_t {
		uint8_t n0 = n;
		uint8_t n1 = (n << 1) | r.flagC;
		r.flagZ = !n1;
		r.flagN = 0;
		r.flagH = 0;
		r.flagC = (n0 & 0x80) == 0x80;
		return n1;
	};
	setCB(0x17, "RL A", 0, 8, [&, rln](auto p) { r.a = rln(r.a); });
	setCB(0x10, "RL B", 0, 8, [&, rln](auto p) { r.b = rln(r.b); });
	setCB(0x11, "RL C", 0, 8, [&, rln](auto p) { r.c = rln(r.c); });
	setCB(0x12, "RL D", 0, 8, [&, rln](auto p) { r.d = rln(r.d); });
	setCB(0x13, "RL E", 0, 8, [&, rln](auto p) { r.e = rln(r.e); });
	setCB(0x14, "RL H", 0, 8, [&, rln](auto p) { r.h = rln(r.h); });
	setCB(0x15, "RL L", 0, 8, [&, rln](auto p) { r.l = rln(r.l); });
	setCB(0x16, "RL (HL)", 0, 16, [&, rln](auto p) {
		uint8_t value = m.Read(r.hl);
		value = rln(value);
		m.Write(r.hl, value);
	});
	// 7. RRC n
	auto rrcn = [&](uint8_t n) -> uint8_t {
		uint8_t n0 = n;
		uint8_t n1 = n >> 1;
		r.flagZ = !n1;
		r.flagN = 0;
		r.flagH = 0;
		r.flagC = n0 & 1;
		return n1;
	};
	setCB(0x0F, "RRC A", 0, 8, [&, rrcn](auto p) { r.a = rrcn(r.a); });
	setCB(0x08, "RRC B", 0, 8, [&, rrcn](auto p) { r.b = rrcn(r.b); });
	setCB(0x09, "RRC C", 0, 8, [&, rrcn](auto p) { r.c = rrcn(r.c); });
	setCB(0x0A, "RRC D", 0, 8, [&, rrcn](auto p) { r.d = rrcn(r.d); });
	setCB(0x0B, "RRC E", 0, 8, [&, rrcn](auto p) { r.e = rrcn(r.e); });
	setCB(0x0C, "RRC H", 0, 8, [&, rrcn](auto p) { r.h = rrcn(r.h); });
	setCB(0x0D, "RRC L", 0, 8, [&, rrcn](auto p) { r.l = rrcn(r.l); });
	setCB(0x0E, "RRC (HL)", 0, 16, [&, rrcn](auto p) {
		uint8_t value = m.Read(r.hl);
		value = rrcn(value);
		m.Write(r.hl, value);
	});
	// 8. RR n
	auto rrn = [&](uint8_t n) -> uint8_t {
		uint8_t n0 = n;
		uint8_t n1 = (n >> 1) | ((uint8_t)r.flagC << 7); // TODO verify
		r.flagZ = !n1;
		r.flagN = 0;
		r.flagH = 0;
		r.flagC = n0 & 1;
		return n1;
	};
	setCB(0x1F, "RR A", 0, 8, [&, rrn](auto p) { r.a = rrn(r.a); });
	setCB(0x18, "RR B", 0, 8, [&, rrn](auto p) { r.b = rrn(r.b); });
	setCB(0x19, "RR C", 0, 8, [&, rrn](auto p) { r.c = rrn(r.c); });
	setCB(0x1A, "RR D", 0, 8, [&, rrn](auto p) { r.d = rrn(r.d); });
	setCB(0x1B, "RR E", 0, 8, [&, rrn](auto p) { r.e = rrn(r.e); });
	setCB(0x1C, "RR H", 0, 8, [&, rrn](auto p) { r.h = rrn(r.h); });
	setCB(0x1D, "RR L", 0, 8, [&, rrn](auto p) { r.l = rrn(r.l); });
	setCB(0x1E, "RR (HL)", 0, 16, [&, rrn](auto p) {
		uint8_t value = m.Read(r.hl);
		value = rrn(value);
		m.Write(r.hl, value);
	});
	// 9. SLA n
	auto slan = [&](uint8_t n) -> uint8_t {
		uint8_t n0 = n;
		uint8_t n1 = (n << 1);
		r.flagZ = !n1;
		r.flagN = 0;
		r.flagH = 0;
		r.flagC = (n0 & 0x80) == 0x80;
		return n1;
	};
	setCB(0x27, "SLA A", 0, 8, [& ,slan](auto p) { r.a = slan(r.a); });
	setCB(0x20, "SLA B", 0, 8, [&, slan](auto p) { r.b = slan(r.b); });
	setCB(0x21, "SLA C", 0, 8, [&, slan](auto p) { r.c = slan(r.c); });
	setCB(0x22, "SLA D", 0, 8, [&, slan](auto p) { r.d = slan(r.d); });
	setCB(0x23, "SLA E", 0, 8, [&, slan](auto p) { r.e = slan(r.e); });
	setCB(0x24, "SLA H", 0, 8, [&, slan](auto p) { r.h = slan(r.h); });
	setCB(0x25, "SLA L", 0, 8, [&, slan](auto p) { r.l = slan(r.l); });
	setCB(0x26, "SLA (HL)", 0, 16, [&, slan](auto p) {
		uint8_t value = m.Read(r.hl);
		value = slan(value);
		m.Write(r.hl, value);
	});
	// 10. SRA n
	auto sran = [&](uint8_t n) -> uint8_t {
		uint8_t n0 = n;
		uint8_t n1 = (n >> 1);
		if (n0 & 0x80) n1 |= 0x80; // TODO verify
		r.flagZ = !n1;
		r.flagN = 0;
		r.flagH = 0;
		r.flagC = n0 & 1;
		return n1;
	};
	setCB(0x2F, "SRA A", 0, 8, [&, sran](auto p) { r.a = sran(r.a); });
	setCB(0x28, "SRA B", 0, 8, [&, sran](auto p) { r.b = sran(r.b); });
	setCB(0x29, "SRA C", 0, 8, [&, sran](auto p) { r.c = sran(r.c); });
	setCB(0x2A, "SRA D", 0, 8, [&, sran](auto p) { r.d = sran(r.d); });
	setCB(0x2B, "SRA E", 0, 8, [&, sran](auto p) { r.e = sran(r.e); });
	setCB(0x2C, "SRA H", 0, 8, [&, sran](auto p) { r.h = sran(r.h); });
	setCB(0x2D, "SRA L", 0, 8, [&, sran](auto p) { r.l = sran(r.l); });
	setCB(0x2E, "SRA (HL)", 0, 16, [&, sran](auto p) {
		uint8_t value = m.Read(r.hl);
		value = sran(value);
		m.Write(r.hl, value);
	});
	// 11. SRL n
	auto srln = [&](uint8_t n) -> uint8_t {
		uint8_t n0 = n;
		uint8_t n1 = (n >> 1);
		r.flagZ = !n1;
		r.flagN = 0;
		r.flagH = 0;
		r.flagC = n0 & 1;
		return n1;
	};
	setCB(0x3F, "SRL A", 0, 8, [&, srln](auto p) { r.a = srln(r.a); });
	setCB(0x38, "SRL B", 0, 8, [&, srln](auto p) { r.b = srln(r.b); });
	setCB(0x39, "SRL C", 0, 8, [&, srln](auto p) { r.c = srln(r.c); });
	setCB(0x3A, "SRL D", 0, 8, [&, srln](auto p) { r.d = srln(r.d); });
	setCB(0x3B, "SRL E", 0, 8, [&, srln](auto p) { r.e = srln(r.e); });
	setCB(0x3C, "SRL H", 0, 8, [&, srln](auto p) { r.h = srln(r.h); });
	setCB(0x3D, "SRL L", 0, 8, [&, srln](auto p) { r.l = srln(r.l); });
	setCB(0x3E, "SRL (HL)", 0, 16, [&, srln](auto p) {
		uint8_t value = m.Read(r.hl);
		value = srln(value);
		m.Write(r.hl, value);
	});

	// 3.3.7 Bit Opcodes

	for (uint16_t i = 0x40; i <= 0xFF; i++)
	{
		setCB2(uint8_t(i), "Bit xxx", 0, 8, [&](auto op, auto p) {
			Bitops(op);
		});
	}

	// 3.3.8 Jumps
	// 1. JP nn
	set(0xC3, "JP nn", 2, 12, [&](auto p) { r.pc = p[0] | (p[1] << 8); });
	// 2. JP cc,nn
	set(0xC2, "JP NZ,nn", 2, 12, [&](auto p) { if (!r.flagZ) r.pc = p[0] | (p[1] << 8); });
	set(0xCA, "JP Z,nn", 2, 12, [&](auto p) { if (r.flagZ) r.pc = p[0] | (p[1] << 8); });
	set(0xD2, "JP NC,nn", 2, 12, [&](auto p) { if (!r.flagC) r.pc = p[0] | (p[1] << 8); });
	set(0xDA, "JP C,nn", 2, 12, [&](auto p) { if (r.flagC) r.pc = p[0] | (p[1] << 8); });
	// 3. JP (HL)
	set(0xE9, "JP nn", 0, 4, [&](auto p) { r.pc = r.hl; }); // TODO verify
	// 4. JR n
	set(0x18, "JR n", 1, 8, [&](auto p) {
		int8_t n = reinterpret_cast<int8_t&>(p[0]);
		r.pc += n;
	});
	// 5. JR cc, n
	set(0x20, "JR NZ,n", 1, 8, [&](auto p) { if (!r.flagZ) r.pc += reinterpret_cast<int8_t&>(p[0]); });
	set(0x28, "JR Z,n", 1, 8, [&](auto p) { if (r.flagZ) r.pc += reinterpret_cast<int8_t&>(p[0]); });
	set(0x30, "JR NC,n", 1, 8, [&](auto p) { if (!r.flagC) r.pc += reinterpret_cast<int8_t&>(p[0]); });
	set(0x38, "JR C,n", 1, 8, [&](auto p) { if (r.flagC) r.pc += reinterpret_cast<int8_t&>(p[0]); });

	// 3.3.9 Calls
	// 1. Call nn
	set(0xCD, "CALL nn", 2, 12, [&,push16](auto p) {
		push16(r.pc);
		r.pc = p[0] | (p[1] << 8);
	});
	// 2. Call cc,nn
	set(0xC4, "CALL NZ,nn", 2, 12, [&, push16](auto p) {
		if (!r.flagZ) {
			push16(r.pc);
			r.pc = p[0] | (p[1] << 8);
		}
	});
	set(0xCC, "CALL Z,nn", 2, 12, [&, push16](auto p) {
		if (r.flagZ) {
			push16(r.pc);
			r.pc = p[0] | (p[1] << 8);
		}
	});
	set(0xD4, "CALL NC,nn", 2, 12, [&, push16](auto p) {
		if (!r.flagC) {
			push16(r.pc);
			r.pc = p[0] | (p[1] << 8);
		}
	});
	set(0xDC, "CALL C,nn", 2, 12, [&, push16](auto p) {
		if (r.flagC) {
			push16(r.pc);
			r.pc = p[0] | (p[1] << 8);
		}
	});

	// 3.3.10 Restarts
	// 1. RST n
	set(0xC7, "RST 00", 0, 32, [&, push16](auto p) { push16(r.pc); r.pc = 0x00; });
	set(0xCF, "RST 08", 0, 32, [&, push16](auto p) { push16(r.pc); r.pc = 0x08; });
	set(0xD7, "RST 10", 0, 32, [&, push16](auto p) { push16(r.pc); r.pc = 0x10; });
	set(0xDF, "RST 18", 0, 32, [&, push16](auto p) { push16(r.pc); r.pc = 0x18; });
	set(0xE7, "RST 20", 0, 32, [&, push16](auto p) { push16(r.pc); r.pc = 0x20; });
	set(0xEF, "RST 28", 0, 32, [&, push16](auto p) { push16(r.pc); r.pc = 0x28; });
	set(0xF7, "RST 30", 0, 32, [&, push16](auto p) { push16(r.pc); r.pc = 0x30; });
	set(0xFF, "RST 38", 0, 32, [&, push16](auto p) { push16(r.pc); r.pc = 0x38; });

	// 3.3.11 Returns
	// 1. RET
	set(0xC9, "RET", 0, 8, [&, pop16](auto p) { r.pc = pop16(); });
	// 2. RET cc
	set(0xC0, "RET NZ", 0, 8, [&, pop16](auto p) { if (!r.flagZ) r.pc = pop16(); });
	set(0xC8, "RET Z", 0, 8, [&, pop16](auto p) { if (r.flagZ) r.pc = pop16(); });
	set(0xD0, "RET NC", 0, 8, [&, pop16](auto p) { if (!r.flagC) r.pc = pop16(); });
	set(0xD8, "RET C", 0, 8, [&, pop16](auto p) { if (r.flagC) r.pc = pop16(); });
	// 3. RETI
	set(0xD9, "RETI", 0, 8, [&, pop16](auto p) {
		r.pc = pop16();
		interruptsEnabled_ = true;
	});
}

void Cpu::Reset()
{
	regs_.af = 0x01B0;
	regs_.bc = 0x0013;
	regs_.de = 0x00D8;
	regs_.hl = 0x014D;
	regs_.sp = 0xFFFE; // TODO populate default stack values
	regs_.pc = 0x0100;

	interruptsEnabled_ = false;
	halted_ = false;
}

uint32_t Cpu::Tick()
{
	uint32_t ticks = 0;

	// Halted?
	if (halted_)
	{
		if (log_.StateEnabled())
			log_.State("Halted");

		if (pic_.InterruptsPending())
		{
			if (log_.StateEnabled())
				log_.State("Wake up from Halt");
			halted_ = false;
		}

		// xxx
		// TODO mario stays in the halt state forever. whats wrong? possible something related to the display.
		// xxx
		//halted_ = false;

		ticks += 0; // TODO
		return ticks;
	}

	// Interrupt?
	if (interruptsEnabled_)
	{
		uint8_t interruptMask = pic_.GetAndClearInterrupt();
		if (interruptMask)
		{
			interruptsEnabled_ = false;

			Push16(regs_.pc);

			if (interruptMask & INT_VBLANK) regs_.pc = 0x0040;
			else if (interruptMask & INT_LCDC) regs_.pc = 0x0048;
			else if (interruptMask & INT_TIMER) regs_.pc = 0x0050;
			else if (interruptMask & INT_SERIAL) regs_.pc = 0x0058;
			else if (interruptMask & INT_PIN) regs_.pc = 0x0060;
			else assert(0);

			if (log_.InterruptEnabled())
				log_.Interrupt("Handle " + AsHexString(interruptMask) + " at " + AsHexString(regs_.pc));

			ticks += 0; // TODO
			return ticks;
		}
	}

	uint16_t instructionLength = 0;

	// Fetch next opcode.
	uint16_t opcodeAddr = regs_.pc;
	uint8_t opcode = memory_.Read(opcodeAddr);
	instructionLength++;

	std::array<Instruction, 256> *table = nullptr;

	if (opcode == 0xCB)
	{
		opcodeAddr++;
		opcode = memory_.Read(opcodeAddr);
		instructionLength++;

		table = &instructionsCB_;
	}
	else if (opcode == 0x10)
	{
		opcodeAddr++;
		opcode = memory_.Read(opcodeAddr);
		instructionLength++;

		table = &instructions10_;
	}
	else
	{
		table = &instructions_;
	}

	const auto& instruction = (*table)[opcode];

	// Read in operands.
	uint8_t operands[8] = {};
	for (uint16_t operandIndex = 0;
		operandIndex < instruction.operands;
		operandIndex++)
	{
		operands[operandIndex] = memory_.Read(regs_.pc + 1 + operandIndex);
		instructionLength++;
	}

	// Check for invalid instructions.
	if (!instruction.handler && !instruction.handler2)
	{
		log_.Error("Invalid instruction " + AsHexString(opcode) + " at " + AsHexString(opcodeAddr));

		assert(0);
		getchar();
		exit(1);
	}

	// Execute instruction.
	if (log_.InstructionEnabled())
		log_.Instruction(AsHexString(regs_.pc) + " -> " + instruction.name);

	regs_.pc += instructionLength;

	if (instruction.handler) instruction.handler(operands);
	else if (instruction.handler2) instruction.handler2(opcode, operands);
	else assert(0);

	ticks += instruction.ticks;

	regs_.f &= 0xF0; // TODO lower bits must be hardwired to 0

	if (log_.StateEnabled())
		log_.State(regs_.ToString());

	// Done. Return number of consumed ticks.
	return ticks;
}

void Cpu::Push8(uint8_t v)
{
	regs_.sp--;
	memory_.Write(regs_.sp, v);
}

void Cpu::Push16(uint16_t v)
{
	uint8_t l = v & 0xFF;
	uint8_t h = v >> 8;
	regs_.sp -= 2;
	memory_.Write(regs_.sp + 0, l);
	memory_.Write(regs_.sp + 1, h);
}

uint8_t Cpu::Pop8()
{
	uint8_t v = memory_.Read(regs_.sp);
	regs_.sp++;
	return v;
}

uint16_t Cpu::Pop16()
{
	uint8_t l = memory_.Read(regs_.sp + 0);
	uint8_t h = memory_.Read(regs_.sp + 1);
	regs_.sp += 2;
	return l | (h << 8);
}

void Cpu::Bitops(uint8_t opcode)
{
	uint8_t op = opcode >> 6;
	uint8_t bit = (opcode & 0x38) >> 3;
	uint8_t reg = opcode & 0x7;
	assert(bit < 8);
	assert(reg < 8);
	assert(op);

	// Read value.
	uint8_t v0 = 0;
	switch (reg)
	{
	case 7: v0 = regs_.a; break;
	case 0: v0 = regs_.b; break;
	case 1: v0 = regs_.c; break;
	case 2: v0 = regs_.d; break;
	case 3: v0 = regs_.e; break;
	case 4: v0 = regs_.h; break;
	case 5: v0 = regs_.l; break;
	case 6: v0 = memory_.Read(regs_.hl); break;
	}

	// Perform operation.
	uint8_t v1 = 0;
	if (op == 1)
	{
		// BIT
		regs_.flagZ = (v0 & (1 << bit)) == 0;
		regs_.flagN = 0;
		regs_.flagH = 1;
	}
	else if (op == 2)
	{
		// RES
		v1 = v0 & ~(1 << bit);
	}
	else if (op == 3)
	{
		// SET
		v1 = v0 | (1 << bit);
	}

	// Write value.
	if (op == 2 || op == 3)
	{
		switch (reg)
		{
		case 7: regs_.a = v1; break;
		case 0: regs_.b = v1; break;
		case 1: regs_.c = v1; break;
		case 2: regs_.d = v1; break;
		case 3: regs_.e = v1; break;
		case 4: regs_.h = v1; break;
		case 5: regs_.l = v1; break;
		case 6: memory_.Write(regs_.hl, v1); break;
		}
	}
}

}
