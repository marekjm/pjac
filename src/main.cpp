#include <sys/stat.h>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
using namespace std;


class Exception {
    public:
        virtual string what() const = 0;
        Exception() {}
        virtual ~Exception() {}
};


class IOException: public Exception {
};

class ReadException: public IOException {
    string filename;

    public:
        string what() const override {
            return filename;
        }
        ReadException(): filename("") {}
        ReadException(const string& f): filename(f) {}
};

class NoSuchFile: public ReadException {
    string filename;

    public:
        string what() const override {
            return filename;
        }
        NoSuchFile(const string& f): filename(f) {}
};


namespace support {
    namespace env {
        bool isfile(const string& path) {
            struct stat sf;

            // not a file if stat returned error
            if (stat(path.c_str(), &sf) == -1) return false;
            // not a file if S_ISREG() macro returned false
            if (not S_ISREG(sf.st_mode)) return false;

            // file otherwise
            return true;
        }
    }

    namespace io {
        vector<string> readlines(const string& filename) {
            vector<string> lines;
            string line;

            if (!support::env::isfile(filename)) {
                throw NoSuchFile(filename);
            }

            ifstream in(filename, ios::in | ios::binary);
            if (!in) {
                throw ReadException(filename);
            }

            while (getline(in, line)) {
                lines.push_back(line);
            }

            return lines;
        }
    }
}


int main(int argc, char **argv) {
    // setup command line arguments vector
    vector<string> args;

    for (int i = 1; i < argc; ++i) {
        args.push_back(string(argv[i]));
    }

    string filename(""), compilename("");

    if (args.size() == 0) {
        cout << "fatal: no input file" << endl;
        return 1;
    }

    if (compilename == "") {
        compilename = "out.asm";
    }

    filename = args[0];
    if (!filename.size()) {
        cout << "fatal: no file to assemble" << endl;
        return 1;
    }
    if (!support::env::isfile(filename)) {
        cout << "fatal: no such file: " << filename << endl;
        return 1;
    }
    vector<string> lines = support::io::readlines(filename);

    return 0;
}
