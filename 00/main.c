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
int ACCESS = 0;
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
			ACCESS += 3;
			return 0;
		case LDA:	
			index_value = mem.data[current_index + 2];
			AC += AC + mem.data[index_value * 2];
			ACCESS += 3;
			return 0;
		case ADD:
			index_value = mem.data[current_index + 2];
			AC += mem.data[index_value * 2];
			ACCESS += 3;
			return 0;
		case OR:
			index_value = mem.data[current_index + 2];
			AC = AC | mem.data[index_value * 2];
			ACCESS += 3;
			return 0;
		case AND:
			index_value = mem.data[current_index + 2];
			AC = AC & mem.data[index_value * 2];
			ACCESS += 3;
			return 0;
		case NOT:
			AC = ~AC;
			ACCESS += 1;
			return 0;
		case JMP:
			index_value = mem.data[current_index + 2];
			PC = index_value - 1;
		       	ACCESS += 2;	
			return 0;
		case JN:
			index_value = mem.data[current_index + 2];
			PC = index_value - 1;
			ACCESS += 2;
		case JZ:
			index_value = mem.data[current_index + 2];
			PC = index_value - 1;
			ACCESS += 2;
		case HLT:
			ACCESS += 1;
			return 1;
		default:
			return 0;
	}
}

FILE *getFile(char *path){
	FILE *file;
	
	file = fopen(path, "rb");

	return file;
}


int main(int argc, char *argv[]) {
	FILE *file = getFile(argv[1]);
	
	if (file == NULL){
		printf("Erro ao abrir o arquivo.\n");
		return 1;
	}


	fread(&mem, sizeof(mem), 1, file);
	
	/*
	for(int i=0; i < sizeof(mem.header); i++){
		printf("%u ", mem.header[i]);
	}
	*/
	printf("\n");	
	
	int interpreter_return;
	for(PC=0; PC < sizeof(mem.data); PC++){
		interpreter_return = interpreter(mem.data[PC], PC);
		if(interpreter_return == 1){
			printf("END: HLT\n");
			break;
		}
		validateResult(AC);
	}
	PC = PC/2 + 1;

	fclose(file);
	
	printf("AC: %d | PC: %d\n", AC, PC);
	printf("N: %b | Z: %b\n", N, Z);
	printf("ACCESS: %d", ACCESS);
	return 0;
}
