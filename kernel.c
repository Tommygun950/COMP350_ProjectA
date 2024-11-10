//ProjectB Functions
void printString(char*);
void printChar(char*);
char* readString(char*);
char* readSector(char*, int);
void handleInterrupt21(int ax, char* bx, int cx, int dx);
//ProjectC Functions
int directoryExists(char* directory_sector, int* file_entry, char* possible_filename);
void readFile(char* filename, char* output_buffer, int* sectorsRead);
void executeProgram(char* name);
void terminate();
//////////////////////////////////////////////////////////////////////////////////////////
//main function
void main(){

/* readFile TEST SCRIPT!!!!	
	char buffer[13312];
	int sectorsRead;
	makeInterrupt21(); 
	interrupt(0x21, 3, "messag", buffer, &sectorsRead);   
	if (sectorsRead>0)
		interrupt(0x21, 0, buffer, 0, 0);   
	else
		interrupt(0x21, 0, "messag not found\r\n", 0, 0);  
	while(1);   
*/	

/* executeProgram TEST SCRIPT!!!!
	makeInterrupt21();
	interrupt(0x21, 4, "tstpr1", 0, 0);
	while(1); 
*/

	makeInterrupt21();
	interrupt(0x21, 4, "shell", 0,0); //loads and executes shell.
}
//////////////////////////////////////////////////////////////////////////////////////////
//Prints string to terminal.
void printString(char* chars){
	int i=0;
	while (chars[i] != 0x0){ //0x0 signifies the last character in the string.
		char al = chars[i]; //AL register is the place holder for each character.
		char ah = 0xe; //AH register is always 0xe.
		int ax = ah*256 + al; //AX is always equal to AH*256+AL.
		interrupt(0x10,ax,0,0,0);//0,0,0 are registers BX,CX,DX but are not used.
		++i;
	}
}

//Prints a single character to the terminal.
void printChar(char* c){	
	char al = c;
	char ah = 0xe;
	int ax = ah*256 + al;
	interrupt(0x10,ax,0,0,0);
}

//Takes input fro mthe screen and sends it back out (like echo command).
char* readString(char* array_input)
{
	char keyboard_input = 0x0;

	int i = 0;
	while (i < 80) { //The character array is limited to 80 elements. 
		//0x16 is how BIOS reads from interrupt.
		keyboard_input = (char) interrupt(0x16,0,0,0,0); 

		//0xd is enter and notifies the system that's the end of the string.
		if (keyboard_input == 0xd)
			break;
		//Makes sure if backspace is pressed, it won't store it.
		if (keyboard_input == 0x8 && i == 0)
			continue;
		//0x8 is the backspace key, and will remove the last element.
		if (keyboard_input == 0x8){
			char space = ' ';
			interrupt(0x10, 0xe * 256 + keyboard_input,0,0,0);
			interrupt(0x10, 0xe * 256 + space,0,0,0);
			interrupt(0x10, 0xe * 256 + keyboard_input,0,0,0);
			//the 2nd backspace removes it from the screen.
			--i;
			continue;
		}

		array_input[i] = keyboard_input;
		interrupt(0x10, 0xe * 256 + keyboard_input,0,0,0);
		++i;
	}
	array_input[i] = 0xa; //0xa is for a line feed.
	array_input[i + 1] = 0x0; //0x0 signifies the end of the string.

	interrupt(0x10, 0xe * 256 + 0xd,0,0,0); //Outputs each entire element from the array.
	interrupt(0x10, 0xe * 256 + 0xa,0,0,0); //Outputs line feed character.

	return array_input;
}

//Reads from a file and outputs it to the screen.
char* readSector(char* buffer, int sector){
	int ah = 2; //Tells BIOS to read a sector rather than write.
	int al = 1; //Leave as 1, its the number of sectors to read.
	int ax = ah * 256 + al;

	char* bx = buffer; //address where the data should be stored to.
	
	int ch = 0; //track number.
	int cl = sector + 1; //relative sector number.
	int cx = ch * 256 + cl; 

	int dh = 0; //head number.
	int dx = dh * 256 + 0x80;

	interrupt(0x13, ax, bx, cx, dx);
	
	return buffer;
}

//Our own defined interrupt that can be called within kernel.c.
void handleInterrupt21(int ax, char* bx, int cx, int dx){
	switch(ax){ //AX is the # that determines which function to run.
		case 0: printString(bx); break; //if ax is 0, it'll printString.
		case 1: readString(bx); break; //if ax is 1, it'll readString.
		case 2: readSector(bx, cx); break; //if ax is 2, it'll read sector.
		case 3: readFile(bx, cx, dx); break; //if ax is 3, it'll readFile.
		case 4: executeProgram(bx); break; //if ax is 4, it'll executeProgram.
		case 5: terminate(); break; //if ax is 5, it'll terminate.
		default: printString("ERROR: AX is invalid."); break; //if ax isn't anything above, it'll print and error.
	}
}

//Will return either 1 or 0 (true or false) if a file exists in the directory.
int directoryExists(char* directory_sector, int* file_entry, char* possible_filename){
	int i = 0; //counter
	int letters = 0; //counter for correct number of letters in the loop. 
	
	//Steps through the directory in increments of 32.
	for(*file_entry=0; *file_entry<512; *file_entry+=32){
		//Compares the first 6 letters in the filename.
		while(i<6){
			//If the letter isn't the same.
			if(directory_sector[*file_entry + i] != possible_filename[i]){
				break;
			}
			//If the letter is the same.
			else{
				++letters;
			}
			++i;
		}
		//If all 6 letters are the same, return true.
		if(letters == 6){
			return 1;
		}
	}
	//If all 6 letters don't match, return false.
	return 0;
}

//Will take a character array containing a file name and reads the file into a buffer.
void  readFile(char* filename, char* output_buffer, int* sectorsRead){
	int i = 0; //counter
	char directory_sector[512]; //load directory sector into 512 byte character array.

	int file_entry = 0;
	int* param = &file_entry;
	*sectorsRead = 0;
	readSector(directory_sector, 2);

	//If the filename exists.
	if(directoryExists(directory_sector, param, filename) == 1){ 
		while(directory_sector[*param + i] != 0){
			readSector(output_buffer, directory_sector[*param + 6 + i]);
			output_buffer += 512;
			++*sectorsRead;
			++i;
		}
	}
	//If the filename doesn't exist.
	else{
		*sectorsRead = 0;
	}
}

//Will take the name of a program, use readFile to locate it, and it'll execute it.
void executeProgram(char* program){
	char buffer[13312]; //13312 is the maximum size of a file.
	int sectorsRead;
	int i = 0;

	//Call readFile to load the file into a buffer.
	readFile(program, buffer, &sectorsRead);

	for( i=0; i<sectorsRead*512; i++){
		//Transfter file from buffer into memory at segment 0x2000.
		putInMemory(0x2000, i, buffer[i]);
	}
	launchProgram(0x2000); //This is a new routine found in kernel.asm.
}

//Will make an interupt 0x21 call to reload and execute shell.
void terminate(){
	char shellname[6];

	//copy letters in one at a time.
	shellname[0]='s';
	shellname[1]='h';
	shellname[2] = 'e';
	shellname[3] = 'l';
	shellname[4] = 'l';
	shellname[5] = '\0';

	executeProgram(shellname);
}

	







