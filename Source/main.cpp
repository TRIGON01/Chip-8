#include<iostream>
#include<fstream>
#include<chrono>
#include<random>

using byte=unsigned char;
using word=unsigned short;
using u32=unsigned int;

static constexpr u32 MAXMEM=4096;
static constexpr u32 mem_limit_upper=0xfff;
static constexpr u32 mem_limit_lower=0x200;
static constexpr int FONTSET_SIZE = 80;
static constexpr int FONTSET_START_ADDRESS = 0x50;


struct CPU
{
//16 8 bit registers V0-VF
byte V0;
byte V1;
byte V2;
byte V3;
byte V4;
byte V5;
byte V6;
byte V7;
byte V8;
byte V9;
byte VA;
byte VB;
byte VC;
byte VD;
byte VE;
byte VF;


word IR;//16 bit index register

byte SP; //8 bit Stack pointer
word PC;//16 bit program counter

byte delay_timer;//8 bit delay timer
byte sound_timer; // 8 bit sound timer

bool frame_buffer[64][32];
byte stack[64];

byte memory[MAXMEM];


uint8_t fontset[FONTSET_SIZE] =
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
//Reset function
void reset()
{
    PC=mem_limit_lower;

    // Load fonts into memory
	for (unsigned int i = 0; i < FONTSET_SIZE; ++i)
	{
		memory[FONTSET_START_ADDRESS + i] = fontset[i];
	}


}


//Loading a rom
void LoadROM(char const* filename)
{
	// Open the file as a stream of binary and move the file pointer to the end
	std::ifstream file(filename, std::ios::binary | std::ios::ate);

	if (file.is_open())
	{
		// Get size of file and allocate a buffer to hold the contents
		std::streampos size = file.tellg();
		char* buffer = new char[size];

		// Go back to the beginning of the file and fill the buffer
		file.seekg(0, std::ios::beg);
		file.read(buffer, size);
		file.close();

		// Load the ROM contents into the Chip8's memory, starting at 0x200
		for (long i = 0; i < size; ++i)
		{
			memory[mem_limit_lower + i] = buffer[i];
		}

		// Free the buffer
		delete[] buffer;
	}
}


void execute(word PC)
{
    word ins=PC;
    PC+=2;
    switch(ins)
    {
            
    }
}
};

int main()
{
    CPU cpu;
    
    return 0;
}