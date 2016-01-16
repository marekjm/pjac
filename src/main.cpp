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
            return message;
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
        void text(const string& s) { token_string = s; }
        void textprepend(const string& s) { token_string = (s + token_string); }

        bool operator==(const string& s) const {
            return (token_string == s);
        }
        bool operator!=(const string& s) const {
            return (token_string != s);
        }

        operator std::string() const {
            return token_string;
        }

        Token(const string& s, string::size_type ln, string::size_type cn, string::size_type bn):
            line_number(ln),
            character_number(cn),
            byte_number(bn),
            token_string(s) {
            }
        Token():
            line_number(0),
            character_number(0),
            byte_number(0),
            token_string("") {
            }
};
using TokenVector = vector<Token>;
using TokenVectorSize = TokenVector::size_type;


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

        bool isalpha(const std::string& s) {
            /** Returns true if s contains only letters.
             */
            bool is_alpha = true;
            for (std::string::size_type i = 0; i < s.size(); ++i) {
                if (not ((s[i] >= 'A' and s[i] <= 'Z') or (s[i] >= 'a' and s[i] <= 'z'))) {
                    is_alpha = false;
                    break;
                }
            }
            return is_alpha;
        }

        bool isname(const std::string& s) {
            /** Returns true if s is a valid name.
             */
            if (s.size() == 0) {
                return false;
            }
            if (not isalpha(s.substr(0, 1))) {
                return false;
            }
            bool is_name = true;
            for (std::string::size_type i = 1; i < s.size(); ++i) {
                if (not ((s[i] >= 'A' and s[i] <= 'Z') or (s[i] >= 'a' and s[i] <= 'z') or (s[i] >= '0' and s[i] <= '9') or s[i] == '_')) {
                    is_name = false;
                    break;
                }
            }
            return is_name;
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

        bool isbooleanliteral(const string& s) {
            return (s == "true" or s == "false");
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

        Token getToken(ostringstream& t, string::size_type line_no, string::size_type char_no, string::size_type byte_no) {
            string tok = t.str();
            return Token(tok, line_no, (char_no - tok.size()), (byte_no - tok.size()));
        }
        Token getToken(const string& tok, string::size_type line_no, string::size_type char_no, string::size_type byte_no) {
            return Token(tok, line_no, (char_no - tok.size()), (byte_no - tok.size()));
        }

        TokenVector lex(const string& s) {
            TokenVector tokens;
            ostringstream token;
            string::size_type i = 0;

            string::size_type line_no = 0;
            string::size_type byte_no = 0;
            string::size_type char_no = 0;

            char c; // for by-character extraction
            string tk; // for string extraction
            while (i < s.size()) {
                c = s[i];
                switch (c) {
                    case ' ':
                    case '\t':
                        if (token.str().size() != 0) {
                            tokens.push_back(getToken(token, line_no, char_no, byte_no));
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
                            tokens.push_back(getToken(token, line_no, char_no, byte_no));
                            token.str("");
                        }
                        token << c;
                        tokens.push_back(getToken(token, line_no, char_no, byte_no));
                        token.str("");
                        if (c == '\n') {
                            ++line_no;
                            char_no = 0;
                        }
                        break;
                    case '"':
                    case '\'':
                        if (token.str().size() != 0) {
                            tokens.push_back(getToken(token, line_no, char_no, byte_no));
                            token.str("");
                        }
                        tk = support::str::extract(s.substr(i));
                        // -1 to not consume the next token as it
                        // may be meaningful (e.g. a semicolon)
                        i += (tk.size()-1);
                        tokens.push_back(getToken(tk, line_no, char_no, byte_no));
                        break;
                    default:
                        token << c;
                        break;
                }
                ++byte_no;
                ++char_no;
                ++i;
            }

            if (token.str().size()) {
                tokens.push_back(getToken(token, line_no, char_no, byte_no));
            }

            return tokens;
        }
    }
}

struct FunctionEnvironment;

struct FunctionSignature {
    string function_name;
    string return_type;
    vector<string> parameters;
    map<string, string> parameter_types;

    string header(bool full = false) const {
        ostringstream oss;
        oss << function_name << '(';
        auto limit = (parameters.size()-1);
        for (vector<string>::size_type i = 0; i < parameters.size(); ++i) {
            oss << parameter_types.at(parameters[i]);
            if (full) {
                oss << ' ' << parameters[i];
            }
            if (i < limit) {
                oss << ", ";
            }
        }
        oss << ")->" << return_type;
        return oss.str();
    }

    FunctionSignature(const string& n, const string& r):
        function_name(n),
        return_type(r)
    {}
    FunctionSignature(): function_name(""), return_type("") {}
};

struct CompilationEnvironment {
    map<string, string> functions;
    map<string, FunctionSignature> signatures;
};

struct Scope {
    map<string, unsigned> variable_registers;
    map<string, string> variable_types;
    map<string, string> variable_values;

    unsigned ifs;

    unsigned whiles;
    string loop_begin;
    string loop_end;

    Scope* parent;
    FunctionEnvironment* function;

    unsigned size() const {
        unsigned sz = variable_registers.size();
        if (parent) {
            sz += parent->size();
        }
        return sz;
    }

    vector<string> names() const {
        vector<string> ns;
        if (parent) {
            ns = parent->names();
        }

        auto p = variable_registers.begin();
        while (p != variable_registers.end()) {
            ns.push_back(p->first);
            ++p;
        }

        return ns;
    }

    bool defined(const string& name) {
        if (variable_registers.count(name)) {
            return true;
        } else if (parent == nullptr) {
            return false;
        } else {
            return parent->defined(name);
        }
    }

    unsigned registerof(const string& name, TokenVectorSize offset) {
        if (variable_registers.count(name)) {
            return variable_registers.at(name);
        } else if (parent == nullptr) {
            throw InvalidSyntax(offset, ("access to name not present in scope: " + name));
        } else {
            return parent->registerof(name, offset);
        }
    }

    unsigned setregisterof(const string& name, unsigned n) {
        variable_registers[name] = n;
        return n;
    }

    string typeof(const string& name, TokenVectorSize offset) {
        if (variable_types.count(name)) {
            return variable_types.at(name);
        } else if (parent == nullptr) {
            throw InvalidSyntax(offset, ("access to name not present in scope: " + name));
        } else {
            return parent->typeof(name, offset);
        }
    }

    string settypeof(const string& name, const string& type) {
        variable_types[name] = type;
        return type;
    }

    string setvalueof(const string& name, const string& type) {
        variable_values[name] = type;
        return type;
    }

    bool isRegisteredClass(const string& s) {
        return (s == "int" or s == "float" or s == "string" or s == "bool" or s == "auto");
    }

    bool isDeclaredFunction(const string& s);
    FunctionSignature getFunctionSignature(const string& s);

    Scope(FunctionEnvironment *fn): parent(nullptr), function(fn) {}
    Scope(Scope* scp): parent(scp), function(nullptr) {}
    Scope(FunctionEnvironment *fn, Scope *scp): parent(scp), function(fn) {}
};

struct FunctionEnvironment {
    vector<string> parameters;
    map<string, string> parameter_types;

    string return_type;

    unsigned begin_balance;
    string function_name;

    bool has_returned;
    unsigned ifs;

    unsigned whiles;
    string loop_begin;
    string loop_end;

    CompilationEnvironment *env;
    Scope *scope;

    string header(bool full = false) const {
        ostringstream oss;
        oss << function_name << '(';
        auto limit = (parameters.size()-1);
        for (vector<string>::size_type i = 0; i < parameters.size(); ++i) {
            oss << parameter_types.at(parameters[i]);
            if (full) {
                oss << ' ' << parameters[i];
            }
            if (i < limit) {
                oss << ", ";
            }
        }
        oss << ")->" << return_type;
        return oss.str();
    }

    FunctionEnvironment(const string& s, CompilationEnvironment* ce):
        begin_balance(0),
        function_name(s),
        has_returned(false),
        ifs(0),
        whiles(0),
        loop_begin(""),
        loop_end(""),
        env(ce),
        scope(new Scope(this))
    {
        }
    ~FunctionEnvironment() {
        delete scope;
    }
};

bool Scope::isDeclaredFunction(const string& s) {
    return (function->env->signatures.count(s) or function->env->signatures.count("::" + s));
}

FunctionSignature Scope::getFunctionSignature(const string& s) {
    if (function->env->signatures.count(s)) {
        return function->env->signatures.at(s);
    } else {
        return function->env->signatures.at("::" + s);
    }
}


vector<Token> removeComments(const vector<Token>& tks) {
    vector<Token> tokens;

    Token token, previous_token;
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

vector<Token> reduceIntegers(const vector<Token>& tks) {
    vector<Token> tokens;

    Token token;
    for (vector<string>::size_type i = 0; i < tks.size(); ++i) {
        token = tks[i];
        if (support::str::isnum(token) and i) {
            if (i >= 2 and tks[i-1] == "-" and tks[i-2] == "=") {
                tokens.pop_back();
                token.textprepend("-");
            }
        }
        tokens.push_back(token);
    }
    return tokens;
}

vector<Token> reduceFloats(const vector<Token>& tks) {
    vector<Token> tokens;

    Token token;
    for (vector<string>::size_type i = 0; i < tks.size(); ++i) {
        token = tks[i];
        if (support::str::isnum(token) and i) {
            if (i >= 2 and tks[i-1] == "." and support::str::isnum(tks[i-2])) {
                token.textprepend(tks[i-2].text() + ".");
                tokens.pop_back();
                tokens.pop_back();
            }
        }
        tokens.push_back(token);
    }
    return tokens;
}

vector<Token> reduceNamespaceResolutionOperator(const vector<Token>& tks) {
    vector<Token> tokens;

    Token token;
    for (vector<string>::size_type i = 0; i < tks.size(); ++i) {
        token = tks[i];
        if (token == ":" and i) {
            if (tks[i-1] == ":") {
                token.text("::");
                tokens.pop_back();
            }
        }
        tokens.push_back(token);
    }
    return tokens;
}

vector<Token> reduceNamespacedNames(const vector<Token>& tks) {
    vector<Token> tokens;

    Token token;
    for (vector<string>::size_type i = 0; i < tks.size(); ++i) {
        token = tks[i];
        if (support::str::isname(token)) {
            while (i < (tks.size()-2)) {
                if (tks[i+1] == "::" and support::str::isname(tks[i+2])) {
                    token.text(token.text() + "::" + tks[i+2].text());
                    i += 2;
                } else {
                    break;
                }
            }
        }
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


TokenVectorSize processVariable(const TokenVector& tokens, TokenVectorSize offset, Scope* scope, ostringstream& output) {
    TokenVectorSize i = offset;
    string var_name = "", var_type = "", var_value = "";
    unsigned var_register = 0;

    var_type = tokens[i];
    var_name = tokens[++i];

    if (not support::str::isname(var_name)) {
        throw InvalidSyntax(i-1, ("invalid variable name in function " + scope->function->header() + ": " + var_name));
    }

    // never store in register 0, if the value is not for return
    var_register = scope->size()+1;

    ++i;

    output << "    .name: " << var_register << ' ' << var_name << '\n';

    if (tokens[i] == ";") {
        if (var_type == "int") {
            var_value = "0";
        } else if (var_type == "string") {
            var_value = "''";
        } else if (var_type == "float") {
            var_value = "0.0";
        } else if (var_type == "bool") {
            var_value = "false";
        } else if (var_type == "auto") {
            throw InvalidSyntax(i, ("unable to determine type of variable " + var_name + " in definition of function " +
                        scope->function->header() + "; 'auto' cannot be used without initialisation"));
        } else {
            throw InvalidSyntax(i, ("invalid type of variable " + var_name + " in definition of function " +
                        scope->function->header() + ": " + var_type));
        }
    } else if (tokens[i] == "=") {
        var_value = tokens[++i];
        // skip terminating ";"
        ++i;
    }

    if (scope->defined(var_value) and scope->typeof(var_value, i) == var_type) {
        output << "    copy " << var_register << ' ' << scope->registerof(var_value, i) << endl;
    } else if (scope->defined(var_value) and scope->typeof(var_value, i) != var_type and var_type == "auto") {
        var_type = scope->typeof(var_value, i);
        output << "    copy " << var_register << ' ' << scope->registerof(var_value, i) << endl;
    } else if (scope->defined(var_value) and scope->typeof(var_value, i) != var_type) {
        throw InvalidSyntax(i, ("cannot convert from " + scope->typeof(var_value, i) + " to " + var_type +
                    " in initialisation"));
    } else {
        output << "    ";
        if (var_type == "auto") {
            if (support::str::isnum(var_value)) {
                output << "istore";
            } else if (var_value.size() >= 2 and var_value[0] == '"' and var_value[var_value.size()-1] == '"') {
                output << "strstore";
            } else if (var_value.size() >= 2 and var_value[0] == '\'' and var_value[var_value.size()-1] == '\'') {
                output << "strstore";
            } else if (support::str::isbooleanliteral(var_value)) {
                var_type = "bool";
            } else {
                throw InvalidSyntax(offset, ("failed to determine type of auto variable " +
                            var_name + " in function " + scope->function->header() + ": " + var_value));
            }
        }
        if (var_type == "int") {
            output << "istore";
        } else if (var_type == "string") {
            output << "strstore";
        } else if (var_type == "float") {
            output << "fstore";
        }
        if (var_type == "bool") {
            if (var_value == "false" or var_value == "0") {
                output << "not (not (istore " << var_register << " 0))" << endl;
            } else if (var_value == "true" or var_value == "1") {
                output << "not (istore " << var_register << " 0)" << endl;
            } else {
                throw InvalidSyntax(offset, ("invalid boolean literal in initialisation of variable " +
                            var_name + " in function " + scope->function->header() + ": " + var_value));
            }
        } else {
            output << ' ' << var_register << ' ' << var_value << endl;
        }
    }

    scope->setregisterof(var_name, var_register);
    scope->settypeof(var_name, var_type);
    scope->setvalueof(var_name, var_value);

    return (i-offset);
}

TokenVectorSize processFrame(const TokenVector& tokens, const string& function_to_call, TokenVectorSize offset, Scope* scope, ostringstream& output) {
    TokenVectorSize i = offset;
    vector<unsigned> parameter_sources;

    if (not scope->isDeclaredFunction(function_to_call)) {
        throw InvalidSyntax(i, ("call to undefined function " + function_to_call));
    }

    if (tokens[i] == ")") {
        if (scope->getFunctionSignature(function_to_call).parameters.size() != 0) {
            throw InvalidSyntax(i, ("missing parameters in call to function " + scope->getFunctionSignature(function_to_call).header()));
        }
        output << "    frame 0" << endl;
        return 2; // number of processed tokens is 2: "(" and ";"
    }

    for (; i < tokens.size() and tokens[i] != ";"; ++i) {
        if (not scope->defined(tokens[i])) {
            ostringstream oss;
            oss << "undefined name as parameter: `" << tokens[i].text() << "` in call to function `";
            oss << function_to_call << "`" << "\n";
            oss << "note: parameters in scope:\n";
            vector<string> names = scope->names();
            for (unsigned ps = 0; ps < names.size(); ++ps) {
                oss << "    " << scope->typeof(names[ps], i) << ' ' << names[ps] << ';' << endl;
            }
            throw InvalidSyntax(i, oss.str());
        }

        if (parameter_sources.size() >= scope->getFunctionSignature(function_to_call).parameters.size()) {
            throw InvalidSyntax(i, ("too many parameters in call to function " + function_to_call));
        }

        string p_name = scope->getFunctionSignature(function_to_call).parameters[parameter_sources.size()];
        string p_type = scope->getFunctionSignature(function_to_call).parameter_types.at(p_name);
        if (p_type != "auto" and p_type != scope->typeof(tokens[i], i)) {
            throw InvalidSyntax(i, ("invalid type for parameter " + p_name + " expected " + p_type + " but got " + scope->typeof(tokens[i], i)));
        }

        parameter_sources.push_back(scope->registerof(tokens[i], i));
        // account for both "," between parameters and
        // closing ")"
        ++i;
    }

    if (scope->getFunctionSignature(function_to_call).parameters.size() != parameter_sources.size()) {
        throw InvalidSyntax(i, ("missing parameters in call to function " + scope->getFunctionSignature(function_to_call).header()));
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

TokenVectorSize processCall(const TokenVector& tokens, TokenVectorSize offset, Scope* scope, ostringstream& output) {
    string function_to_call = tokens[offset++];
    // skip opening "("
    ++offset;
    TokenVectorSize i = processFrame(tokens, function_to_call, offset, scope, output);
    output << "    call 0 " << function_to_call << endl;

    return i;
}
TokenVectorSize processCallWithReturnValueUsed(const TokenVector& tokens, TokenVectorSize offset, Scope* scope, ostringstream& output) {
    string return_to = tokens[offset++];

    // skip "="
    ++offset;

    string function_to_call = tokens[offset++];

    // skip opening "("
    ++offset;

    if (scope->typeof(return_to, offset-4) != scope->function->env->functions.at(function_to_call)) {
        throw InvalidSyntax(offset, (
                    "mismatched type of return target variable " + return_to + " of type " + scope->typeof(return_to, offset-4) + " and return type of function " + scope->getFunctionSignature(function_to_call).header()));
    }

    TokenVectorSize i = (processFrame(tokens, function_to_call, offset, scope, output) + 4);
    output << "    call " << scope->registerof(return_to, (offset-4)) << ' ' << function_to_call << endl;

    return i;
}

TokenVectorSize processBlock(const TokenVector& tokens, TokenVectorSize offset, Scope* scope, ostringstream& output);

TokenVectorSize processIfStatement(const TokenVector& tokens, TokenVectorSize offset, Scope* scope, ostringstream& output) {
    TokenVectorSize i = offset;

    if (not support::str::isname(tokens[i])) {
        throw InvalidSyntax(i, ("unexpected token in condition experssion: " + support::str::strencode(tokens[i].text())));
    }
    if (not scope->defined(tokens[i])) {
        throw InvalidSyntax(i, ("undeclared variable in condition experssion: " + support::str::strencode(tokens[i].text())));
    }

    string if_test_variable_name = tokens[i++];
    string false_branch_name = ("__" + scope->function->function_name + "_if_" + support::str::stringify(scope->function->ifs++));

    if (tokens[i] != "{") {
        throw InvalidSyntax(i, ("missing opening '{' in if-statement in function " + scope->function->header()));
    }
    scope->function->begin_balance += 1;

    output << "    branch " << scope->registerof(if_test_variable_name, i) << ' ';
    output << "+1 " << false_branch_name << '\n';

    // skip opening "{"
    ++i;

    // FIXME: memory leaks on exceptions thrown
    // this is not severe as when an exception is thrown the only course of action is to
    // terminate the program since the comiler cannot recover from invalid source code
    Scope *block_scope = new Scope(scope->function, scope);
    i += processBlock(tokens, i, block_scope, output);
    delete block_scope;

    output << "    .mark: " << false_branch_name << '\n';

    return (i - offset);
}

TokenVectorSize processWhileStatement(const TokenVector& tokens, TokenVectorSize offset, Scope* scope, ostringstream& output) {
    TokenVectorSize i = offset;

    if (not support::str::isname(tokens[i])) {
        throw InvalidSyntax(i, ("unexpected token in condition experssion: " + tokens[i].text()));
    }
    if (not scope->defined(tokens[i])) {
        throw InvalidSyntax(i, ("undeclared variable in condition experssion: " + tokens[i].text()));
    }

    string if_test_variable_name = tokens[i++];
    string loop_name_begin = ("__" + scope->function->function_name + "_begin_while_" + support::str::stringify(scope->function->whiles++));
    string loop_name_end = ("__" + scope->function->function_name + "_end_while_" + support::str::stringify(scope->function->whiles));

    string prev_loop_begin = scope->function->loop_begin;
    string prev_loop_end = scope->function->loop_end;
    scope->function->loop_begin = loop_name_begin;
    scope->function->loop_end = loop_name_end;

    if (tokens[i] != "{") {
        throw InvalidSyntax(i, ("missing opening '{' in while-statement in function " + scope->function->header()));
    }
    scope->function->begin_balance += 1;

    output << "    .mark: " << loop_name_begin << '\n';
    output << "    branch " << scope->registerof(if_test_variable_name, i) << ' ';
    output << "+1 " << loop_name_end << '\n';

    // skip opening "{"
    ++i;

    // FIXME: memory leaks on exceptions thrown
    // this is not severe as when an exception is thrown the only course of action is to
    // terminate the program since the comiler cannot recover from invalid source code
    Scope* block_scope = new Scope(scope->function, scope);
    i += processBlock(tokens, i, block_scope, output);
    delete block_scope;

    output << "    jump " << loop_name_begin << '\n';
    output << "    .mark: " << loop_name_end << '\n';

    scope->function->loop_begin = prev_loop_begin;
    scope->function->loop_end = prev_loop_end;

    return (i - offset);
}

TokenVectorSize processFunction(const TokenVector& tokens, TokenVectorSize offset, CompilationEnvironment& cenv, ostringstream& output, const string& namespace_prefix = "") {
    TokenVectorSize number_of_processed_tokens = 0;

    string name = tokens[offset + (number_of_processed_tokens++)];
    if (namespace_prefix.size()) {
        name = (namespace_prefix + "::" + name);
    }
    FunctionEnvironment fenv(name, &cenv);
    Scope* scope = fenv.scope;

    if ((offset+number_of_processed_tokens+2) >= tokens.size() or tokens[offset+number_of_processed_tokens] !=  "(") {
        throw InvalidSyntax((offset+number_of_processed_tokens), ("missing parameter list in definition of function " + fenv.header()));
    }


    // skip opening "("
    ++number_of_processed_tokens;

    decltype(offset) i = offset+number_of_processed_tokens;
    string param_name, param_type;
    for (; i < tokens.size() and tokens[i] != ")"; ++i) {
        param_type = tokens[i++];
        if (not scope->isRegisteredClass(param_type)) {
            throw InvalidSyntax(i-1, ("invalid parameter type in function " + scope->function->header() + ": " + param_type));
        }

        param_name = tokens[i];
        if (not support::str::isname(param_name)) {
            throw InvalidSyntax(i, ("invalid parameter name in function " + scope->function->header() + ": " + param_name));
        }

        fenv.parameters.push_back(param_name);
        fenv.parameter_types[param_name] = param_type;
        if (tokens[i+1] == ",") {
            ++i;
        } else if (tokens[i+1] == ")") {
            // explicitly do nothing
        } else {
            throw InvalidSyntax(i, ("unexpected token in parameters list of function " + fenv.function_name + ": " + tokens[i].text()));
        }
        number_of_processed_tokens += 3;
    }

    // this if is in case the function has no parameters and
    // must be here because the for above wasn't entered
    if (tokens[offset+number_of_processed_tokens] == ")") {
        ++number_of_processed_tokens;
    }

    if ((offset+number_of_processed_tokens+2) >= tokens.size() and tokens[offset+number_of_processed_tokens] != "{") {
        throw InvalidSyntax((offset+number_of_processed_tokens), ("unexpected end of token stream in definition of function " + fenv.function_name));
    }
    if ((tokens[offset+number_of_processed_tokens] != "-" or tokens[offset+number_of_processed_tokens+1] != ">") and tokens[offset+number_of_processed_tokens] != "{" and tokens[offset+number_of_processed_tokens] != ";") {
        throw InvalidSyntax(
                (offset+number_of_processed_tokens),
                ("missing return type specifier in definition of function " + fenv.function_name +
                 ", expected '->' but got: '" +
                 support::str::strencode(tokens[offset+number_of_processed_tokens]) +
                 support::str::strencode(tokens[offset+number_of_processed_tokens+1]) +
                 "'\ntwo previous tokens: " +
                 support::str::strencode(tokens[offset+number_of_processed_tokens-2]) +
                 support::str::strencode(tokens[offset+number_of_processed_tokens-1]) +
                 ""
                 ));
    }

    if (tokens[offset+number_of_processed_tokens] == "-") {
        // skip over "-" and ">" that make up return type specifier
        number_of_processed_tokens += 2;
        fenv.return_type = tokens[offset + (number_of_processed_tokens++)];
        if (fenv.return_type != "void" and fenv.return_type != "string" and fenv.return_type != "int" and fenv.return_type != "float" and fenv.return_type != "bool") {
            throw InvalidSyntax((offset+number_of_processed_tokens), ("invalid return type in definition of function " + fenv.function_name));
        }
    } else {
        fenv.return_type = "void";
    }

    cenv.functions[fenv.function_name] = fenv.return_type;
    cenv.signatures[fenv.function_name] = FunctionSignature(fenv.function_name, fenv.return_type);
    cenv.signatures[fenv.function_name].parameters = fenv.parameters;
    cenv.signatures[fenv.function_name].parameter_types = fenv.parameter_types;

    if (tokens[offset+number_of_processed_tokens] == ";") {
        return ++number_of_processed_tokens;
    }

    if (tokens[offset+number_of_processed_tokens] != "{") {
        throw InvalidSyntax((offset+number_of_processed_tokens), ("missing opening '{' in definition of function " + fenv.header()));
    }

    // skip opening "{"
    ++number_of_processed_tokens;
    ++fenv.begin_balance;

    output << ".function: " << fenv.function_name << endl;

    for (decltype(FunctionEnvironment::parameters)::size_type i = 0; i < fenv.parameters.size(); ++i) {
        output << "    .name: " << i+1 << ' ' << fenv.parameters[i] << endl;
        output << "    arg " << i+1 << ' ' << i << endl;
        scope->setregisterof(fenv.parameters[i], i+1);
        scope->settypeof(fenv.parameters[i], fenv.parameter_types[fenv.parameters[i]]);
    }

    number_of_processed_tokens += processBlock(tokens, (offset+number_of_processed_tokens), scope, output);

    if (not fenv.has_returned) {
        output << "    end" << endl;
    }
    if (not fenv.has_returned and fenv.return_type != "void") {
        throw InvalidSyntax(i, ("function " + fenv.header() + " declared return type " + fenv.return_type + " but reached end of definition without return statement"));
    }
    output << ".end" << endl;

    return number_of_processed_tokens;
}

TokenVectorSize processNamespace(const TokenVector& tokens, TokenVectorSize offset, CompilationEnvironment& cenv, ostringstream& output) {
    TokenVectorSize number_of_processed_tokens = offset;

    string name = tokens[number_of_processed_tokens++];

    // skip {} for now
    number_of_processed_tokens += 2;

    return (number_of_processed_tokens-offset);
}

TokenVectorSize processBlock(const TokenVector& tokens, TokenVectorSize offset, Scope* scope, ostringstream& output) {
    TokenVectorSize number_of_processed_tokens = 0;

    for (; number_of_processed_tokens+offset < tokens.size() and scope->function->begin_balance; ++number_of_processed_tokens) {
        if (tokens[offset+number_of_processed_tokens] == "var") {
            number_of_processed_tokens += processVariable(tokens, (offset + (++number_of_processed_tokens)), scope, output);
        } else if (tokens[offset+number_of_processed_tokens] == "return") {
            scope->function->has_returned = true;

            // skip "return" keyword
            ++number_of_processed_tokens;

            // this if deals with `return <token> ;` case
            if (tokens[offset + number_of_processed_tokens] != ";") {
                // this if deals with `return <number> ;` case
                if (support::str::isnum(tokens[offset+number_of_processed_tokens])) {
                    if (scope->function->return_type != "int") {
                        throw InvalidSyntax((offset+number_of_processed_tokens), ("mismatched return type in function " + scope->function->header() + ", expected " + scope->function->return_type + " but got int"));
                    }
                    if (tokens[offset+number_of_processed_tokens] == "0") {
                        output << "    izero 0" << endl;
                    } else {
                        output << "    istore 0 " << tokens[offset+number_of_processed_tokens].text() << endl;
                    }
                } else if (scope->registerof(tokens[offset+number_of_processed_tokens], offset+number_of_processed_tokens) != 0) {
                    if (scope->function->return_type != scope->typeof(tokens[offset+number_of_processed_tokens], offset+number_of_processed_tokens)) {
                        throw InvalidSyntax((offset+number_of_processed_tokens), ("mismatched return type in function " + scope->function->header() + ", expected " + scope->function->return_type + " but got " + scope->typeof(tokens[offset+number_of_processed_tokens], offset+number_of_processed_tokens)));
                    }
                    output << "    move 0 " << scope->registerof(tokens[offset+number_of_processed_tokens], offset+number_of_processed_tokens) << endl;
                }

                // advance after the returned <token>
                ++number_of_processed_tokens;
            } else {
                if (scope->function->return_type != "void") {
                    throw InvalidSyntax((offset+number_of_processed_tokens), ("mismatched return type in function " + scope->function->header() + ", expected " + scope->function->return_type + " but got void"));
                }
            }

            // no need to deal with terminating ";" as loop increment will take care of it
            output << "    end" << endl;
        } else if (tokens[offset+number_of_processed_tokens] == "asm") {
            output << "    ";
            while (tokens[offset + (++number_of_processed_tokens)] != ";") {
                output << tokens[offset+number_of_processed_tokens].text() << ' ';
            }
            output << endl;
        } else if (tokens[offset+number_of_processed_tokens] == "\n") {
            continue;
        } else if (tokens[offset+number_of_processed_tokens] == ";") {
            continue;
        } else if (tokens[offset+number_of_processed_tokens] == "{") {
            scope->function->begin_balance += 1;

            Scope* block_scope = new Scope(scope->function, scope);
            number_of_processed_tokens += processBlock(tokens, (offset + (++number_of_processed_tokens)), block_scope, output);
            delete block_scope;

            // skip closing "}" for nested blocks
            ++number_of_processed_tokens;
        } else if (tokens[offset+number_of_processed_tokens] == "}") {
            scope->function->begin_balance -= 1;
            break;
        } else if (tokens[offset+number_of_processed_tokens] == "break") {
            if (scope->function->loop_end == "") {
                throw InvalidSyntax((offset+number_of_processed_tokens), ("break outside of loop inside function " + scope->function->header()));
            }
            output << "    ; from break instruction" << endl;
            output << "    jump " << scope->function->loop_end << '\n';
        } else if (tokens[offset+number_of_processed_tokens] == "if") {
            number_of_processed_tokens += processIfStatement(tokens, (offset + (++number_of_processed_tokens)), scope, output);
        } else if (tokens[offset+number_of_processed_tokens] == "while") {
            number_of_processed_tokens += processWhileStatement(tokens, (offset + (++number_of_processed_tokens)), scope, output);
        } else {
            if ((offset+number_of_processed_tokens+3) >= tokens.size()) {
                throw InvalidSyntax(
                        (offset+number_of_processed_tokens),
                        ("missing tokens during call to " + tokens[offset+number_of_processed_tokens].text())
                      );
            } else if (tokens[offset+number_of_processed_tokens+1] == "(") {
                number_of_processed_tokens += processCall(tokens, (offset + number_of_processed_tokens), scope, output);
            } else if (scope->defined(tokens[offset+number_of_processed_tokens]) and tokens[offset+number_of_processed_tokens+1] == "=" and tokens[offset+number_of_processed_tokens+3] == "(") {
                number_of_processed_tokens += processCallWithReturnValueUsed(tokens, (offset+number_of_processed_tokens), scope, output);
            } else {
                throw InvalidSyntax((offset+number_of_processed_tokens),
                        ("unexpected token: " + support::str::strencode(tokens[offset+number_of_processed_tokens])));
            }
        }
    }

    return number_of_processed_tokens;
}

void processSource(const TokenVector& tokens, ostringstream& output) {
    string previous_token = "", token = "";

    CompilationEnvironment cenv;

    for (vector<string>::size_type i = 0; i < tokens.size(); ++i) {
        token = tokens[i];
        if (token == "function") {
            i += processFunction(tokens, ++i, cenv, output);
        } else if(token == "\n") {
            // explicitly do nothing
        } else if (token == "namespace") {
            i += processNamespace(tokens, ++i, cenv, output);
        } else {
            throw InvalidSyntax(i, ("invalid top-level token: " + support::str::strencode(tokens[i].text())));
        }
        previous_token = token;
    }

    if (cenv.signatures.count("main") == 0) {
        cout << "warning: main()->int function was not defined" << endl;
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

    auto primitive_toks = support::str::lex(source_text);
    auto toks = reduceNamespacedNames(reduceNamespaceResolutionOperator(reduceFloats(reduceIntegers(removeComments(primitive_toks)))));

    ostringstream out;
    try {
        processSource(toks, out);
        ofstream compile_output(compilename);
        compile_output << out.str();
    } catch (const InvalidSyntax& e) {
        cout << compilename << ':' << toks[e.tokenIndex()].line()+1 << ':' << toks[e.tokenIndex()].character()+1 << ": " << e.what() << endl;

        cout << "note: source context: " << compilename << ':' << toks[e.tokenIndex()].line()+1 << endl;
        istringstream in(source_text);
        string line;
        int i = 0, tline = toks[e.tokenIndex()].line();
        while (getline(in, line)) {
            if (i >= tline-1 and i <= tline+1) {
                cout << ((i == tline) ? "->  " : "    ") << line << endl;
            }
            if (i == tline+1) {
                break;
            }
            ++i;
        }
        return 1;
    }

    return 0;
}
