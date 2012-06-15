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

void setup()
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
        modbusHandler();
        serialHandler();
    }

    // something has gone terribly wrong. release resources and reset the system
    modbus_mapping_free(mb_mapping);
    close(socket);
    modbus_free(ctx);

    // TODO: figure out how to flag the system to reset!
    
    return 0;
}