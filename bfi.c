#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define ESPACO_DISP 16
#define TAM_MAX 1024

char Memoria[ESPACO_DISP];
char Output[TAM_MAX];
int ptr=0;
int verbose=0;
float sleep_time=0;

typedef struct{
	int fptr;
	int line;
	int column;
} stack;

void error(const char* error_msg){
	fprintf(stderr, "\033[0;31m");
	fprintf(stderr, "error: %s.\n", error_msg);
	fprintf(stderr, "\033[0m");
	exit(1);
}

int comperror(const char* error_msg, int line, int column){
	fprintf(stderr, "\033[0;31m");
	fprintf(stderr, "error in line %d, column %d:\n%s.\n", line, column, error_msg);
	fprintf(stderr, "\033[0m");
	return 1;
}

void zera_Memoria (char Memo[ESPACO_DISP]){
	int i;
	for(i=0; i<ESPACO_DISP; i++) Memo[i]=0;
}

void showmemory(){
	int i;
	printf("( ");
	for(i=0; i<ESPACO_DISP; i++){
		printf(" %s%i%s ", (i==ptr)?"\033[44m ":"\033[90m ", Memoria[i], " \033[m");
		//printf(" %c%i%c ", (i==ptr)?'[':' ', Memoria[i], (i==ptr)?']':' ');
	}
	printf(" )\n\n");
}

void showdetails(char Command, int line, int column){
	static int counter=0;
	if(Command < 33) return;
	//puts("\e[1;1H\e[2J");
	printf("\033[7m        Details in line %3d, column %3d        \033[m\n\n", line, column);
	printf("Command %d: ", ++counter);
	switch(Command){
		case '>':
		case '<': printf("\033[45m "); break;
		case '.': printf("\033[46m "); break;
		case ',': printf("\033[43m "); break;
		case '+': printf("\033[42m "); break;
		case '-': printf("\033[41m "); break;
		case '[':
		case ']': printf("\033[44m "); break;
		default: printf("\033[30m"); break;
	}
	printf("%c \033[0;m\n\n", Command);
	showmemory();
	printf("Output: \033[36m%s\033[0m\n\n", Output);
	sleep(sleep_time);
}

void display_help(){
	printf("Usage: bfi [options] file...\n\nOptions:\n  -h           Display this help\n  -v [float]   Verbose: executes one command at time\n\n");
	return;
}

int compila_codigo(FILE *fp){
	stack Stack[TAM_MAX/2];
	char Comando;
	int line=1, column=0;
	int i=0, j=0, k, repetidor=1;
	
	while(!feof(fp)){
		column++;
		Comando = fgetc(fp);
		
		switch(Comando){
			case '>':
				if((ptr+1)==ESPACO_DISP) return comperror("memory superior limit exceded", line, column);
				ptr++;
				break;
			
			case '<':
				if((ptr-1)<0) return comperror("memory inferior limit exceded", line, column);
				ptr--;
				break;
			
			case '+':
				for(k=repetidor; k>0; k--) Memoria[ptr]++;
				break;
				
			case '-':
				for(k=repetidor; k>0; k--) Memoria[ptr]--;
				break;
			
			case '.':
				if(!verbose) printf("%c", Memoria[ptr]);
				Output[i++] = Memoria[ptr];
				Output[i] = '\0';
				break;

			case ',': Memoria[ptr] = getc(stdin); break;
			
			case '[':
				j++;
				Stack[j].fptr = ftell(fp);
				Stack[j].line = line;
				Stack[j].column = column;
				break;
			case ']':
				if(!j) return comperror("missing '['", line, column);
				if(Memoria[ptr]){
					fseek(fp, Stack[j].fptr, SEEK_SET);
					line = Stack[j].line;
					column = Stack[j].column;
					break;
				}
				j--;
				break;
			
			case '\n': line++; column=0; break;
			
			default: break;
		}
		repetidor=1;
		if(Comando>='1' && Comando<='9') repetidor=Comando-48;
		if(verbose) showdetails(Comando, line, column); //mostra_Memoria(Memoria, Comando, j);
	}
	
	if(j) return comperror("unclosed ']'", line, column);
	
	return 0;
}

int main(int argc, char* argv[]){
	char Codigo[TAM_MAX], options[] = "hv:", *filename;
	char opt, *str;
	int r=1, i;
	FILE *fp;
	
	if(argc < 2) error("no input file (type 'bfi -h' for help)");
	
	while((opt = getopt(argc, argv, options)) != EOF){
		switch(opt){
			case 'h': display_help(); exit(0);
			case 'v':
				str = optarg;
				verbose = 1;
				if(str[0]!='0' && !(sleep_time=atof(str))) error("parameter of command line option '-v' is not a float");
				break;
			default:
				fprintf(stderr, "\033[0;33m");
				fprintf(stderr, "warning: unrecognized command line option '-%c'.\n", opt);
				fprintf(stderr, "\033[0m");
		}
	}

	argc -= optind;
	argv += optind;
	
	fp = fopen(argv[0], "r");
	if(!fp){
		fprintf(stderr, "\033[0;31m");
		fprintf(stderr, "error: can't open file '%s'\n", argv[0]);
		fprintf(stderr, "\033[0m");
		exit(2);
	}
	
	zera_Memoria(Memoria);
	compila_codigo(fp);
	
	fclose(fp);
	printf("\n");
	return 0;
}
