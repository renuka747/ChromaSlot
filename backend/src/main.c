#include "server.h"
#include <stdio.h>

int main(int argc, char **argv) {
    const char *port = "8000";
    if (argc > 1) port = argv[1];
    
    printf("Starting ChromaSlot backend...\n");
    if (start_server(port) != 0) {
        fprintf(stderr, "Failed to start server on port %s\n", port);
        return 1;
    }
    return 0;
}
