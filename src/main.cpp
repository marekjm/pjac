#include <sys/stat.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <fstream>
#include <algorithm>
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

class InvalidSyntax: public Exception {
        vector<string>::size_type token_no;
        string message;
    public:
        string what() const override {
            ostringstream oss;
            oss << "invalid syntax on token " << token_no << ": " << message;
            return oss.str();
        }
        decltype(token_no) tokenIndex() const {
            return token_no;
        }
        InvalidSyntax(decltype(token_no) t, const string& m): token_no(t), message(m) {}
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
                        // -1 to not consume the next token as it
                        // may be meaningful (e.g. a semicolon)
                        i += (tk.size()-1);
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
            while (++i < tks.size() and tks[i] != "\n");
            // last pushed token has to be removed as it is the starting "/"
            tokens.pop_back();
            continue;
        }
        if (token == "*" and previous_token == "/") {
            // block comments go from "/*" to "*/"
            // last pushed token has to be removed as it is the starting "/"
            tokens.pop_back();
            // skip the "*" of comment opening
            ++i;
            while (++i < tks.size() and not (tks[i-1] == "*" and tks[i] == "/"));
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

vector<string>::size_type processVariable(const vector<string>& tokens, vector<string>::size_type offset, map<string, unsigned>& variable_registers, map<string, string>& variable_types, map<string, string>& variable_values, ofstream& output) {
    vector<string>::size_type i = offset;
    string var_name = "", var_type = "", var_value = "";
    unsigned var_register = 0;

    var_type = tokens[i];
    var_name = tokens[++i];
    var_register = variable_registers.size();

    variable_registers[var_name] = var_register;
    variable_types[var_name] = var_type;

    ++i;

    if (tokens[i] == ";") {
        if (var_type == "int") {
            var_value = "0";
        } else if (var_type == "string") {
            var_value = "''";
        } else if (var_type == "float") {
            var_value = "0.0";
        }
    } else if (tokens[i] == "=") {
        var_value = tokens[++i];
        // skip terminating ";"
        ++i;
    }
    variable_values[var_name] = var_value;

    if (variable_registers.count(var_value)) {
        output << "    copy " << var_register << ' ' << variable_registers[var_value] << endl;
    } else {
        output << "    ";
        if (var_type == "int") {
            output << "istore";
        } else if (var_type == "string") {
            output << "strstore";
        } else if (var_type == "float") {
            output << "fstore";
        }
        output << ' ' << var_register << ' ' << var_value << endl;
    }

    return (i-offset);
}

vector<string>::size_type processFrame(const vector<string>& tokens, const string& function_to_call, vector<string>::size_type offset, map<string, unsigned>& variable_registers, ofstream& output) {
    vector<string>::size_type i = offset;
    vector<unsigned> parameter_sources;

    if (tokens[i] == ")") {
        output << "    frame 0" << endl;
        return 2; // number of processed tokens is 2: "(" and ";"
    }

    for (; i < tokens.size() and tokens[i] != ";"; ++i) {
        if (variable_registers.count(tokens[i]) == 0) {
            cout << "fatal: undefined name as parameter: `" << tokens[i] << "` in call to function `" << function_to_call << '`' << endl;
            exit(1);
        }
        parameter_sources.push_back(variable_registers.at(tokens[i]));
        // account for both "," between parameters and
        // closing ")"
        ++i;
    }

    output << "    frame ^[";
    for (unsigned j = 0; j < parameter_sources.size(); ++j) {
        output << "(param " << j << ' ' << parameter_sources[j] << ')';
        if (j < (parameter_sources.size()-1)) {
            output << ' ';
        }
    }
    output << "]" << endl;

    // skip terminating ";"
    ++i;

    return (i-offset);
}

vector<string>::size_type processCall(const vector<string>& tokens, vector<string>::size_type offset, map<string, unsigned>& variable_registers, ofstream& output) {
    string function_to_call = tokens[offset++];
    // skip opening "("
    ++offset;
    vector<string>::size_type i = (processFrame(tokens, function_to_call, offset, variable_registers, output) + 2);
    output << "    call 0 " << function_to_call << endl;

    return i;
}
vector<string>::size_type processCallWithReturnValueUsed(const vector<string>& tokens, vector<string>::size_type offset, map<string, unsigned>& variable_registers, ofstream& output) {
    string return_to = tokens[offset++];

    // skip "="
    ++offset;

    string function_to_call = tokens[offset++];

    // skip opening "("
    ++offset;

    vector<string>::size_type i = (processFrame(tokens, function_to_call, offset, variable_registers, output) + 4);
    output << "    call " << variable_registers.at(return_to) << ' ' << function_to_call << endl;

    return i;
}

void annotateInvalidToken(const vector<string>& tokens, vector<string>::size_type no) {
    for (vector<string>::size_type i = 0; i < tokens.size(); ++i) {
        cout << ((i == no) ? ">>> " : "    ") << support::str::strencode(tokens[i]) << endl;
    }
}

vector<string>::size_type processFunction(const vector<string>& tokens, vector<string>::size_type offset, ofstream& output) {
    vector<string>::size_type number_of_processed_tokens = 0;

    string function_name = tokens[offset + (number_of_processed_tokens++)];
    map<string, unsigned> variable_registers;
    map<string, string> variable_types;
    map<string, string> variable_values;

    bool has_returned = false;

    output << ".function: " << function_name << endl;

    for (; number_of_processed_tokens+offset < tokens.size(); ++number_of_processed_tokens) {
        if (tokens[offset+number_of_processed_tokens] == "var") {
            number_of_processed_tokens += processVariable(tokens, (offset + (++number_of_processed_tokens)), variable_registers, variable_types, variable_values, output);
        } else if (tokens[offset+number_of_processed_tokens] == "return") {
            has_returned = true;

            // skip "return" keyword
            ++number_of_processed_tokens;

            // this if deals with `return <token> ;` case
            if (tokens[offset + number_of_processed_tokens] != ";") {
                // this if deals with `return <number> ;` case
                if (support::str::isnum(tokens[offset+number_of_processed_tokens])) {
                    if (tokens[offset+number_of_processed_tokens] == "0") {
                        output << "    izero 0" << endl;
                    } else {
                        output << "    istore 0 " << tokens[offset+number_of_processed_tokens] << endl;
                    }
                } else if (variable_registers[tokens[offset+number_of_processed_tokens]] != 0) {
                    output << "    move 0 " << variable_registers[tokens[offset+number_of_processed_tokens]] << endl;
                }

                // advance after the returned <token>
                ++number_of_processed_tokens;
            }

            // no need to deal with terminating ";" as loop increment will take care of it
            output << "    end" << endl;
        } else if (tokens[offset+number_of_processed_tokens] == "asm") {
            output << "    ";
            while (tokens[offset + (++number_of_processed_tokens)] != ";") {
                output << tokens[offset+number_of_processed_tokens] << ' ';
            }
            output << endl;
        } else if (tokens[offset+number_of_processed_tokens] == "\n") {
            continue;
        } else if (tokens[offset+number_of_processed_tokens] == ";") {
            continue;
        } else if (tokens[offset+number_of_processed_tokens] == "end") {
            break;
        } else {
            if ((offset+number_of_processed_tokens+3) >= tokens.size()) {
                throw InvalidSyntax((offset+number_of_processed_tokens), ("missing tokens during call to " + tokens[offset+number_of_processed_tokens]));
            } else if (tokens[offset+number_of_processed_tokens+1] == "(") {
                number_of_processed_tokens += processCall(tokens, (offset + number_of_processed_tokens), variable_registers, output);
            } else if (variable_registers.count(tokens[offset+number_of_processed_tokens]) and tokens[offset+number_of_processed_tokens+1] == "=" and tokens[offset+number_of_processed_tokens+3] == "(") {
                number_of_processed_tokens += processCallWithReturnValueUsed(tokens, (offset+number_of_processed_tokens), variable_registers, output);
            } else {
                throw InvalidSyntax((offset+number_of_processed_tokens), "other error");
            }
        }
    }

    if (not has_returned) {
        output << "    end" << endl;
    }
    output << ".end" << endl;

    return number_of_processed_tokens;
}


void processSource(const vector<string>& tokens, ofstream& output) {
    string previous_token = "", token = "";
    for (vector<string>::size_type i = 0; i < tokens.size(); ++i) {
        token = tokens[i];
        if (token == "function" and previous_token == "begin") {
            i += processFunction(tokens, ++i, output);
        }
        previous_token = token;
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

    filename = args[0];
    if (!filename.size()) {
        cout << "fatal: no file to assemble" << endl;
        return 1;
    }
    if (!support::env::isfile(filename)) {
        cout << "fatal: no such file: " << filename << endl;
        return 1;
    }

    if (compilename == "") {
        compilename = (filename + ".asm");
    }

    string source_text = support::io::readfile(filename);

    vector<string> primitive_tokens = support::str::tokenize(source_text);
    vector<string> decommented_tokens = removeComments(primitive_tokens);

    try {
        ofstream out(compilename);
        processSource(decommented_tokens, out);
    } catch (const InvalidSyntax& e) {
        cout << "fatal: " << e.what() << endl;
        annotateInvalidToken(decommented_tokens, e.tokenIndex());
        return 1;
    }

    return 0;
}
