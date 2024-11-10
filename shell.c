//PROJECTC BASIC FUNCTIONS
void type(char* input_filename);
void exec(char* input_filename);
//PROJECTC FUNCTIONS FOR VALIDATING INPUT.
void find_cmd(char input[], char* output);
void find_filename(char input[], char* output);
int fileExists(char input, char* exists);

int main(){
	while(1){
		char userInput[30]; //max user input, 30 characters.
		char cmd[15]; //max command length, 15 characters.
		char filename[15]; //max filename length, 15 characters.

		//Outputs SHELL header to terminal and calls for user input.
		syscall(0, "\rSHELL> ");
		syscall(1, userInput);
		
		//parses command and filename from user input.
		find_cmd(userInput, cmd);
		find_filename(userInput, filename);
		
		//If the command entered is <type>, use the type command.
		if(fileExists(cmd, "type")){
			type(filename);
		}
		//If the command entered is <exec>, user the exec command.
		else if(fileExists(cmd, "exec")){
			exec(filename);
		}
		//If they typed an invalid or non-existing command.
		else{
			syscall(0, "Bad command!\n\r");
		}
	}
}

//If user types <type filename>, the shell should print out file.
void type(char* input_filename){
	char buffer[13312]; //allows for max-sized file.
	int sectorsRead;
	syscall(3, input_filename, buffer, &sectorsRead); //3 is readFile.

	if(sectorsRead > 0){
		syscall(0, buffer); //0 is printString.
	}
	else{
		syscall(0, "File not found.\r\n"); //0 is printString.
	}
}

//IF user types <exec filename>, the shell will execute file.
void exec(char* input_filename){
	char buffer[13312]; //max file-size.
	int sectorsRead;
	syscall(3, input_filename, buffer, &sectorsRead); //3 is readFile.

	if(sectorsRead > 0){
		syscall(4, input_filename); //4 is executeProgram.
	}
	else{
		syscall(0, "File not found.\r\n"); //0 is printString.
	}
}

//Will parse the command out of the shell input.
void find_cmd(char input[], char* output){
	int i=0; //counter
	
	while(input[i] != ' '){
		output[i] = input[i];
		i++;
	}

	input[i] = "\0";
}

//Will parse the filename out of the shell input.
void find_filename(char input[], char* output){
	int i=0; //counter
	
	while(i<6){ 
		output[i] = input[i+5];
		i++;
	}
}

//Will return 0 or 1 (true or false) if the filename in the command input exists or not.
int fileExists(char input[], char* exists){
	int i =0;
	//While the input string and the existing files doesn't reach it's end.
	while(input[i] != '\0' && exists[i] != '\0'){ 
		if(input[i] != exists[i]){
			return 0; //False if the input string doesn't exist as a filename.
		}
		i++;
	}
	return 1; //If all of the characters are found in an existing file.
}

