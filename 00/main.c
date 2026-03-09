#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

typedef enum {
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
} MNEMO;

const char *get_key_mnemo(MNEMO mnemo) {
    switch (mnemo) {
        case NOP: return "NOP";
        case STA: return "STA";
        case LDA: return "LDA";
		case ADD: return "ADD";
		case OR: return "OR";
		case AND: return "AND";
		case NOT: return "NOT";
		case JMP: return "JMP";
		case JN: return "JN";
		case JZ: return "JZ";
		case HLT: return "HLT";
        default:  return "-";
    }
}

struct MEM {
	unsigned char header[4];
	unsigned char data[512];
};

int PC, AC, ACCESS = 0, INSTRUCTIONS = 0;
bool N, Z;
struct MEM mem;

bool is_negative(int value){
	return ((signed char)value) < 0;
}

bool is_zero(int value){
	return ((unsigned char)value) == 0;
}

void validate_result(int result){
	N = is_negative(result);
	Z = is_zero(result);
}

void new_PC(int index_end){
	if(index_end <= 1){
		PC = index_end;
		return;
	}
	
	if(index_end > PC / 2){
		PC = index_end * 2 - 1;
	}else if(index_end < PC / 2){
		PC = index_end / 2 - 1;
	}
}

int interpreter(unsigned char byte, int current_index){
	MNEMO mnemo;
	int index_value;
	switch(byte){
		case NOP:
			return 0;
		case STA:
			mem.data[mem.data[current_index + 2] * 2] = AC;
			ACCESS += 3;
			INSTRUCTIONS ++;
			return 0;
		case LDA:
			AC = mem.data[mem.data[current_index + 2] * 2];
			ACCESS += 3;
			INSTRUCTIONS ++;
			return 0;
		case ADD:
			AC = (unsigned char)(AC + mem.data[mem.data[current_index + 2] * 2]);
			ACCESS += 3;
			INSTRUCTIONS ++;
			return 0;
		case OR:
			AC = (unsigned char)(AC | mem.data[mem.data[current_index + 2] * 2]);
			ACCESS += 3;
			INSTRUCTIONS ++;
			return 0;
		case AND:
			AC = (unsigned char)(AC & mem.data[mem.data[current_index + 2] * 2]);
			ACCESS += 3;
			INSTRUCTIONS ++;
			return 0;
		case NOT:
			AC = (unsigned char)~(AC);
			ACCESS += 1;
			INSTRUCTIONS ++;
			return 0;
		case JMP:
			index_value = mem.data[current_index + 2];
			new_PC(index_value);
		       	ACCESS += 2;
			INSTRUCTIONS ++;
			return 0;
		case JN:
			index_value = mem.data[current_index + 2];
			if(N == 1){
				new_PC(index_value);		
			}	
			ACCESS += 2;
			INSTRUCTIONS ++;
			return 0;
		case JZ:
			index_value = mem.data[current_index + 2];
			if(Z == 1){
				new_PC(index_value);		
			}
			ACCESS += 2;
			INSTRUCTIONS ++;
			return 0;
		case HLT:
			ACCESS += 1;
			INSTRUCTIONS ++;
			return 1;
		default:
			return 0;
	}
}

FILE *get_file(char *path){
	FILE *file;
	
	file = fopen(path, "rb");

	return file;
}

char *define_format(char *command){
	char *format;
	if (strcmp(command, "-hex") == 0 || strcmp(command, "--h") == 0) {
		format = "%#x";
	} else if (strcmp(command, "-dec") == 0 || strcmp(command, "--d") == 0) {
		format = "%d";
	}

	return format;
}

void print_state(const char *spec) {
    char str[64];

    snprintf(str, sizeof(str), "AC: %s | PC: %s\n", spec, spec);
    printf(str, AC, PC);

    snprintf(str, sizeof(str), "N: %s | Z: %s\n", spec, spec);
    printf(str, N, Z);

    snprintf(str, sizeof(str), "ACCESS: %s | INSTRUCTIONS: %s\n", spec, spec);
    printf(str, ACCESS, INSTRUCTIONS);
}

void print_memorymap(const char *spec){
	char str[16];
	printf("END | DATA | MNEMO\n");
	int end = 0;
	for(int i=0; i < sizeof(mem.data); i++){
		if(mem.data[i] == 0){
			continue;
		}
		
		snprintf(str, sizeof(str), "%s | %s | %s\n", spec, spec, "%s");
		printf(str, i/2, mem.data[i], get_key_mnemo(mem.data[i]));
	}
}

int main(int argc, char *argv[]) {
	if(argc < 2){
		printf("Needs file path!!");
		return  1;
	}
	
	char *format = "%d";
	if(argc > 2){
		format = define_format(argv[2]);
	}

	FILE *file = get_file(argv[1]);

	if (file == NULL){
		printf("Error to open file.\n");
		return 1;
	}

	fread(&mem, sizeof(mem), 1, file);
	
	if(ferror(file)){
		printf("Error to read file");
		return 1;
	}
	
	printf("\n");
	print_memorymap(format);
	printf("\n");	
	
	int interpreter_return;
	for(PC=0; PC < sizeof(mem.data); PC++){
		interpreter_return = interpreter(mem.data[PC], PC);
		if(interpreter_return == 1){
			printf("END: HLT\n");
			break;
		}
		validate_result(AC);
	}
	PC = PC/2 + 1;

	fclose(file);
	
	print_state(format);	

	printf("\n");
	print_memorymap(format);
	printf("\n");	

	return 0;
}
