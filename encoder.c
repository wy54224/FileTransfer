#include "haffmantree.h"

int main(int argc, char **args){
    buildHaffmanTreeFromFile(args[1]);
    encode(args[1], args[2]);
    destroy_all();
    return 0;
}