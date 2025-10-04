#include <iostream>

#include "ExePacker.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input_executable> <output_sfx>" << std::endl;
        return 1;
    }

    ExePacker packer(argv[0]);

    if (packer.HasPayload()) {
        return packer.RunAsUnpacker(argc, argv);
    }
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input_executable> <output_sfx>" << std::endl;
        return 1;
    }
    return packer.RunAsPacker(argv[1], argv[2]);
}
