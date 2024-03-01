
#include "../src/comm/GrpcService.h"
#include <string>
#include <numeric>


int main(int argc, char** argv)
{
    GrpcService grpc;
    grpc.Run(GRPC_PORT);
    while(true){
        usleep(100);
    }
};