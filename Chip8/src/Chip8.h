#pragma once
#include <cstdint>
#include <chrono>
#include <random>

class Chip8
{

public:
	// chip8 have 16 - 8bit registers
	uint8_t registers[16]{};

	// 4K of ram 4 * 1024 = 4096 bytes
	uint8_t memory[4096]{};

	// 16bit index register
	uint16_t index{};

	// PC: program counter, 16bit because of 4k of memory
	// addresses can go to 0xFFF so we need a PC of size bigger than 8bits
	uint16_t pc{};

	// stack of 16 level
	uint16_t stack[16]{};
	uint8_t sp{}; // stack pointer

	uint8_t delayTimer{};
	uint8_t soundTimer{};

	uint8_t keypad[16]{};

	uint8_t VIDEO_WIDTH = 64;
	uint8_t VIDEO_HEIGHT = 32;

	uint32_t video[64 * 32]{}; // display

	uint16_t opcode;

	const unsigned int FONTSET_SIZE = 80;

	uint8_t fontset[80] =
	{
		0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
		0x20, 0x60, 0x20, 0x20, 0x70, // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
		0x90, 0x90, 0xF0, 0x10, 0x10, // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
		0xF0, 0x10, 0x20, 0x40, 0x40, // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90, // A
		0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
		0xF0, 0x80, 0x80, 0x80, 0xF0, // C
		0xE0, 0x90, 0x90, 0x90, 0xE0, // D
		0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
		0xF0, 0x80, 0xF0, 0x80, 0x80  // F
	};

private:
	const unsigned int START_ADDRESS = 0x200;
	const unsigned int FONTSET_START_ADDRESS = 0x50;

	std::default_random_engine randGen;
	std::uniform_int_distribution<unsigned int> randByte;


private:
	void Table0();
	void Table8();
	void TableE();
	void TableF();

	void OP_NULL();
	void OP_00E0(); void OP_00EE(); void OP_1nnn();
	void OP_2nnn(); void OP_3xkk(); void OP_4xkk();
	void OP_5xy0(); void OP_6xkk(); void OP_7xkk();
	void OP_8xy0(); void OP_8xy1(); void OP_8xy2();
	void OP_8xy3(); void OP_8xy4(); void OP_8xy5();
	void OP_8xy6(); void OP_8xy7(); void OP_8xyE();
	void OP_9xy0(); void OP_Annn(); void OP_Bnnn();
	void OP_Cxkk(); void OP_Dxyn(); void OP_Ex9E();
	void OP_ExA1(); void OP_Fx07(); void OP_Fx0A();
	void OP_Fx15(); void OP_Fx18(); void OP_Fx1E();
	void OP_Fx29(); void OP_Fx33(); void OP_Fx55();
	void OP_Fx65();

	// type function pointer
	typedef void (Chip8::*Chip8Func)();

	Chip8Func table[0xF + 1]{ &Chip8::OP_NULL };
	Chip8Func table0[0xE + 1]{ &Chip8::OP_NULL };
	Chip8Func table8[0xE + 1]{ &Chip8::OP_NULL };
	Chip8Func tableE[0xE + 1]{ &Chip8::OP_NULL };
	Chip8Func tableF[0x65 + 1]{ &Chip8::OP_NULL };

public:
	Chip8();

	void LoadRom(const char* filename);
	void Cycle();
};

