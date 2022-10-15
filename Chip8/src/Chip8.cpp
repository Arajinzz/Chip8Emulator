#include "Chip8.h"
#include <fstream>
#include <iostream>

Chip8::Chip8():
	opcode(0x0),
	randGen(std::chrono::system_clock::now().time_since_epoch().count())
{
	pc = START_ADDRESS;

	for (uint8_t i = 0; i < FONTSET_SIZE; i++) {
		memory[FONTSET_START_ADDRESS + i] = fontset[i];
	}

	// we will need that in an instruction that place a random number in register
	randByte = std::uniform_int_distribution<unsigned int>(0, 255U);

	// Set up function pointer table
	table[0x0] = &Chip8::Table0;
	table[0x1] = &Chip8::OP_1nnn;
	table[0x2] = &Chip8::OP_2nnn;
	table[0x3] = &Chip8::OP_3xkk;
	table[0x4] = &Chip8::OP_4xkk;
	table[0x5] = &Chip8::OP_5xy0;
	table[0x6] = &Chip8::OP_6xkk;
	table[0x7] = &Chip8::OP_7xkk;
	table[0x8] = &Chip8::Table8;
	table[0x9] = &Chip8::OP_9xy0;
	table[0xA] = &Chip8::OP_Annn;
	table[0xB] = &Chip8::OP_Bnnn;
	table[0xC] = &Chip8::OP_Cxkk;
	table[0xD] = &Chip8::OP_Dxyn;
	table[0xE] = &Chip8::TableE;
	table[0xF] = &Chip8::TableF;

	table0[0x0] = &Chip8::OP_00E0;
	table0[0xE] = &Chip8::OP_00EE;

	table8[0x0] = &Chip8::OP_8xy0;
	table8[0x1] = &Chip8::OP_8xy1;
	table8[0x2] = &Chip8::OP_8xy2;
	table8[0x3] = &Chip8::OP_8xy3;
	table8[0x4] = &Chip8::OP_8xy4;
	table8[0x5] = &Chip8::OP_8xy5;
	table8[0x6] = &Chip8::OP_8xy6;
	table8[0x7] = &Chip8::OP_8xy7;
	table8[0xE] = &Chip8::OP_8xyE;

	tableE[0x1] = &Chip8::OP_ExA1;
	tableE[0xE] = &Chip8::OP_Ex9E;

	tableF[0x07] = &Chip8::OP_Fx07;
	tableF[0x0A] = &Chip8::OP_Fx0A;
	tableF[0x15] = &Chip8::OP_Fx15;
	tableF[0x18] = &Chip8::OP_Fx18;
	tableF[0x1E] = &Chip8::OP_Fx1E;
	tableF[0x29] = &Chip8::OP_Fx29;
	tableF[0x33] = &Chip8::OP_Fx33;
	tableF[0x55] = &Chip8::OP_Fx55;
	tableF[0x65] = &Chip8::OP_Fx65;

}

void Chip8::LoadRom(const char* filename) {

	// open file as a stream of binary and move file pointer to the end
	std::ifstream file(filename, std::ios::binary | std::ios::ate);

	if (file.is_open()) {
		// Get size of file and allocate a buffer to hold the contents
		std::streampos size = file.tellg();
		char* buffer = new char[size];

		// Go back to the beginning of the file and fill the buffer
		file.seekg(0, std::ios::beg);
		file.read(buffer, size);
		file.close();

		// load rom onto memory beginning from 0x200
		for (long i = 0; i < size; i++) {
			memory[START_ADDRESS + i] = buffer[i];
		}

		// free memory
		delete[] buffer;

	}
	else {
		std::cout << "Could not Open file" << std::endl;
	}

}

void Chip8::Cycle()
{

	// fetch opcode
	opcode = (memory[pc] << 8) | (memory[pc + 1]);

	// increment pc
	pc += 2;

	// Decode and Execute
	((*this).*(table[(opcode & 0xF000) >> 12]))();

	// Decrement the delay timer if it's been set
	if (delayTimer > 0)
	{
		--delayTimer;
	}

	// Decrement the sound timer if it's been set
	if (soundTimer > 0)
	{
		--soundTimer;
	}
}


void Chip8::Table0()
{
	((*this).*(table0[opcode & 0x000Fu]))();
}

void Chip8::Table8()
{
	((*this).*(table8[opcode & 0x000Fu]))();
}

void Chip8::TableE()
{
	((*this).*(tableE[opcode & 0x000Fu]))();
}

void Chip8::TableF()
{
	((*this).*(tableF[opcode & 0x00FFu]))();
}


void Chip8::OP_NULL()
{
}

// clear display
void Chip8::OP_00E0()
{
	memset(video, 0, sizeof(video));
}

// RET
void Chip8::OP_00EE()
{
	sp--;
	pc = stack[sp];
}

// set pc to adress nnn
// JP addr
void Chip8::OP_1nnn()
{
	// to clarify
	uint16_t address = opcode & 0x0FFF;
	pc = address;
}

// call subroutine
// CALL addr
void Chip8::OP_2nnn()
{
	stack[sp] = pc;
	sp++;
	pc = opcode & 0x0FFF;
}

// if vx == kk skip next instruction
// SE Vx, byte
void Chip8::OP_3xkk()
{
	uint8_t Vx = (opcode & 0x0F00) >> 8;
	uint8_t byte = opcode & 0x00FF;
	if (registers[Vx] == byte)
		pc += 2;
}

// 4xkk - SNE Vx, byte
// Skip next instruction if Vx != kk
void Chip8::OP_4xkk()
{
	uint8_t Vx = (opcode & 0x0F00) >> 8;
	uint8_t byte = opcode & 0x00FF;
	if (registers[Vx] != byte)
		pc += 2;
}

// 5xy0 - SE Vx, Vy
// Skip next instruction if Vx = Vy.
// The interpreter compares register Vx to register Vy, and if they are equal, increments the program counter by 2.
void Chip8::OP_5xy0()
{
	uint8_t Vx = (opcode & 0x0F00) >> 8;
	uint8_t Vy = (opcode & 0x00F0) >> 4;
	if (registers[Vx] == registers[Vy])
		pc += 2;
}

/*
6xkk - LD Vx, byte
Set Vx = kk.
The interpreter puts the value kk into register Vx.
*/
void Chip8::OP_6xkk()
{
	uint8_t Vx = (opcode & 0x0F00) >> 8;
	uint8_t byte = opcode & 0x00FF;
	registers[Vx] = byte;
}

/*
7xkk - ADD Vx, byte
Set Vx = Vx + kk.
Adds the value kk to the value of register Vx, then stores the result in Vx.
*/
void Chip8::OP_7xkk()
{
	uint8_t Vx = (opcode & 0x0F00) >> 8;
	uint8_t byte = opcode & 0x00FF;
	registers[Vx] += byte;
}

/*
8xy0 - LD Vx, Vy
Set Vx = Vy.
Stores the value of register Vy in register Vx.
*/
void Chip8::OP_8xy0()
{
	uint8_t Vx = (opcode & 0x0F00) >> 8;
	uint8_t Vy = (opcode & 0x00F0) >> 4;
	registers[Vx] = registers[Vy];
}

/*
* 8xy1 - OR Vx, Vy
Set Vx = Vx OR Vy.
Performs a bitwise OR on the values of Vx and Vy, then stores the result in Vx.
*/
void Chip8::OP_8xy1()
{
	uint8_t Vx = (opcode & 0x0F00) >> 8;
	uint8_t Vy = (opcode & 0x00F0) >> 4;
	registers[Vx] = registers[Vx] | registers[Vy];
}

/*
* 8xy2 - AND Vx, Vy
Set Vx = Vx AND Vy.
*/
void Chip8::OP_8xy2()
{
	uint8_t Vx = (opcode & 0x0F00) >> 8;
	uint8_t Vy = (opcode & 0x00F0) >> 4;
	registers[Vx] = registers[Vx] & registers[Vy];
}

/*
* 8xy3 - XOR Vx, Vy
Set Vx = Vx XOR Vy.
*/
void Chip8::OP_8xy3()
{
	uint8_t Vx = (opcode & 0x0F00) >> 8;
	uint8_t Vy = (opcode & 0x00F0) >> 4;
	registers[Vx] = registers[Vx] ^ registers[Vy];
}

/*
8xy4 - ADD Vx, Vy
Set Vx = Vx + Vy, set VF = carry.
The values of Vx and Vy are added together. If the result is greater than 8 bits (i.e., > 255,) VF is set to 1,
otherwise 0. Only the lowest 8 bits of the result are kept, and stored in Vx.
*/
void Chip8::OP_8xy4()
{
	uint8_t Vx = (opcode & 0x0F00) >> 8;
	uint8_t Vy = (opcode & 0x00F0) >> 4;

	uint16_t sum = registers[Vx] + registers[Vy];

	if (sum > 255)
		registers[0xF] = 1;
	else
		registers[0xF] = 0;

	registers[Vx] = sum & 0x00FF;
}

/*
8xy5 - SUB Vx, Vy
Set Vx = Vx - Vy, set VF = NOT borrow.
If Vx > Vy, then VF is set to 1, otherwise 0. Then Vy is subtracted from Vx, and the results stored in Vx.
*/
void Chip8::OP_8xy5()
{
	uint8_t Vx = (opcode & 0x0F00) >> 8;
	uint8_t Vy = (opcode & 0x00F0) >> 4;

	if (registers[Vx] > registers[Vy])
		registers[0xF] = 1;
	else
		registers[0xF] = 0;
	
	registers[Vx] -= registers[Vy];

}

/*
8xy6 - SHR Vx {, Vy}
Set Vx = Vx SHR 1.
If the least-significant bit of Vx is 1, then VF is set to 1, otherwise 0.
Then Vx is divided by 2.
*/
void Chip8::OP_8xy6()
{
	uint8_t Vx = (opcode & 0x0F00) >> 8;
	// Save LSB in VF
	registers[0xF] = (registers[Vx] & 0x1);
	registers[Vx] >>= 1;
}

/*
8xy7 - SUBN Vx, Vy
Set Vx = Vy - Vx, set VF = NOT borrow.
If Vy > Vx, then VF is set to 1, otherwise 0. Then Vx is subtracted from Vy, and the results stored in Vx.
*/
void Chip8::OP_8xy7()
{
	uint8_t Vx = (opcode & 0x0F00) >> 8;
	uint8_t Vy = (opcode & 0x00F0) >> 4;

	if (registers[Vy] > registers[Vx])
		registers[0xF] = 1;
	else
		registers[0xF] = 0;

	registers[Vx] = registers[Vy] - registers[Vx];

}

/*
8xyE - SHL Vx {, Vy}
Set Vx = Vx SHL 1.
If the most-significant bit of Vx is 1, then VF is set to 1, otherwise to 0. Then Vx is multiplied by 2.
*/
void Chip8::OP_8xyE()
{
	uint8_t Vx = (opcode & 0x0F00) >> 8;
	// Save MSB in VF
	registers[0xF] = (registers[Vx] & 0x80) >> 7;
	registers[Vx] <<= 1;
}

/*
9xy0 - SNE Vx, Vy
Skip next instruction if Vx != Vy.
The values of Vx and Vy are compared, and if they are not equal, the program counter is increased by 2.
*/
void Chip8::OP_9xy0()
{
	uint8_t Vx = (opcode & 0x0F00) >> 8;
	uint8_t Vy = (opcode & 0x00F0) >> 4;

	if (registers[Vx] != registers[Vy])
		pc += 2;
}

/*
Annn - LD I, addr
Set I = nnn.
The value of register I is set to nnn.
*/
void Chip8::OP_Annn()
{
	index = opcode & 0x0FFF;
}

/*
Bnnn - JP V0, addr
Jump to location nnn + V0.
The program counter is set to nnn plus the value of V0.
*/
void Chip8::OP_Bnnn()
{
	uint16_t addr = opcode & 0x0FFF;
	pc = addr + registers[0];
}

/*
Cxkk - RND Vx, byte
Set Vx = random byte AND kk.
The interpreter generates a random number from 0 to 255,
which is then ANDed with the value kk. The results are stored in Vx.
See instruction 8xy2 for more information on AND.
*/
void Chip8::OP_Cxkk()
{
	uint8_t random_byte = randByte(randGen);
	uint8_t Vx = (opcode & 0x0F00) >> 8;
	uint8_t kk = opcode & 0x00FF;

	registers[Vx] = random_byte & kk;
}

/*
Dxyn - DRW Vx, Vy, nibble
Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
The interpreter reads n bytes from memory, starting at the address stored in I.
These bytes are then displayed as sprites on screen at coordinates (Vx, Vy).
Sprites are XORed onto the existing screen.
If this causes any pixels to be erased, VF is set to 1, otherwise it is set to 0.
If the sprite is positioned so part of it is outside the coordinates of the display, 
it wraps around to the opposite side of the screen. See instruction 8xy3 for more information on XOR,
and section 2.4, Display, for more information on the Chip-8 screen and sprites.
*/
// http://www.emulator101.com/chip-8-sprites.html
void Chip8::OP_Dxyn()
{
	uint8_t Vx = (opcode & 0x0F00) >> 8;
	uint8_t Vy = (opcode & 0x00F0) >> 4;
	uint8_t height = opcode & 0x000F;

	Vx = registers[Vx] % VIDEO_WIDTH;
	Vy = registers[Vy] % VIDEO_HEIGHT;

	for (uint8_t row = 0; row < height; row++) {
		uint8_t spritebyte = memory[index + row];

		for (uint8_t col = 0; col < 8; col++) {
			uint8_t spritepixel = spritebyte & (0x80 >> col);
			uint32_t* displaypixel = &video[(Vy + row) * VIDEO_WIDTH + (Vx + col)];

			// sprite is on
			if (spritepixel) {
				// erased
				if (*displaypixel == 0xFFFFFFFF)
					registers[0xF] = 1;
				*displaypixel ^= 0xFFFFFFFF;
			}
		}
		
	}
}

/*
Ex9E - SKP Vx
Skip next instruction if key with the value of Vx is pressed.
Checks the keyboard, and if the key corresponding to the value of Vx is currently in the down position, PC is increased by 2.
*/
void Chip8::OP_Ex9E()
{
	uint8_t Vx = (opcode & 0x0F00) >> 8;
	uint8_t key = registers[Vx];

	if (keypad[key])
		pc += 2;

}

/*
ExA1 - SKNP Vx
Skip next instruction if key with the value of Vx is not pressed.
Checks the keyboard, and if the key corresponding to the value of Vx is currently in the up position, PC is increased by 2.
*/
void Chip8::OP_ExA1()
{
	uint8_t Vx = (opcode & 0x0F00) >> 8;
	uint8_t key = registers[Vx];

	if (!keypad[key])
		pc += 2;
}

/*
Fx07 - LD Vx, DT
Set Vx = delay timer value.
The value of DT is placed into Vx.
*/
void Chip8::OP_Fx07()
{
	uint8_t Vx = (opcode & 0x0F00) >> 8;
	registers[Vx] = delayTimer;
}

/*
Fx0A - LD Vx, K
Wait for a key press, store the value of the key in Vx.
All execution stops until a key is pressed, then the value of that key is stored in Vx.
*/
void Chip8::OP_Fx0A()
{
	uint8_t Vx = (opcode & 0x0F00) >> 8;
	
	if (keypad[0])
	{
		registers[Vx] = 0;
	}
	else if (keypad[1])
	{
		registers[Vx] = 1;
	}
	else if (keypad[2])
	{
		registers[Vx] = 2;
	}
	else if (keypad[3])
	{
		registers[Vx] = 3;
	}
	else if (keypad[4])
	{
		registers[Vx] = 4;
	}
	else if (keypad[5])
	{
		registers[Vx] = 5;
	}
	else if (keypad[6])
	{
		registers[Vx] = 6;
	}
	else if (keypad[7])
	{
		registers[Vx] = 7;
	}
	else if (keypad[8])
	{
		registers[Vx] = 8;
	}
	else if (keypad[9])
	{
		registers[Vx] = 9;
	}
	else if (keypad[10])
	{
		registers[Vx] = 10;
	}
	else if (keypad[11])
	{
		registers[Vx] = 11;
	}
	else if (keypad[12])
	{
		registers[Vx] = 12;
	}
	else if (keypad[13])
	{
		registers[Vx] = 13;
	}
	else if (keypad[14])
	{
		registers[Vx] = 14;
	}
	else if (keypad[15])
	{
		registers[Vx] = 15;
	}
	else
	{
		pc -= 2;
	}
}

/*
Fx15 - LD DT, Vx
Set delay timer = Vx.
DT is set equal to the value of Vx.
*/
void Chip8::OP_Fx15()
{
	uint8_t Vx = (opcode & 0x0F00) >> 8;
	delayTimer = registers[Vx];
}

/*
Fx18 - LD ST, Vx
Set sound timer = Vx.
ST is set equal to the value of Vx.
*/
void Chip8::OP_Fx18()
{
	uint8_t Vx = (opcode & 0x0F00) >> 8;
	soundTimer = registers[Vx];
}

/*
Fx1E - ADD I, Vx
Set I = I + Vx.
The values of I and Vx are added, and the results are stored in I.
*/
void Chip8::OP_Fx1E()
{
	uint8_t Vx = (opcode & 0x0F00) >> 8;
	index += registers[Vx];
}

/*
Fx29 - LD F, Vx
Set I = location of sprite for digit Vx.
The value of I is set to the location for the hexadecimal sprite corresponding to the value of Vx.
See section 2.4, Display, for more information on the Chip-8 hexadecimal font.
*/
void Chip8::OP_Fx29()
{
	uint8_t Vx = (opcode & 0x0F00) >> 8;
	uint8_t digit = registers[Vx];
	index = FONTSET_START_ADDRESS + (5 * digit);
}

/*
Fx33 - LD B, Vx
Store BCD representation of Vx in memory locations I, I+1, and I+2.
The interpreter takes the decimal value of Vx, and places the hundreds digit in memory at location in I,
the tens digit at location I+1, and the ones digit at location I+2.
*/
void Chip8::OP_Fx33()
{
	uint8_t Vx = (opcode & 0x0F00) >> 8;
	uint8_t value = registers[Vx];

	// Ones-place
	memory[index + 2] = value % 10;
	value /= 10;

	// Tens-place
	memory[index + 1] = value % 10;
	value /= 10;

	// Hundreds-place
	memory[index] = value % 10;
}

/*
Fx55 - LD [I], Vx
Store registers V0 through Vx in memory starting at location I.
The interpreter copies the values of registers V0 through Vx into memory, starting at the address in I.
*/
void Chip8::OP_Fx55()
{
	uint8_t Vx = (opcode & 0x0F00) >> 8;

	for (uint8_t i = 0; i <= Vx; ++i)
	{
		memory[index + i] = registers[i];
	}
}

/*
Fx65 - LD Vx, [I]
Read registers V0 through Vx from memory starting at location I.
The interpreter reads values from memory starting at location I into registers V0 through Vx.
*/
void Chip8::OP_Fx65()
{
	uint8_t Vx = (opcode & 0x0F00) >> 8;

	for (uint8_t i = 0; i <= Vx; ++i)
	{
		registers[i] = memory[index + i];
	}
}
