#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <modbus.h>

#define BAUDRATE B9600
#define PORT_0 "/dev/ttyS0"


enum {
    TCP,
    RTU
};

int socket;
modbus_t *ctx;
modbus_mapping_t *mb_mapping;
int rc;

struct termios termOptions;
int ttyFid;

int data[10];

void setup()
{
	// initialize data registers
	data[0] = 0;
	data[1] = 1;
	data[2] = 2;
	data[3] = 3;
	data[4] = 4;
	data[5] = 5;
	data[6] = 6;
	data[7] = 7;
	data[8] = 8;
	data[9] = 9;
}

void updateMemoryTable()
{
}

void serialHandler()
{
    // open the tty
    ttyFid = open( port, O_RDWR );
    if (ttyFid == -1)
    {
        printf( "Error unable to open port: %s\n", port );
        return -1;
    }
    
    // get the current options:
    tcgetattr( ttyFid, &termOptions );

    // Set the input/output speed to 9600
    cfsetispeed( &termOptions, BAUDRATE );

    // Now set the term options (set immediately)
    tcsetattr( ttyFid, TCSANOW, &termOptions );
	
	//
	// **************************************************************
	//
	byte bufferIndex = 0;
    char buffer[25];

    // read the position command from the serial port
    // should look like:
    //      D20BIX+00733Y+00080S99\r\n
    //  
    // and we need the X+00000 and Y+00000 parts
    //
    if (Serial.available() > 0)
	{
        while (Serial.available() > 0 && bufferIndex < 25)
		{
            buffer[bufferIndex] = Serial.read();

            if (buffer[bufferIndex++] == '\n')
			{
                buffer[bufferIndex] = '\0';
                break;
            }
        }

        Serial.flush();

		// check to see if we have good orientation on the buffer by
		// checking for lines starting with model identifier 'D'
		String input = String(buffer);

		if (buffer[0] == 'D' && bufferIndex <=24)
		{  
			int x_result = data[1];
			int y_result = data[4];
			String check;
			char token_buffer[8] = {'0', '0', '0', '0', '0', '0', '0', '0' };

			// scan for x, target token is X+00000
			String x_token = input.substring(5, 11);
			check = x_token.substring(2, 6);
			check.toCharArray(token_buffer, 8);

			x_result = atoi(token_buffer);
			if (x_token[1] == '-')
			{
				x_result *= -1;
			}

			// scan for y, target token is Y+00000
			String y_token = input.substring(12, 18);
			check = y_token.substring(2, 6);
			check.toCharArray(token_buffer, 8);

			y_result = atoi(token_buffer);
			if (y_token[1] == '-')
			{
				y_result *= -1;
			}

			// finalize results
			data[1] = x_result;
			data[4] = y_result;
		}
	}
}

void modbusHandler()
{
    if (ctx == NULL)
    {
        ctx = modbus_new_tcp("127.0.0.1", 502);
        socket = modbus_tcp_listen(ctx, 1);
        modbus_tcp_accept(ctx, &socket);
        
        mb_mapping = modbus_mapping_new(MODBUS_MAX_READ_BITS, 0,
                                    MODBUS_MAX_READ_REGISTERS, 0);
                                    
        if (mb_mapping == NULL) {
            \\fprintf(stderr, "Failed to allocate the mapping: %s\n", modbus_strerror(errno));
            modbus_free(ctx);
            return;
        }
    }
    }

    uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];

    rc = modbus_receive(ctx, query);
    if (rc >= 0) {
        modbus_reply(ctx, query, rc, mb_mapping);
    } else {
        modbus_mapping_free(mb_mapping);
        close(socket);
        modbus_free(ctx);
    }
}

int main(int argc, char *argv[])
{
    
    
    setup();

    for(;;) {
        serialHandler();
		modbusHandler();
		
		updateMemoryTable();
    }

    // something has gone terribly wrong. release resources and reset the system
    modbus_mapping_free(mb_mapping);
    close(socket);
    modbus_free(ctx);

    // TODO: figure out how to flag the system to reset!
    
    return 0;
}