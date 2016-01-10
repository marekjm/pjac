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


class Token {
    string::size_type line_number;
    string::size_type character_number;
    string::size_type byte_number;
    string token_string;

    public:
        decltype(line_number) line() const { return line_number; }
        decltype(character_number) character() const { return character_number; }
        decltype(byte_number) byte() const { return byte_number; }
        string text() const { return token_string; }

        Token(const string& s, string::size_type ln, string::size_type cn, string::size_type bn):
            line_number(ln),
            character_number(cn),
            byte_number(bn),
            token_string(s) {
            }
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

        string readfile(const string& filename) {
            if (!support::env::isfile(filename)) {
                throw NoSuchFile(filename);
            }

            ifstream in(filename, ios::in | ios::binary);
            if (!in) {
                throw ReadException(filename);
            }

            ostringstream text;
            string line;
            if (getline(in, line)) {
                text << line;
            }
            while (getline(in, line)) {
                text << '\n' << line;
            }

            return text.str();
        }
    }

    namespace str {
        bool startswith(const std::string& s, const std::string& w) {
            /*  Returns true if s stars with w.
             */
            return (s.compare(0, w.length(), w) == 0);
        }

        string sub(const string& s, long unsigned b = 0, long int e = -1) {
            /*  Returns substring of s.
             *  If only s is passed, returns copy of s.
             */
            if (b == 0 and e == -1) return string(s);

            ostringstream part;
            part.str("");

            long unsigned end;
            if (e < 0) { end = (s.size() + e + 1); }
            else { end = static_cast<long unsigned>(e); }

            for (long unsigned i = b; i < s.size() and i < end; ++i) {
                part << s[i];
            }

            return part.str();
        }

        string lstrip(const string& s) {
            /*  Removes whitespace from left side of the string.
             */
            unsigned i = 0;
            while (i < s.size()) {
                if (not (s[i] == ' ' or s[i] == '\t' or s[i] == '\v' or s[i] == '\n')) {
                    break;
                };
                ++i;
            }
            return sub(s, i);
        }

        string chunk(const string& s, bool ignore_leading_ws = true) {
            /*  Returns part of the string until first whitespace from left side.
             */
            ostringstream chnk;

            string str = (ignore_leading_ws ? lstrip(s) : s);

            for (unsigned i = 0; i < str.size(); ++i) {
                if (str[i] == *" " or str[i] == *"\t" or str[i] == *"\v" or str[i] == *"\n") break;
                chnk << str[i];
            }
            return chnk.str();
        }

        bool startswithchunk(const std::string& s, const std::string& w) {
            /*  Returns true if s stars with chunk w.
             */
            return (chunk(s) == w);
        }

        bool endswith(const std::string& s, const std::string& w) {
            /*  Returns true if s ends with w.
             */
            return (s.compare(s.length()-w.length(), s.length(), w) == 0);
        }

        bool isnum(const std::string& s, bool negatives = true) {
            /*  Returns true if s contains only numerical characters.
             *  Regex equivalent: `^[0-9]+$`
             */
            bool num = false;
            unsigned start = 0;
            if (s[0] == '-' and negatives) {
                // must handle negative numbers
                start = 1;
            }
            for (unsigned i = start; i < s.size(); ++i) {
                switch (s[i]) {
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                        num = true;
                        break;
                    default:
                        num = false;
                }
                if (!num) break;
            }
            return num;
        }

        bool isfloat(const std::string& s, bool negatives = true) {
            /*  Returns true if s contains only numerical characters.
             *  Regex equivalent: `^[0-9]+\.[0-9]+$`
             */
            bool is = false;
            unsigned start = 0;
            if (s[0] == '-' and negatives) {
                // to handle negative numbers
                start = 1;
            }
            int dot = -1;
            for (unsigned i = start; i < s.size(); ++i) {
                if (s[i] == '.') {
                    dot = static_cast<int>(i);
                    break;
                }
            }
            is = isnum(sub(s, 0, dot), negatives) and isnum(sub(s, (static_cast<unsigned>(dot)+1)));
            return is;
        }

        vector<string> chunks(const string& s) {
            /*  Returns chunks of string.
             */
            vector<string> chnks;
            string tmp(lstrip(s));
            string chnk;
            while (tmp.size()) {
                chnk = chunk(tmp);
                tmp = lstrip(sub(tmp, chnk.size()));
                chnks.push_back(chnk);
            }
            return chnks;
        }

        string join(const string& s, const vector<string>& parts) {
            /** Join elements of vector with given string.
             */
            ostringstream oss;
            long unsigned limit = parts.size();
            for (long unsigned i = 0; i < limit; ++i) {
                oss << parts[i];
                if (i < (limit-1)) {
                    oss << s;
                }
            }
            return oss.str();
        }

        string extract(const string& s) {
            /** Extracts *enquoted chunk*.
             *
             *  It is particularly useful if you have a string encoded in another string.
             *
             *  This function will return `"Hello 'Beautiful' World!"` when fed `"Hello 'Beautiful' World!" some other (42) things;`, and
             *  will return `'Hello "Beautiful" World!'` when fed `'Hello "Beautiful" World!' some other (42) things;`.
             *  Starting quote character is irrelevant.
             *
             *  In fact, this function will treat *the first character* of the string it is fed as a delimiter for
             *  string extraction - whatever that may be (e.g. the backtick character) so you can get creative.
             *  One character that is not recommended for use as a delimiter is the backslash as it is treated specially (as
             *  the escape character) by this function.
             */
            if (s.size() == 0) {
                return string("");
            }

            ostringstream chnk;
            char quote;
            chnk << (quote = s[0]);

            int backs = 0;
            for (unsigned i = 1; i < s.size(); ++i) {
                chnk << s[i];
                if (s[i] == quote and (backs == 0)) {
                    break;
                }
                if (s[i] == quote and (backs != 0)) {
                    backs = 0;
                    continue;
                }
                if (s[i] == '\\') {
                    ++backs;
                }
            }

            return chnk.str();
        }

        unsigned lshare(const string& s, const string& w) {
            unsigned share = 0;
            for (unsigned i = 0; i < s.size() and i < w.size(); ++i) {
                if (s[i] == w[i]) {
                    ++share;
                } else {
                    break;
                }
            }
            return share;
        }
        bool contains(const string&s, const char c) {
            bool it_does = false;
            for (unsigned i = 0; i < s.size(); ++i) {
                if (s[i] == c) {
                    it_does = true;
                    break;
                }
            }
            return it_does;
        }

        string enquote(const string& s) {
            /** Enquote the string.
             */
            ostringstream encoded;
            char closing = '"';

            encoded << closing;
            for (unsigned i = 0; i < s.size(); ++i) {
                if (s[i] == closing) { encoded << "\\"; }
                encoded << s[i];
            }
            encoded << closing;

            return encoded.str();
        }

        string strdecode(const string& s) {
            /** Decode escape sequences in strings.
             *
             *  This function recognizes escape sequences as listed on:
             *  http://en.cppreference.com/w/cpp/language/escape
             *  The function does not recognize sequences for:
             *      - arbitrary octal numbers (escape: \nnn),
             *      - arbitrary hexadecimal numbers (escape: \xnn),
             *      - short arbitrary Unicode values (escape: \unnnn),
             *      - long arbitrary Unicode values (escape: \Unnnnnnnn),
             *
             *  If a character that does not encode an escape sequence is
             *  preceded by a backslash (\\) the function consumes the backslash and
             *  leaves only the character preceded by it in the outpur string.
             *
             */
            ostringstream decoded;
            char c;
            for (unsigned i = 0; i < s.size(); ++i) {
                c = s[i];
                if (c == '\\' and i < (s.size()-1)) {
                    ++i;
                    switch (s[i]) {
                        case '\'':
                            c = '\'';
                            break;
                        case '"':
                            c = '"';
                            break;
                        case '?':
                            c = '?';
                            break;
                        case '\\':
                            c = '\\';
                            break;
                        case 'a':
                            c = '\a';
                            break;
                        case 'b':
                            c = '\b';
                            break;
                        case 'f':
                            c = '\f';
                            break;
                        case 'n':
                            c = '\n';
                            break;
                        case 'r':
                            c = '\r';
                            break;
                        case 't':
                            c = '\t';
                            break;
                        case 'v':
                            c = '\v';
                            break;
                        default:
                            c = s[i];
                    }
                }
                decoded << c;
            }
            return decoded.str();
        }
        string strencode(const string& s) {
            /** Encode escape sequences in strings.
             *
             *  Reverse of strdecode() function.
             */
            ostringstream encoded;
            char c;
            for (unsigned i = 0; i < s.size(); ++i) {
                c = s[i];
                switch (s[i]) {
                    case '\\':
                        encoded << "\\\\";
                        break;
                    case '\a':
                        encoded << "\\a";
                        break;
                    case '\b':
                        encoded << "\\b";
                        break;
                    case '\f':
                        encoded << "\\f";
                        break;
                    case '\n':
                        encoded << "\\n";
                        break;
                    case '\r':
                        encoded << "\\r";
                        break;
                    case '\t':
                        encoded << "\\t";
                        break;
                    case '\v':
                        encoded << "\\v";
                        break;
                    default:
                        encoded << c;
                        break;
                }
            }
            return encoded.str();
        }

        string stringify(const vector<string>& sv) {
            ostringstream oss;
            oss << '[';
            long unsigned sz = sv.size();
            for (long unsigned i = 0; i < sz; ++i) {
                oss << enquote(sv[i]);
                if (i < (sz-1)) {
                    oss << ", ";
                }
            }
            oss << ']';
            return oss.str();
        }
        string stringify(unsigned n) {
            ostringstream oss;
            oss << n << endl;
            return oss.str();
        }

        vector<string> tokenize(const string& s) {
            vector<string> tokens;
            ostringstream token;
            string::size_type i = 0;

            char c; // for by-character extraction
            string tk; // for string extraction
            while (i < s.size()) {
                c = s[i];
                switch (c) {
                    case ' ':
                    case '\t':
                        if (token.str().size() != 0) {
                            tokens.push_back(token.str());
                            token.str("");
                        }
                        break;
                    case '\n':
                    case '(':
                    case ')':
                    case '[':
                    case ']':
                    case '{':
                    case '}':
                    case '<':
                    case '>':
                    case '~':
                    case '!':
                    case '@':
                    case '#':
                    case '$':
                    case '%':
                    case '^':
                    case '&':
                    case '*':
                    case '-':
                    case '+':
                    case '=':
                    case '|':
                    case '\\':
                    case ':':
                    case ';':
                    case ',':
                    case '.':
                    case '?':
                    case '/':
                        if (token.str().size() != 0) {
                            tokens.push_back(token.str());
                            token.str("");
                        }
                        token << c;
                        tokens.push_back(token.str());
                        token.str("");
                        break;
                    case '"':
                    case '\'':
                        if (token.str().size() != 0) {
                            tokens.push_back(token.str());
                            token.str("");
                        }
                        tk = support::str::extract(s.substr(i));
                        i += tk.size();
                        tokens.push_back(tk);
                        break;
                    default:
                        token << c;
                        break;
                }
                ++i;
            }

            if (token.str().size()) {
                tokens.push_back(token.str());
            }

            return tokens;
        }
    }
}


vector<string> removeComments(const vector<string>& tks) {
    vector<string> tokens;

    string token = "", previous_token = "";
    for (vector<string>::size_type i = 0; i < tks.size(); ++i) {
        token = tks[i];

        if (token == "/" and previous_token == "/") {
            // double-slash comments go until the first newline
            while (tks[++i] != "\n");
            // last pushed token has to be removed as it is the starting "/"
            tokens.pop_back();
            continue;
        }

        previous_token = token;
        tokens.push_back(token);
    }

    return tokens;
}


vector<string> mapStringVector(const vector<string>& sv, string(*fn)(const string&)) {
    vector<string> mapped;
    for (vector<string>::size_type i = 0; i < sv.size(); ++i) {
        mapped.push_back(fn(sv[i]));
    }
    return mapped;
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

    string source_text = support::io::readfile(filename);

    vector<string> primitive_tokens = support::str::tokenize(source_text);
    vector<string> decommented_tokens = removeComments(primitive_tokens);

    cout << "token count [primitive]: " << primitive_tokens.size() << endl;
    cout << "token count [decommented]: " << decommented_tokens.size() << endl;
    cout << support::str::join(" ", mapStringVector(decommented_tokens, &support::str::strencode)) << endl;

    return 0;
}
