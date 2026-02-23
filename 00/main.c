#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

enum MNEMO {
	NOP = 0,
	STA = 16,
	LDA = 32,	
	ADD = 48,
	OR = 64,
	AND = 80,
	NOT = 96,
	JMP = 128,
	JN = 144,
	JZ = 160,
	HLT = 240
};

struct MEM {
	unsigned char header[4];
	unsigned char data[516];
};

int AC, PC;
bool N, Z;
struct MEM mem;

bool isNegative(int value){
	if(value < 0){
		return true;
	}
	return false;
}

bool isZero(int value){
	if(value == 0){
		return true;
	}
	return false;
}

void validateResult(int result){
	N = isNegative(result);
	Z = isZero(result);
}

int interpreter(unsigned char byte, int current_index){
	enum MNEMO mnemo;
	int index_value;
	int result;
	switch(byte){
		case NOP:
			return 0;
		case STA:
			index_value = mem.data[current_index + 2];
			mem.data[index_value * 2] = AC;
			return 0;
		case LDA:	
			index_value = mem.data[current_index + 2];
			AC += AC + mem.data[index_value * 2];
			return 0;
		case ADD:
			index_value = mem.data[current_index + 2];
			AC += mem.data[index_value * 2];
			return 0;
		case OR:
			index_value = mem.data[current_index + 2];
			AC = AC | mem.data[index_value * 2];
			return 0;
		case AND:
			index_value = mem.data[current_index + 2];
			AC = AC & mem.data[index_value * 2];
			return 0;
		case NOT:
			AC = ~AC;
			return 0;
		case JMP:
			index_value = mem.data[current_index + 2];
			PC = index_value - 1; 
			return 0;
		case HLT:
			return 1;
		default:
			return 0;
	}
}

int main() {
	FILE *file;
	file = fopen("neader_code.mem", "rb");

	if (file == NULL){
		printf("Erro ao abrir o arquivo.\n");
		return 1;
	}
	
	fseek(file, 0L, SEEK_END);
	
	long file_size = ftell(file);

	rewind(file);

	printf("file size: %d\n", file_size);
	

	fread(&mem, sizeof(mem), 1, file);
	
	for(int i=0; i < sizeof(mem.header); i++){
		printf("%u ", mem.header[i]);
	}
	
	printf("\n");	
	

	int interpreter_return;
	for(PC=0; PC < sizeof(mem.data); PC++){
		interpreter_return = interpreter(mem.data[PC], PC);
		if(interpreter_return == 1){
			printf("HLT\n");
			break;
		}
		validateResult(AC);
	}
	PC = PC/2 + 1;

	fclose(file);
	
	printf("AC: %d | PC: %d\n", AC, PC);
	printf("N: %b | Z: %b", N, Z);
	return 0;
}
