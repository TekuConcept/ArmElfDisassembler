#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>



void print_usage(const char* program)
{ std::cout << "Usage: " << program << " <hex-file> [bin-file]\n"; }



char char2nibble(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    else if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    else if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    else {
        std::cout << "Unrecognized character " << (int)c << std::endl;
        std::cout << "Aborting!\n";
        exit(1);
        return 0;
    }
}



int translate(std::istream& in, std::ostream& out) {
    char c;
    char byte = 0;
    unsigned int count = 0;

    std::cout << std::setfill('0') << std::hex;
    in >> std::noskipws;
    while (in >> c) {
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
            continue;
        byte = (byte << 4) | (0xF & char2nibble(c));
        //std::cout << c;
        if ((count % 2) == 1) {
            //std::cout << ':' << std::setw(2) << (unsigned int)(byte) << ' ';
            out.write(&byte, 1);
            byte = 0;
        }
        count++;
	//if (count == 20) break;
    }
    std::cout << std::setfill(' ') << std::dec;
    std::cout << std::endl;
    return 0;
}



int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "No file!\n";
        print_usage(argv[0]);
        return 1;
    }
    else if (argc > 3) {
        std::cerr << "Too many arguments\n";
        print_usage(argv[0]);
        return 1;
    }

    std::string outfilename;
    std::ifstream in;
    std::ofstream out;

    if (argc == 3)
        outfilename = argv[2];
    else outfilename = "out.bin";

    in.open(argv[1]);
    if (!in.is_open()) {
        std::cerr << "Failed to open file " << argv[1] << std::endl;
        return 1;
    }
    out.open(outfilename, std::ios::trunc);
    if (!out.is_open()) {
        in.close();
        std::cerr << "Failed to open file " << outfilename << std::endl;
        return 1;
    }

    if (translate(in, out)) {
        out.close();
        in.close();
        std::cerr << "Could not translate hex to binary\n";
        return 1;
    }

    out.close();
    in.close();
    return 0;
}


