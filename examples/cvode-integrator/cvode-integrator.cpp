#include <iostream>

#include <csim/model.h>

int main(int argc, char* argv[])
{
    std::cout << "bob" << std::endl;
    csim::Model model;
    model.loadCellmlModel("");
    return 0;
}
