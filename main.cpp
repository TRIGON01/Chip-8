#include<iostream>
#include<fstream>
#include<chrono>
#include<random>
#include<memory.h>
#include<iostream>
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

using namespace std;


static constexpr uint32_t MAXMEM=4096;
static constexpr uint32_t mem_limit_upper=0xfff;
static constexpr uint32_t mem_limit_lower=0x200;
static constexpr int FONTSET_SIZE = 80;
static constexpr int FONTSET_START_ADDRESS = 0x50;
static constexpr int VIDEO_WIDTH = 64;
static constexpr int VIDEO_HEIGHT = 32;

class CPU
{
public:

CPU()
{
	
	initialize();

	// Set up function pointer table
std::cout<<"starting function pointer table setup\n";
		table[0x0] = &CPU::Table0;
		table[0x1] = &CPU::OP_1nnn;
		table[0x2] = &CPU::OP_2nnn;
		table[0x3] = &CPU::OP_3xkk;
		table[0x4] = &CPU::OP_4xkk;
		table[0x5] = &CPU::OP_5xy0;
		table[0x6] = &CPU::OP_6xkk;
		table[0x7] = &CPU::OP_7xkk;
		table[0x8] = &CPU::Table8;
		table[0x9] = &CPU::OP_9xy0;
		table[0xA] = &CPU::OP_Annn;
		table[0xB] = &CPU::OP_Bnnn;
		table[0xC] = &CPU::OP_Cxkk;
		table[0xD] = &CPU::OP_Dxyn;
		table[0xE] = &CPU::TableE;
		table[0xF] = &CPU::TableF;

		for (size_t i = 0; i <= 0xE; i++)
		{
			table0[i] = &CPU::OP_NULL;
			table8[i] = &CPU::OP_NULL;
			tableE[i] = &CPU::OP_NULL;
		}

		table0[0x0] = &CPU::OP_00E0;
		table0[0xE] = &CPU::OP_00EE;

		table8[0x0] = &CPU::OP_8xy0;
		table8[0x1] = &CPU::OP_8xy1;
		table8[0x2] = &CPU::OP_8xy2;
		table8[0x3] = &CPU::OP_8xy3;
		table8[0x4] = &CPU::OP_8xy4;
		table8[0x5] = &CPU::OP_8xy5;
		table8[0x6] = &CPU::OP_8xy6;
		table8[0x7] = &CPU::OP_8xy7;
		table8[0xE] = &CPU::OP_8xyE;

		tableE[0x1] = &CPU::OP_ExA1;
		tableE[0xE] = &CPU::OP_Ex9E;

		for (size_t i = 0; i <= 0x65; i++)
		{
			tableF[i] = &CPU::OP_NULL;
		}

		tableF[0x07] = &CPU::OP_Fx07;
		tableF[0x0A] = &CPU::OP_Fx0A;
		tableF[0x15] = &CPU::OP_Fx15;
		tableF[0x18] = &CPU::OP_Fx18;
		tableF[0x1E] = &CPU::OP_Fx1E;
		tableF[0x29] = &CPU::OP_Fx29;
		tableF[0x33] = &CPU::OP_Fx33;
		tableF[0x55] = &CPU::OP_Fx55;
		tableF[0x65] = &CPU::OP_Fx65;
		cout<<"FP setup done!!\n";
}

typedef void (CPU::*CPUFunc)();
CPUFunc table[0xF + 1];
CPUFunc table0[0xE + 1];
CPUFunc table8[0xE + 1];
CPUFunc tableE[0xE + 1];
CPUFunc tableF[0x65 + 1];

void Table0()
{
	((*this).*(table0[opcode & 0x000Fu]))();
}

void Table8()
{
	((*this).*(table8[opcode & 0x000Fu]))();
}

void TableE()
{
	((*this).*(tableE[opcode & 0x000Fu]))();
}

void TableF()
{
	((*this).*(tableF[opcode & 0x00FFu]))();
}

void OP_NULL()
{}

//16 8 bit registers V0-VF
uint8_t registers[16];


uint16_t IR;//16 bit index register

uint8_t SP; //8 bit Stack pointer
uint16_t PC;//16 bit program counter

uint8_t delay_timer;//8 bit delay timer
uint8_t sound_timer; // 8 bit sound timer

uint32_t video[64 * 32];
uint16_t stack[16];

uint8_t memory[MAXMEM];

uint16_t opcode;

uint8_t keypad[16];

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

//Random byte implementation
std::mt19937 rng;  // the Mersenne Twister with a popular choice of parameters
uint32_t seed_val;           // populate somehow
std::uniform_int_distribution<uint8_t> uint_dist;

void initialize()
{
  rng.seed(seed_val);
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

		// Load the ROM contents into the& memory, starting at 0x200
		for (long i = 0; i < size; ++i)
		{
			memory[mem_limit_lower + i] = buffer[i];
			printf("%0x ",memory[mem_limit_lower + i]);
		}

		// Free the buffer
		//cout<<"Freed buffer successfully-Load rom phase\n";
		delete[] buffer;
	}
}


void OP_00E0() //Cls clear screen
{
	for(int i=0;i<32*64;i++)
	{
	 video[i]=0;
	}
}

void OP_00EE()//Return from sub-routine
{
	SP--;
	PC = stack[SP];
}

void OP_1nnn() //Jump to location nnn
{
    uint16_t val = opcode;
    val=val&0x0FFFu;
    PC=val;

}

void OP_2nnn() //Call subroutine at nnn
{

	uint16_t address =  opcode& 0x0FFFu;
	stack[SP] = PC;
    SP++;
	PC= address;
}

void OP_3xkk() //Skip next instruction if Vx=kk
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	if (registers[Vx] == byte)
	{
		PC += 2;
	}
}

void OP_4xkk() //Skip next instruction if Vx!=kk
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	if (registers[Vx] != byte)
	{
		PC += 2;
	}
}

void OP_5xy0()  //Skip next instruction of Vx=Vy
{
    uint8_t Vx=(opcode&0x0F00u)>>8u;
    uint8_t Vy=(opcode&0X00F0)>>4u;

    if(registers[Vx]==registers[Vy])
    {
        PC+=2;
    }
}

void OP_6xkk()  // Set Vx=kk
{
    uint8_t Vx=(opcode&0x0F00u)>>8u;
    uint16_t kk=(opcode&0x00FFu);

    if(registers[Vx]=kk)
    {
        PC+=2;
    }
}

void OP_7xkk() //Add kk to Vx,  Vx=Vx+kk
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	registers[Vx] += byte;
}

void OP_8xy0() //Assign Vy to Vx, Vx = Vy
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[Vx] = registers[Vy];
}

void OP_8xy1() // Vx = Vx|Vy
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[Vx] |= registers[Vy];
}

void OP_8xy2() // Vx = Vx&Vy
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[Vx] &= registers[Vy];
}

void OP_8xy3() // Vx = Vx^Vy
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[Vx] ^= registers[Vy];
}

void OP_8xy4() // Vx=Vx+Vy
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	uint16_t sum = registers[Vx] + registers[Vy];

	if (sum > 255U) //VF is overfloe/carry register
	{
		registers[0xF] = 1;
	}
	else
	{
		registers[0xF] = 0;
	}

	registers[Vx] = sum & 0xFFu;
}

void OP_8xy5() //Vx=Vx-Vy
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	if (registers[Vx] > registers[Vy])
	{
		registers[0xF] = 1;
	}
	else
	{
		registers[0xF] = 0;
	}

	registers[Vx] -= registers[Vy];
}

void OP_8xy6() //Set Vx = Vx SHR 1.
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	// Save LSB in VF
	registers[0xF] = (registers[Vx] & 0x1u);

	registers[Vx] >>= 1;
}

void OP_8xy7()//Set Vx = Vy - Vx, set VF = NOT borrow.
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	if (registers[Vy] > registers[Vx])
	{
		registers[0xF] = 1;
	}
	else
	{
		registers[0xF] = 0;
	}

	registers[Vx] = registers[Vy] - registers[Vx];
}

void OP_8xyE()//Set Vx = Vx SHL 1.
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	// Save MSB in VF
	registers[0xF] = (registers[Vx] & 0x80u) >> 7u;

	registers[Vx] <<= 1;
}

void OP_9xy0() //Skip next instruction if Vx!=Vy
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	if (registers[Vx] != registers[Vy])
	{
		PC += 2;
	}
}

void OP_Annn()//Set index register to nnn
{
	uint16_t address = opcode & 0x0FFFu;

	IR = address;
}

void OP_Bnnn()//Jump to location V0+nnn
{
uint16_t offset = opcode & 0x0FFFu;

PC = registers[0] + offset;
}

void OP_Cxkk() //Set Vx = random byte AND kk.
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	uint8_t rbyte=uint_dist(rng); //!!!! Check this later for any issues
	registers[Vx] = rbyte & byte;
}

void OP_Dxyn() //Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;
	uint8_t height = opcode & 0x000Fu;

	// Wrap if going beyond screen boundaries
	uint8_t xPos = registers[Vx] % VIDEO_WIDTH;
	uint8_t yPos = registers[Vy] % VIDEO_HEIGHT;

	registers[0xF] = 0;

	for (unsigned int row = 0; row < height; ++row)
	{
		uint8_t spriteByte = memory[IR + row];

		for (unsigned int col = 0; col < 8; ++col)
		{
			uint8_t spritePixel = spriteByte & (0x80u >> col);
			uint32_t* screenPixel = &video[(yPos + row) * VIDEO_WIDTH + (xPos + col)];

			// Sprite pixel is on
			if (spritePixel)
			{
				// Screen pixel also on - collision
				if (*screenPixel == 0xFFFFFFFF)
				{
					registers[0xF] = 1;
				}

				// Effectively XOR with the sprite pixel
				*screenPixel ^= 0xFFFFFFFF;
			}
		}
	}
}

void OP_Ex9E()//Skip next instruction if key with the value of Vx is pressed.
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	uint8_t key = registers[Vx];

	if (keypad[key])
	{
		PC += 2;
	}
}

void OP_ExA1()//Skip next instruction if key with the value of Vx is not pressed.
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	uint8_t key = registers[Vx];

	if (!keypad[key])
	{
		PC += 2;
	}
}

void OP_Fx07()  //Set Vx to delay timer value
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	registers[Vx] = delay_timer;
}

void OP_Fx0A()//Wait for a key press, store the value of the key in Vx.
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

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
		PC -= 2; //Repeats the same instructions as the cycle increases by +2 so this -2 will return to the same position again and again....
	}
}

void OP_Fx15()  //Set delay timer = Vx
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	delay_timer = registers[Vx];
}

void OP_Fx18() // Set sound timer = Vx
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	sound_timer = registers[Vx];
}

void OP_Fx1E()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	IR += registers[Vx];
}

void OP_Fx29()//Set I = location of sprite for digit Vx.
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t digit = registers[Vx];

	IR = FONTSET_START_ADDRESS + (5 * digit);
}

void OP_Fx33() //for 0-255 max 3 difits
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t value = registers[Vx];

	// Units-place
	memory[IR + 2] = value % 10;
	value /= 10;

	// Tens-place
	memory[IR + 1] = value % 10;
	value /= 10;

	// Hundreds-place
	memory[IR] = value % 10;
}

void OP_Fx55() // Store V0-Vx in memory starting at I
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	for (uint8_t i = 0; i <= Vx; ++i)
	{
		memory[IR + i] = registers[i];
	}
}

void OP_Fx65() // Load data from memory in to registers V0 to Vx
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	for (uint8_t i = 0; i <= Vx; ++i)
	{
		registers[i] = memory[IR + i];
	}
}
////////////////////////////////////////////
//Cycle
///////////////////////////////////////////
void cycle() //Working
{
    //Fetch
	printf("Decode-execute-cycle - opcode - %d %0x \n",PC,opcode);
    // Fetch
	opcode = (memory[PC] << 8u) | memory[PC + 1];

    // Increment the PC before we execute anything
	PC += 2;
    //Decode and execute
	
	((*this).*(table[(opcode & 0xF000u) >> 12u]))();
	

    // Decrement the delay timer if it's been set
	if (delay_timer > 0)
	{
		delay_timer--;
	}

	// Decrement the sound timer if it's been set
	if (sound_timer > 0)
	{
		sound_timer--;
	}

}

	

};

class Platform
{
public:
    Platform(const char* title, int windowWidth, int windowHeight, int textureWidth, int textureHeight)
    {
        window.create(sf::VideoMode(windowWidth, windowHeight), title);
		window.clear(sf::Color::Green);
        texture.create(textureWidth, textureHeight);
        sprite.setTexture(texture);
    }

    ~Platform()
    {
        // No need to explicitly destroy SFML objects; they are managed automatically.
    }

    void Update(const void* buffer, int pitch)
    {
        texture.update(static_cast<const sf::Uint8*>(buffer),VIDEO_WIDTH,VIDEO_HEIGHT, texture.getSize().x, texture.getSize().y);
        window.clear();
        window.draw(sprite);
        window.display();
    }

    bool ProcessInput(uint8_t* keys)
    {
        sf::Event event;

        while (window.pollEvent(event))
        {
            switch (event.type)
            {
                case sf::Event::Closed:
                    return true;

                case sf::Event::KeyPressed:
                    HandleKeyPress(event.key.code, keys);
                    break;

                case sf::Event::KeyReleased:
                    HandleKeyRelease(event.key.code, keys);
                    break;

                default:
                    break;
            }
        }

        return false;
    }

private:
    sf::RenderWindow window;
    sf::Texture texture;
    sf::Sprite sprite;

    void HandleKeyPress(sf::Keyboard::Key key, uint8_t* keys)
    {
        switch (key)
        {
            case sf::Keyboard::Escape:
                keys[0] = 1;
                break;
            case sf::Keyboard::X:
                keys[1] = 1;
                break;
            case sf::Keyboard::Num1:
                keys[2] = 1;
                break;
            case sf::Keyboard::Num2:
                keys[3] = 1;
                break;
            case sf::Keyboard::Q:
                keys[4] = 1;
                break;
            case sf::Keyboard::W:
                keys[5] = 1;
                break;
            case sf::Keyboard::E:
                keys[6] = 1;
                break;
            case sf::Keyboard::A:
                keys[7] = 1;
                break;
            case sf::Keyboard::S:
                keys[8] = 1;
                break;
            case sf::Keyboard::D:
                keys[9] = 1;
                break;
            case sf::Keyboard::Z:
                keys[0xA] = 1;
                break;
            case sf::Keyboard::C:
                keys[0xB] = 1;
                break;
            case sf::Keyboard::Num4:
                keys[0xC] = 1;
                break;
            case sf::Keyboard::R:
                keys[0xD] = 1;
                break;
            case sf::Keyboard::F:
                keys[0xE] = 1;
                break;
            case sf::Keyboard::V:
                keys[0xF] = 1;
                break;
        }
    }

    void HandleKeyRelease(sf::Keyboard::Key key, uint8_t* keys)
    {
        switch (key)
        {
            case sf::Keyboard::Escape:
                keys[0] = 0;
                break;
            case sf::Keyboard::X:
                keys[1] = 0;
                break;
            case sf::Keyboard::Num1:
                keys[2] = 0;
                break;
            case sf::Keyboard::Num2:
                keys[3] = 0;
                break;
            case sf::Keyboard::Q:
                keys[4] = 0;
                break;
            case sf::Keyboard::W:
                keys[5] = 0;
                break;
            case sf::Keyboard::E:
                keys[6] = 0;
                break;
            case sf::Keyboard::A:
                keys[7] = 0;
                break;
            case sf::Keyboard::S:
                keys[8] = 0;
                break;
            case sf::Keyboard::D:
                keys[9] = 0;
                break;
            case sf::Keyboard::Z:
                keys[0xA] = 0;
                break;
            case sf::Keyboard::C:
                keys[0xB] = 0;
                break;
            case sf::Keyboard::Num4:
                keys[0xC] = 0;
                break;
            case sf::Keyboard::R:
                keys[0xD] = 0;
                break;
            case sf::Keyboard::F:
                keys[0xE] = 0;
                break;
            case sf::Keyboard::V:
                keys[0xF] = 0;
                break;
        }
    }
};


int main(int argc, char* argv[])
{
    if (argc != 4)
    {
        std::cerr << "Usage: " << argv[0] << " <Scale> <Delay> <ROM>\n";
        std::exit(EXIT_FAILURE);
    }

    int videoScale = std::stoi(argv[1]);
    int cycleDelay = std::stoi(argv[2]);
    const char* romFilename = argv[3];

    Platform platform("CHIP-8 Emulator", VIDEO_WIDTH * videoScale, VIDEO_HEIGHT * videoScale, VIDEO_WIDTH, VIDEO_HEIGHT);

    CPU chip8;
    chip8.LoadROM(romFilename);

    int videoPitch = sizeof(chip8.video[0]) * VIDEO_WIDTH;

    auto lastCycleTime = std::chrono::high_resolution_clock::now();
    bool quit = false;

	printf("\n%0x\n",chip8.memory[mem_limit_lower+1]);//Remove later

    while (!quit)
    {
        quit = platform.ProcessInput(chip8.keypad);

        auto currentTime = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - lastCycleTime).count();

        if (dt > cycleDelay)
        {
            lastCycleTime = currentTime;

            chip8.cycle();
			// cout<<"Cycle running\n";
            platform.Update(chip8.video, videoPitch);
        }
    }

    return 0;
}