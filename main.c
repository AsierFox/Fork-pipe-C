#include <string.h>
#include <fcntl.h>
#include <signal.h>

//
// Global variables
#define TRUE  1

#define INPUT  0
#define OUTPUT 1

#define RES_BUFFER_MAX 25

// Global pipes
int screen_pipe[2];
int file_pipe[2];
// Global buffer

char user_res[ RES_BUFFER_MAX ];
// PID of the child processes
pid_t childScreen, childFile;

//
// Function prototypes
inline void mainProcess();
inline void childProcessScreen();
inline void childProcessFile();
size_t print_msg(char *msg);

//
// Main method
int main(int argc, char *argv[]) {

	// Open the pipes
	if ( pipe(screen_pipe) || pipe(file_pipe) ) {
		perror("Error creating pipes");
		return -1;
	}

	// Start child process for screen
	if ( ( childScreen = fork() ) == 0 ) {
		childProcessScreen();
	}
	// Start child process for file
	if ( ( childFile = fork() ) == 0 ) {
		childProcessFile();
	}
	else {
		// Start parent process
		mainProcess();
	}

	return 0;
}

//
// Functions

/**
 * This functions is going to INPUT the input of the user until
 * he types 'exit'
 */
inline void mainProcess() {
	close(2);
	close(screen_pipe[INPUT]);
	close(file_pipe[INPUT]);

	char req_msg[] = "{{ Proceso padre }} Escriba alguna entrada ('exit' para salir): ";
	
	print_msg(req_msg);
	read(0, user_res, RES_BUFFER_MAX);
	while ( strncmp(user_res, "exit", strlen("exit") ) != 0 ) {

		write(screen_pipe[OUTPUT], user_res, strlen(user_res));
		write(file_pipe[OUTPUT], user_res, strlen(user_res));

		print_msg(req_msg);
		read(0, user_res, RES_BUFFER_MAX);
	}

	// Ask to finish to the two child processes
	kill(childScreen, SIGTERM);
	kill(childFile, SIGTERM);
}

/**
 * This functions is going to output the user input received from
 * the parent process to screen
 */
inline void childProcessScreen() {
	close(0);
	close(screen_pipe[OUTPUT]);
	dup2(screen_pipe[OUTPUT], 1);

	char msg[] = "\n{{ Proceso hijo #1 }} ";
	int msg_size = strlen(msg);

	while ( TRUE ) {
		read(screen_pipe[INPUT], user_res, RES_BUFFER_MAX);
		write(1, msg, msg_size); // TODO Reusar print_msg
		write(1, user_res, strlen(user_res));// TODO Reusar print_msg
	}
}

/**
 * This functions is going to output the user input received from
 * the parent process to a file
 */
inline void childProcessFile() {
	close(0);
	close(file_pipe[OUTPUT]);
	dup2(file_pipe[OUTPUT], 1);

	char msg[] = "\n{{ Proceso hijo #2 }} Mensaje escrito en fichero";
	int msg_size = strlen(msg);

	int file_desc = open("./test.txt", O_RDWR | O_CREAT | O_TRUNC , 0644);

	if ( file_desc != -1 ) {

		while ( TRUE ) {
			read(file_pipe[INPUT], user_res, RES_BUFFER_MAX);
			write(file_desc, user_res, strlen(user_res)); // TODO Reusar print_msg
			write(1, msg, msg_size); // TODO Reusar print_msg
		}

        close(file_desc);
	}
	else {
		perror("Error opening the file");
	}
}

/**
 * Prints the message passed by parametter is the standard output
 */
size_t print_msg(char *msg) {
	return write(1, msg, strlen(msg));
}
