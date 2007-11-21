#include"io.h"
#include"socket.h"

#define INPUT_SIZE 32
#define OUTPUT_SIZE 128
char InputBuff[INPUT_SIZE];
char OutputBuff[OUTPUT_SIZE];

struct PIPE InputPipe;//Pipe We read from.
struct PIPE OutputPipe;//Pipe we write to.

struct SOCKET IoSocket;

void IoInit()
{
	PipeInit( ReadBuff, INPUT_SIZE, &InputPipe );
	PipeInit( OutputBuff, OUTPUT_SIZE, &OutputPipe );
	SocketInit( InputPipe, OutputPipe, &IoSocket );
}

void IoWrite( char * buff, COUNT size )
{
	//TODO
}

COUNT IoRead( char * buff, COUNT size )
{
	//TODO
}

