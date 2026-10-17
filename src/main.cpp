#include <iostream>
#include "model/cli.hpp"

using namespace std;

int main(int argc, char* argv[]) {
    if((argv[argc-1][0] == '-') || (argv[argc-1][0] == '@')) {
        if(argv[argc-1] == "--web") {
            cout<<"Web mode is not implemented yet."<<endl;
        } else {
            cli_main(argc, argv);
        }
    }else{
        cout<<"desktop mode is not implemented yet."<<endl;
    }
    return 0;
}