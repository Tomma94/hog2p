#include <iostream>
#include <string.h>
#include <cassert>
#include "PancakeExperiment.h"
#include "MNExperiment.h"

int main(int argc, char* argv[])
{
    if (argc ==1){
        assert(!"Please provide domain");
    }

    if (strcmp(argv[1], "-pancake") == 0){
        if(argc>=4){
            TestPancake(atoi(argv[2]), atof(argv[3]), atoi(argv[4]));
        }
        else {
            assert(!"Not enough arguments");
        }

    }


    else if (strcmp(argv[1], "-stp") == 0){
        TestMN();
    }
    else{
        assert(!"Domain unrecognized");
    }
}