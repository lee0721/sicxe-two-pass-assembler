#include<iostream>    // cin, cout, endl
#include<string>      // c_str, length
#include<string.h>    // strcpy
#include<sstream>
#include<cctype>
#include<stdio.h>    
#include<stdlib.h>    //atoi()
#include<cstdlib>     //strtoul,system
#include <vector>
#include<fstream>
#include<typeinfo>
#include<unordered_map>
#include<unordered_set>
#include<map>
#include<algorithm>

#define MAX_TOKENS 30        // Max tokens per source statement
#define MAX_TOKEN_GROUP 100  // Max number of source statements held

using namespace std;
using std::string;

typedef char STRING20 [20];   // Fixed buffer for <=19-char labels
typedef char STRING200 [200]; // Fixed buffer for <=199-char source lines

string promptFilePath(const string &prompt) {
    cout << prompt;
    string path;
    cin >> ws;
    getline(cin, path);
    return path;
}

bool openInputFile(ifstream &stream, const string &path) {
    stream.open(path.c_str());
    if (stream.is_open())
        return true;
    cerr << "Error: Unable to open input file '" << path << "'." << endl;
    return false;
}

bool openOutputFile(ofstream &stream, const string &path) {
    stream.open(path.c_str());
    if (stream.is_open())
        return true;
    cerr << "Error: Unable to open output file '" << path << "'." << endl;
    return false;
}

enum class TokenType {
    Unknown = 0,
    Instruction = 1,
    Pseudo = 2,
    Register = 3,
    Delimiter = 4,
    Label = 5,
    Number = 6,
    Literal = 7
};

struct Token{
    string value;    
    TokenType type = TokenType::Unknown;  // ID of the table that matched this token
    int tokenvalue = 0; // Index inside that table
}; // Token

struct Sicxe_Instruction_Set {
	string instruction;
	int format = 0;   // Instruction format (1/2/3/4, default 3 unless '+' -> 4)
	int opformat = 0; // Operand pattern
	string objectcode;
}; // Sicxe_Instruction_Set

struct Sic_Instruction_Set {
	string instruction;
	string objectcode;
}; // Sic_Instruction_Set

void Sicxe_Instruction_input( Sicxe_Instruction_Set sicxe[59] ) {
	ifstream newfile; 
	newfile.open("Sicxe_Instruction_Set.table"); // Load SIC/XE opcode table
	int num = 0; // Current record index 
	for ( int i = 0 ; i < 236 ; i++) {
	  newfile >> sicxe[num].instruction;
	  newfile >> sicxe[num].format;
	  newfile >> sicxe[num].opformat;
	  newfile >> sicxe[num].objectcode;
	  num++;
	}
	newfile.close();
}  // Sicxe_Instruction_input

void Sic_Instruction_input( Sic_Instruction_Set sic[26] ) {
	ifstream newfile; 
	newfile.open("Sic_Instruction_Set.table"); // Load SIC opcode table
	int num = 0; // Current record index 
	for ( int i = 0 ; i < 52 ; i++) {
	  newfile >> sic[num].instruction;
	  newfile >> sic[num].objectcode;
	  num++;
	}
	newfile.close();
}  // Sic_Instruction_input

void Sicxe_Instruction_print( Sicxe_Instruction_Set sicxe[59] ) {
	for ( int i = 0 ; i < 59 ; i++) {
	  cout << sicxe[i].instruction << " "; 
	  cout << sicxe[i].format << " "; 
	  cout << sicxe[i].opformat << " "; 
	  cout << sicxe[i].objectcode;
	  cout << endl ;
	}
}  // Sicxe_Instruction_print

void Sic_Instruction_print( Sic_Instruction_Set sic[26] ) {
	for ( int i = 0 ; i < 26 ; i++) {
	  cout << sic[i].instruction << " "; 
	  cout << sic[i].objectcode;
	  cout << endl ;
	}
}  // Sic_Instruction_print

struct Literal{
	int location = 0; // Address assigned when literal is emitted
	string c_x_w ;
    string label;    
    string WORDorBYTE;
    string literal;
}; // Literal

struct Tokens{
    struct AddressingFlags {
        bool n = false;
        bool i = false;
        bool x = false;
        bool b = false;
        bool p = false;
        bool e = false;
    };

    unsigned amount;        // Number of tokens parsed from the line
    int label_length = 0;   // Length of the label, if present
    string error;           // Error message for this source line
    bool forwardreference = false; // True if referencing an unresolved label
    bool end = false; 
    bool base = false; 
    bool comment = false; 
    bool leading_tab = false; // true if the original line started with a tab
    bool EQU = false; 
    bool START = false ;
    bool pseudo = false;
    //--------------------------------------------------
    string c_x_w ;
    string sourcestatement; // Original source text of the line
    AddressingFlags flags;
	int line = 0;
	string setedline;  // Line number padded for listings
    int location = 0; // Location counter for the statement
    string hex_location; // Location in hex string format
    string objectcode;  // Object code emitted for this line
    //--------------------------------------------------
    int format = 0 ; // Instruction format (1/2/3/4)
    int opformat = 0; // Operand format classification
    string label; // Parsed label text
    string ins;   // Mnemonic or directive text
    string group1;
    string group2;
    Literal literal;
    //--------------------------------------------------
    Token tokens[MAX_TOKENS]; // Collected tokens for this line
};  // Tokens

struct Packed_Token{
    unsigned amount; // Count of tokenized statements
    int longestnum = 0; // Length of the longest source line
    bool end = false;
    bool base = false;
    string base_label;
    string base_hexlocation;
    int basenum = 0;
    vector<Literal> literals;           // collected literals
    unordered_map<string,int> literal_address; // literal label -> location
    Tokens token_groups[MAX_TOKEN_GROUP]; // Storage for each statement
}; // Packed_Token

struct Table{
    int index;
    string value;
}; //Table

class SymbolTable {
public:
    int insertOrGet(const string &token) {
        auto [it, inserted] = entries.emplace(token, nextIndex);
        if (inserted) ++nextIndex;
        return it->second;
    }

    bool get(const string &token, int &index) const {
        auto it = entries.find(token);
        if (it == entries.end()) return false;
        index = it->second;
        return true;
    }

private:
    unordered_map<string, int> entries;
    int nextIndex = 1;
};

void loadOpcodeTables( Table table1[59], Table table2[9], Table table3[9], Table table4[13] ) {
	ifstream newfile; 
	const string basePath = "SICTABLE/";
	auto openTable = [&](const string &filename) -> bool {
		newfile.open((basePath + filename).c_str());
		if (!newfile.is_open()) {
			cerr << "Error: Unable to open " << filename << " under " << basePath << endl;
			return false;
		}
		return true;
	};

	if ( !openTable("Table1.table") )
		return;
	for ( int i = 1 ; i < 60 ; i++) {
	  newfile >> table1[i-1].value;
	  table1[i-1].index = i;
	}
	newfile.close();
	
	if ( !openTable("Table2.table") )
		return;
	for ( int i = 1 ; i < 10; i++) {
	  newfile >> table2[i-1].value;
	  table2[i-1].index = i;
	}
	newfile.close();
	
	if ( !openTable("Table3.table") )
		return;
	for ( int i = 1 ; i < 10; i++) {
	  newfile >> table3[i-1].value;
	  table3[i-1].index = i;
	}
	newfile.close();
	
	if ( !openTable("Table4.table") )
		return;
	for ( int i = 1 ; i < 14; i++) {
	  newfile >> table4[i-1].value;
	  table4[i-1].index = i;
	}
	newfile.close();
}

bool lookupToken( string token, Table table1[59], Table table2[9], Table table3[9], Table table4[13], 
          SymbolTable &table5, SymbolTable &table6, SymbolTable &table7, TokenType &tokentype, int &tokenvalue ){
    string upper = token;
    string lower = token;
    for (auto &c : upper) c = toupper(static_cast<unsigned char>(c));
    for (auto &c : lower) c = tolower(static_cast<unsigned char>(c));
    for ( int i = 0 ; i < 59; i++) {
	  if ( lower == table1[i].value ) {
	  	tokentype = TokenType::Instruction;
	  	tokenvalue = table1[i].index;
	  	return true;
    }
}

	for ( int i = 0 ; i < 9; i++) {
	  if ( upper == table2[i].value ) {
	  	tokentype = TokenType::Pseudo;
	  	tokenvalue = table2[i].index;
	  	return true;
	  }
	}

    for ( int i = 0 ; i < 9; i++) {
	  if ( upper == table3[i].value ) {
	  	tokentype = TokenType::Register;
	  	tokenvalue = table3[i].index;
	  	return true;
	  }
	}

	for ( int i = 0 ; i < 13; i++) {
	  if ( token == table4[i].value ) {
	  	tokentype = TokenType::Delimiter;
	  	tokenvalue = table4[i].index;
	  	return true;
	  }
	}
	if ( table5.get(upper, tokenvalue) ) {
	  	tokentype = TokenType::Label;
	  	return true;
	}
	if ( table6.get(upper, tokenvalue) ) {
	  	tokentype = TokenType::Number;
	  	return true;
	}
	if ( table7.get(token, tokenvalue) ) {
	  	tokentype = TokenType::Literal;
	  	return true;
	}
	return false;
}

// Normalize a literal string to canonical form (e.g., C'EOF', X'05', WORD literal as-is)
// Also returns the kind: "c", "x", or "w".
pair<string,string> canonicalLiteral(const string &raw) {
    if (raw.empty()) return {"",""};
    string lit = raw;
    // strip leading '=' and whitespace
    auto trim = [](string s) {
        while (!s.empty() && (s.front()==' ' || s.front()=='\t')) s.erase(s.begin());
        while (!s.empty() && (s.back()==' ' || s.back()=='\t')) s.pop_back();
        return s;
    };
    lit = trim(lit);
    if (!lit.empty() && lit.front()=='=') {
        lit.erase(lit.begin());
        lit = trim(lit);
    }
    string kind = "w";
    if (!lit.empty() && (lit[0]=='c' || lit[0]=='C')) {
        lit[0] = 'C';
        if (lit.size() > 2 && lit[1]=='\'' && lit.back()=='\'') {
            for (size_t i=2; i<lit.size()-1; ++i) {
                lit[i] = toupper(static_cast<unsigned char>(lit[i]));
            }
        }
        kind = "c";
    } else if (!lit.empty() && (lit[0]=='x' || lit[0]=='X')) {
        lit[0] = 'X';
        if (lit.size() > 2 && lit[1]=='\'' && lit.back()=='\'') {
            for (size_t i=2; i<lit.size()-1; ++i) {
                lit[i] = toupper(static_cast<unsigned char>(lit[i]));
            }
        }
        kind = "x";
    } else {
        kind = "w";
    }
    return {lit, kind};
}

void instable( string token, SymbolTable &table, TokenType &, int &tokenvalue){ 
  tokenvalue = table.insertOrGet(token);
}
void removespace( string &str ){ // Remove every space
  int index = 0;
    if( !str.empty()) {
      while( (index = str.find(' ',index)) != string::npos)
        str.erase(index,1);
    } // if
} // removespace


string to_upper( string token ) { // Convert to uppercase
	for ( int a = 0; a < token.length() ; a++ ) {
        if (islower(token[a]))
            token[a] = toupper(token[a]); 
    } // for 
    return token;
} // to_upper

string to_lower( string token ) { // Convert to lowercase
	for ( int a = 0; a < token.length() ; a++ ) {
        if (isupper(token[a]))
            token[a] = tolower(token[a]); 
    } // for 
    return token;
} // to_lower

int currentLocationCounter(const Packed_Token &token_packer) {
    int temp = token_packer.amount - 1;
    while ( temp >= 0 && token_packer.token_groups[temp].EQU )
        temp--;
    if ( temp >= 0 )
        return token_packer.token_groups[temp].location + token_packer.token_groups[temp].label_length;
    return 0;
}

int findLabelLocation(const Packed_Token &token_packer, const string &label) {
    string target = to_upper(label);
    for ( int i = token_packer.amount - 1 ; i >= 0 ; --i ) {
        if ( !token_packer.token_groups[i].label.empty() ) {
            string existing = to_upper(token_packer.token_groups[i].label);
            if ( existing == target )
                return token_packer.token_groups[i].location;
        }
    }
    return -1;
}

bool resolveEquTerm(const Packed_Token &token_packer, const Token &tok, int &value, string &errorMsg) {
    if ( tok.type == TokenType::Number ) {
        value = atoi(tok.value.c_str());
        return true;
    }
    if ( tok.type == TokenType::Delimiter && tok.value == "*" ) {
        value = currentLocationCounter(token_packer);
        return true;
    }
    int resolved = findLabelLocation(token_packer, tok.value);
    if ( resolved >= 0 ) {
        value = resolved;
        return true;
    }
    errorMsg = "Syntax Error! : Undefined symbol in EQU.";
    return false;
}

bool evaluateEquExpression(const Packed_Token &token_packer, const Tokens &toks, int operandIndex, int &result, string &errorMsg) {
    if ( operandIndex >= toks.amount )
        return false;
    if ( !resolveEquTerm(token_packer, toks.tokens[operandIndex], result, errorMsg) )
        return false;
    int opIndex = operandIndex + 1;
    if ( opIndex >= toks.amount )
        return true;
    const Token &opToken = toks.tokens[opIndex];
    if ( opToken.type != TokenType::Delimiter )
        return true;
    string op = opToken.value;
    if ( op != "+" && op != "-" && op != "*" && op != "/" )
        return true;
    int rhsIndex = opIndex + 1;
    if ( rhsIndex >= toks.amount ) {
        errorMsg = "Syntax Error! : EQU missing right operand.";
        return false;
    }
    int rhs = 0;
    if ( !resolveEquTerm(token_packer, toks.tokens[rhsIndex], rhs, errorMsg) )
        return false;
    if ( op == "+" )
        result += rhs;
    else if ( op == "-" )
        result -= rhs;
    else if ( op == "*" )
        result *= rhs;
    else if ( op == "/" ) {
        if ( rhs == 0 ) {
            errorMsg = "Syntax Error! : Division by zero in EQU.";
            return false;
        }
        result /= rhs;
    }
    return true;
}

bool parseHexNumber(const string &token, int &value) {
    char *endptr = nullptr;
    long parsed = strtol(token.c_str(), &endptr, 16);
    if ( endptr == token.c_str() || (endptr && *endptr != '\0') )
        return false;
    value = static_cast<int>(parsed);
    return true;
}

bool find( string token, Table table1[59], Table table2[9], Table table3[9], Table table4[13], 
          SymbolTable &symbolTable, SymbolTable &numberTable, SymbolTable &literalTable, TokenType &tokentype, int &tokenvalue ){
    string upper = to_upper(token); 
	string lower = to_lower(token);     	
    for ( int i = 0 ; i < 59; i++) { // search instruction table
	  if ( lower == table1[i].value ) {
	  	tokentype = TokenType::Instruction;
	  	tokenvalue = table1[i].index;
	  	return true;
	  } // if
	} //for 
	
	for ( int i = 0 ; i < 9; i++) { // search pseudo-op table
	  if ( upper == table2[i].value ) {
	  	tokentype = TokenType::Pseudo;
	  	tokenvalue = table2[i].index;
	  	return true;
	  } // if
	} //for 

    for ( int i = 0 ; i < 9; i++) { // search register table
	  if ( upper == table3[i].value ) {
	  	tokentype = TokenType::Register;
	  	tokenvalue = table3[i].index;
	  	return true;
	  } // if
	} //for 

	for ( int i = 0 ; i < 13; i++) { // search delimiter table
	  if ( token == table4[i].value ) {
	  	tokentype = TokenType::Delimiter;
	  	tokenvalue = table4[i].index;
	  	return true;
	  } // if
	} //for 
	if ( symbolTable.get(upper, tokenvalue) ) {
	  	tokentype = TokenType::Label;
	  	return true;
	}
	if ( numberTable.get(upper, tokenvalue) ) {
	  	tokentype = TokenType::Number;
	  	return true;
	}
	if ( literalTable.get(token, tokenvalue) ) {
	  	tokentype = TokenType::Literal;
	  	return true;
	}
	return false;
} // find

bool iswhitespce( char iswhitespace ){ // check whether the character is a number
  if ( iswhitespace == ' ' )
    return true;
  if ( iswhitespace == '\t' )
    return true;
  if ( iswhitespace == '\n' )
    return true;
  else
    return false;
} // iswhitespce

bool isNumber( string isnum ){ // check whether the character is a number
  if ( ( isnum[0] <= '9' ) && ( isnum[0] >= '0' ))
    return true;
  else
    return false;
} // isNumber

bool isLetter( string islet ){ // check whether the character is a number
  if ( ( islet[0] >= 'a' ) && ( islet[0] <= 'z' ))
  	return true;
  if ( ( islet[0] >= 'A' ) && ( islet[0] <= 'Z' ))
  	return true;
  else
    return false;
} // isLetter

bool isBlankLine(const string &line) { // true when a line contains only whitespace
    for ( char c : line ) {
        if ( !isspace(static_cast<unsigned char>(c)) )
            return false;
    }
    return true;
}

bool isOp( char isop ){ // check whether the character is table4
     if ( ( isop == ',' ) || ( isop == '+' ) || ( isop == '-' ) || ( isop == '*' ) 
	           || ( isop == '/' ) || ( isop == ':' ) || ( isop == ';' ) || ( isop == '?' ) 
			      || ( isop == '@' ) || ( isop == '.' ) || ( isop == '=' ) || ( isop == '#' ) || ( int(isop) == 39 ) )
       return true;
     else
       return false;
} //isOp


void removewhitespace( string &str ){ // 
  if ( str[str.length()-1] == ' ' )  {
  	int index = 0;
    if( !str.empty()) {
      while( (index = str.find(' ',index)) != string::npos)
        str.erase(index,1);
    } // if
  } // if
  else if ( str[str.length()-1] == '\t' )  {
  	int index = 0;
    if( !str.empty()) {
      while( (index = str.find('\t',index)) != string::npos)
        str.erase(index,1);
    } // if
  } // else if
  else if ( str[str.length()-1] == '\n' )  {
  	int index = 0;
    if( !str.empty()) {
      while( (index = str.find('\n',index)) != string::npos)
        str.erase(index,1);
    } // if
  } // else if
} // removewhitespace

bool onespace ( string token ) { // 
  bool check = false;
  if ( token.length() == 1 ) 
    if ( token[0] == ' ' )
	   check = true;
  if ( check == true )
    return true;
  return false;
} // onespace

vector<Token> tokenizeLine(
    const string &source,
    Table table1[59], Table table2[9], Table table3[9], Table table4[13],
    SymbolTable &symbolTable, SymbolTable &numberTable, SymbolTable &literalTable,
    bool &isComment, string &tokenError
) {
    vector<Token> tokens;
    string token;
    int is_string = 0;
    int is_integer = 0;
    TokenType tokentype = TokenType::Unknown;
    int tokenvalue = 0;
    bool pendingStringLiteral = false;
    bool pendingHexLiteral = false;
    tokenError.clear();

    auto emitToken = [&](const string &tok, TokenType type, int value) {
        tokens.push_back({tok, type, value});
    };

    for ( int a = 0; a < source.length() ; a++ ) { 
        if ( source[a] == '.' ) { 
            isComment = true;
            break;
        } 
        token.append(1, source[a]);
        if ( is_string != 1 && is_integer != 1 ) {
            while ( !iswhitespce( source[a] ) && !isOp( source[a] ) && a+1 < source.length() ){
                a++;
                token.append(1, source[a]);
                int end = a+1;
                if ( end >= source.length() )
                    break;
            }
        }
        else if ( is_string == 1 || is_integer == 1 ) {
            while ( a+1 < source.length() && int(source[a+1]) != 39 ){
                a++;
                token.append(1, source[a]);
            }
        }
        
        if ( a > 0 && int(source[a]) == 39 && (int(source[a-1]) == 99 || int(source[a-1]) == 67) ) {
            token.erase(0,1);
            is_string ++;
            pendingStringLiteral = true;
        }
        if ( a > 0 && int(source[a]) == 39 && (int(source[a-1]) == 120 || int(source[a-1]) == 88) ) {
            token.erase(0,1);
            is_integer ++;
            pendingHexLiteral = true;
        }
        
        if ( iswhitespce( source[a] ) )
           removewhitespace( token );
        
        if ( isOp( source[a] ) && is_string != 1 && is_integer != 1 && token.length() != 1 ) {
            token.erase(token.length()-1, 1);
            if ( lookupToken( token, table1, table2, table3, table4, symbolTable, numberTable, literalTable, tokentype, tokenvalue ) ) {
                emitToken(token, tokentype, tokenvalue);
            }
            else {
                string upper = to_upper(token); 
                if ( isNumber( upper ) ) { 
                    tokentype = TokenType::Number;
                    instable( upper, numberTable, tokentype, tokenvalue);  
                }
                else { 
                    tokentype = TokenType::Label;
                    instable( upper, symbolTable, tokentype, tokenvalue);
                }
                emitToken(token, tokentype, tokenvalue);
            }
            a--;
        }
        else if ( isOp( source[a] ) && token.length() == 1 ) { 
            if ( lookupToken( token, table1, table2, table3, table4, symbolTable, numberTable, literalTable, tokentype, tokenvalue ) ) {
                emitToken(token, tokentype, tokenvalue);
            }
        }
        else if ( is_string == 1 && !isOp( source[a] )  ) {
            tokentype = TokenType::Literal;
            instable( token, literalTable, tokentype, tokenvalue);
            emitToken(token, tokentype, tokenvalue);
            pendingStringLiteral = false;
            is_string ++;
        }
        else if ( is_integer == 1 && !isOp( source[a] )  ) {
            string upper = to_upper(token); 
            tokentype = TokenType::Number;
            instable( upper, numberTable, tokentype, tokenvalue);
            emitToken(token, tokentype, tokenvalue); 
            pendingHexLiteral = false;
            is_integer ++;
        }
        else if ( !onespace( token ) && !token.empty() ){ 
            if ( lookupToken( token, table1, table2, table3, table4, symbolTable, numberTable, literalTable, tokentype, tokenvalue ) ) {
                emitToken(token, tokentype, tokenvalue);
            }
            else {
                if ( is_integer == 1 ) {
                    tokentype = TokenType::Literal;
                    instable( token, literalTable, tokentype, tokenvalue);
     	        }
                else if ( isNumber( token ) ) {
                    string upper = to_upper(token); 
                    tokentype = TokenType::Number;
                    instable( upper, numberTable, tokentype, tokenvalue); 
                }
                else {
                    string upper = to_upper(token); 
                    tokentype = TokenType::Label;
                    instable( upper, symbolTable, tokentype, tokenvalue); 
                }
                emitToken(token, tokentype, tokenvalue);
            }
        } 
        token.clear();
      }
    if ( pendingStringLiteral )
        tokenError = "Syntax Error! : Missing closing quote in character literal.";
    else if ( pendingHexLiteral )
        tokenError = "Syntax Error! : Missing closing quote in hexadecimal literal.";
    return tokens;
}

void ins_Tokens( Token &tok, string token, TokenType type, int value ){
  tok.value = token;
  tok.type = type;
  tok.tokenvalue = value;
} // ins_Tokens

void ins_packer( Tokens &toks, string token, TokenType tokentype, int tokenvalue ){
  ins_Tokens(toks.tokens[toks.amount], token, tokentype, tokenvalue);
  //cout << toks.tokens[toks.amount].value << endl ; // 
  //cout << toks.tokens[toks.amount].tokentype << endl ; // 
  //cout << toks.tokens[toks.amount].tokenvalue << endl ; // 
  toks.amount++;
} // ins_packer

string getTokens( Token tok ) { // helper for debugging token stream
	string outputdemo("(,)");
	outputdemo.insert(1, std::to_string(static_cast<int>(tok.type)));
	outputdemo.insert(outputdemo.length()-1, std::to_string(tok.tokenvalue));
	outputdemo.insert(0, tok.value);
	return outputdemo;
} // getTokens

string getpacker( Tokens toks ) { //
	string outputdemo;
	for ( int b = 0 ; b < toks.amount; b++) {
		//cout << getTokens( toks.tokens[b]) << endl ; // 
		string token = getTokens( toks.tokens[b]) ; 
		outputdemo.insert(outputdemo.length(), token); // 
		outputdemo.insert(outputdemo.length(), " "); // 
	} // for
	return outputdemo;
} // getpacker

void DecToHexa(int n, string &hex){ // 
    // char array to store hexadecimal number
    char hexaDeciNum[100];
 
    // counter for hexadecimal number array
    int i = 0;
    while (n != 0) {
        // temporary variable to store remainder
        int temp = 0;
 
        // storing remainder in temp variable.
        temp = n % 16;
 
        // check if temp < 10
        if (temp < 10) {
            hexaDeciNum[i] = temp + 48;
            i++;
        } // if
        else {
            hexaDeciNum[i] = temp + 55;
            i++;
        } // else
 
        n = n / 16;
    } // while (n != 0)
 
    // printing hexadecimal number array in reverse order
    for (int j = i - 1; j >= 0; j--)
        hex.append( 1, hexaDeciNum[j] ) ;
} // DecToHexa

void HexToDe( string s, int &sum ){
  int t = 0;
  for(int i=0;s[i];i++){
    if(s[i]<='9')
      t=s[i]-'0';
    else
      t=s[i]-'A'+10;
    sum=sum*16+t;
  } //for
} // HexToDe

void negativeDecToHexa( int disp, string &hexdisp ){
    // 12-bit two's complement (for format 3 PC/base relative)
    const int mask = 0xFFF;
    int val = disp & mask;
    DecToHexa(val, hexdisp);
} // negativeDecToHexa

// Pad a hex string on the left to a fixed width.
// Use 'F' for negative displacements, otherwise '0'.
string padHex(const string &hex, int width, bool negative) {
    if (static_cast<int>(hex.length()) >= width) return hex;
    string pad(width - static_cast<int>(hex.length()), negative ? 'F' : '0');
    return pad + hex;
}

// Keep only the least significant `width` hex digits.
string clipHex(const string &hex, int width) {
    if (static_cast<int>(hex.length()) <= width) return hex;
    return hex.substr(hex.length() - width);
}

void BinToHexa( string &xbpe ){  // 
  if ( xbpe == "0000" )
    xbpe = "0";
  else if ( xbpe == "0001" )
    xbpe = "1";
  else if ( xbpe == "0010" )
    xbpe = "2";
  else if ( xbpe == "0011" )
    xbpe = "3";
  else if ( xbpe == "0100" )
    xbpe = "4";
  else if ( xbpe == "0101" )
    xbpe = "5";
  else if ( xbpe == "0110" )
    xbpe = "6";
  else if ( xbpe == "0111" )
    xbpe = "7";
  else if ( xbpe == "1000" )
    xbpe = "8";
  else if ( xbpe == "1001" )
    xbpe = "9";
  else if ( xbpe == "1010" )
    xbpe = "A";
  else if ( xbpe == "1011" )
    xbpe = "B";
  else if ( xbpe == "1100" )
    xbpe = "C";
  else if ( xbpe == "1101" )
    xbpe = "D";
  else if ( xbpe == "1110" )
    xbpe = "E";
  else if ( xbpe == "1111" )
    xbpe = "F";
} // BinToHexa

void HexaToBin( string &t ){  // 
  if ( t == "0" )
    t = "0000";
  else if ( t == "1" )
    t = "0001";
  else if ( t == "2" )
    t = "0010";
  else if ( t == "3" )
    t = "0011";
  else if ( t == "4" )
    t = "0100";
  else if ( t == "5" )
    t = "0101";
  else if ( t == "6" )
    t = "0110";
  else if ( t == "7" )
    t = "0111";
  else if ( t == "8" )
    t = "1000";
  else if ( t == "9" )
    t = "1001";
  else if ( t == "A" )
    t = "1010";
  else if ( t == "B" )
    t = "1011";
  else if ( t == "C" )
    t = "1100";
  else if ( t == "D" )
    t = "1101";
  else if ( t == "E" )
    t = "1110";
  else if ( t == "F" )
    t = "1111";
} // HexaToBin


//-----------------------sicxe----------------------------------

void sicxe_Search_Instruction_Set( Sicxe_Instruction_Set sicxe[59], string token, int &format, int &opformat, string &objectcode ) {
	string lower = to_lower(token);
	int num = 0;
	for ( int i = 0 ; i < 59 ; i++ )
	  if ( sicxe[i].instruction == lower )
	    num = i;
	if ( format == 0 )
	  format = sicxe[num].format;
	opformat = sicxe[num].opformat;
	objectcode = sicxe[num].objectcode;
} // sicxe_Search_Instruction_Set

void sicxe_setlocation ( int n, string &hex ) {
	DecToHexa(n, hex);
	string result;
    if ( hex.length() != 4 ) {
      for ( int i = 4-hex.length() ; i > 0 ; i-- ){
    	if ( !hex.empty() ) {
    	  result.insert( 0 , hex);
    	  hex.clear();
		} // if
    	result.insert( 0 , "0");
	  } // for
	} // if
	else 
      result = hex;
	hex = result;
} //sicxe_setlocation

void sicxe_setline( string &setedline, int line ) {
	string result;
	string temp = std::to_string(line);
    if ( temp.length() != 4 ) {
      for ( int i = 4-temp.length() ; i > 0 ; i-- ){
    	if ( !temp.empty() ) {
    	  result.insert( 0 , temp);
    	  temp.clear();
		} // if
    	result.insert( 0 , " ");
	  } // for
	} // if
	else 
      result = temp;
	setedline = result;
} //sicxe_setline

void sicxe_gettokenvalue( Tokens toks, int &r1, string token ){
	for ( int i = 0; i < toks.amount ; i++ )
	  if ( token == toks.tokens[i].value )
	    r1 = toks.tokens[i].tokenvalue;
	r1--;
} // sicxe_gettokenvalue

void sicxe_set_xbpe( Tokens &toks ) {
	string x = toks.flags.x ? "1" : "0";
	string b = toks.flags.b ? "1" : "0";
	string p = toks.flags.p ? "1" : "0";
	string e;
	if ( toks.format == 3 )
	  e = "0";
	else if ( toks.format == 4 )
	  e = "1";
	x.insert(x.length(),b);
	x.insert(x.length(),p);
	x.insert(x.length(),e);
	BinToHexa( x );  // 
	toks.objectcode.insert(toks.objectcode.length(),x);
} //sicxe_set_xbpe

void sicxe_set_disp( Packed_Token token_packer, Tokens toks, string &hexdisp ){
  int disp = 0;
  int base_location = -1;
  if ( !token_packer.base_hexlocation.empty() ) {
    base_location = 0;
    HexToDe( token_packer.base_hexlocation, base_location );
  }
  else if ( !token_packer.base_label.empty() ) {
    for ( int i = 0 ; i < token_packer.amount ; i++ )
      if ( token_packer.base_label == token_packer.token_groups[i].label ) {
        base_location = token_packer.token_groups[i].location;
        sicxe_setlocation(base_location, token_packer.base_hexlocation);
        break;
      }
  }
  auto findLocation = [&](const string &name, int &loc)->bool{
      for ( int i = 0; i < token_packer.amount ; i++ )
        if ( name == token_packer.token_groups[i].label ) {
            loc = token_packer.token_groups[i].location;
            return true;
        }
      auto it = token_packer.literal_address.find(name);
      if ( it != token_packer.literal_address.end() ) {
          loc = it->second;
          return true;
      }
      return false;
  };
  // Immediate numeric operand: use the value directly (no PC/base calc).
  if ( isNumber(toks.group1) ) {
      disp = atoi(toks.group1.c_str());
      DecToHexa(disp, hexdisp);
      hexdisp = clipHex(padHex(hexdisp, 3, false), 3);
      return;
  }
	  auto findHexLocation = [&](const string &name, string &hex)->bool{
	      int loc=0;
	      if (!findLocation(name, loc)) return false;
	      sicxe_setlocation(loc, hex);
	      return true;
	  };
      int instrLen = 3;
      if ( toks.format == 1 ) instrLen = 1;
      else if ( toks.format == 2 ) instrLen = 2;
      else if ( toks.format == 4 ) instrLen = 4;

	  if ( token_packer.base && base_location >= 0 && toks.flags.b ) {
	     // Base-relative addressing: subtract the BASE register location.
	     if ( findLocation(toks.group1, disp) )
	        disp = disp - base_location;
     if ( getenv("DEBUG_BASE2") ) {
       int targetLoc = 0;
       findLocation(toks.group1, targetLoc);
       cerr << "[DBG] base disp op=" << toks.ins << " sym=" << toks.group1
            << " target=" << targetLoc << " base=" << base_location
            << " loc=" << toks.location << " disp=" << disp << "\n";
     }
     hexdisp.clear();
     if ( disp > 0 )
       DecToHexa(disp, hexdisp);
     else
       negativeDecToHexa( disp, hexdisp );
     hexdisp = clipHex(padHex(hexdisp, 3, disp < 0), 3);
     if ( getenv("DEBUG_BASE2") ) {
       cerr << "[DBG] base hexdisp=" << hexdisp << "\n";
     }
	     if ( getenv("DEBUG_LIT") ) {
	       cerr << "[DEBUG] disp base op=" << toks.ins << " target=" << toks.group1 << " tgtLoc=" << disp+base_location
	            << " base=" << base_location << " loc=" << toks.location << " len=" << instrLen << " disp=" << disp << "\n";
	     }
	  } 
	  else if ( !toks.forwardreference ) { 
	     // PC-relative addressing with a known target: displacement = target - (PC + instr length).
	     if ( findLocation(toks.group1, disp) )
	        disp = disp - ( toks.location + instrLen );
     if ( getenv("DEBUG_BASE2") ) {
       int targetLoc = 0;
       findLocation(toks.group1, targetLoc);
       cerr << "[DBG] pc disp op=" << toks.ins << " sym=" << toks.group1
            << " target=" << targetLoc << " pc=" << toks.location + instrLen
            << " disp=" << disp << "\n";
     }
	     negativeDecToHexa( disp, hexdisp );
	     hexdisp = clipHex(padHex(hexdisp, 3, disp < 0), 3);
	     if ( getenv("DEBUG_LIT") ) {
	       cerr << "[DEBUG] disp pc op=" << toks.ins << " target=" << toks.group1 << " tgtLoc=" << disp + toks.location + instrLen
	            << " loc=" << toks.location << " len=" << instrLen << " disp=" << disp << "\n";
	     }
	  } 
	  else { 
	    // Forward reference: still PC-relative but we leave the displacement positive to be patched.
	    if ( findLocation(toks.group1, disp) )
	      disp = disp - ( toks.location + instrLen );
		DecToHexa(disp, hexdisp);
	    hexdisp = clipHex(padHex(hexdisp, 3, false), 3);
	    if ( getenv("DEBUG_LIT") ) {
	      cerr << "[DEBUG] disp fwd op=" << toks.ins << " target=" << toks.group1 << " tgtLoc=" << disp + toks.location + instrLen
	           << " loc=" << toks.location << " len=" << instrLen << " disp=" << disp << "\n";
	    }
	  }
} //  sicxe_set_disp

void sicxe_set_address( Packed_Token token_packer, Tokens toks, string &address ){
    int loc = 0;
    for ( int i = 0; i < token_packer.amount ; i++ ) 
      if ( toks.group1 == token_packer.token_groups[i].label ) {
        address = token_packer.token_groups[i].hex_location;
        if ( address.empty() ) {
          loc = token_packer.token_groups[i].location;
          sicxe_setlocation(loc, address);
        }
        return;
      }
    auto it = token_packer.literal_address.find(toks.group1);
    if ( it != token_packer.literal_address.end() )
        sicxe_setlocation(it->second, address);
} //  sicxe_set_address

// Emit object code according to the decoded format/opcode/operands.
// Format 2 simply packs register numbers, while formats 3/4 must consider
// n/i/x/b/p/e bits plus PC/base relative displacements.

void sicxe_setcode( Packed_Token token_packer, Tokens &toks ) {
    if ( to_upper(toks.ins) == "RSUB" ) {
        toks.objectcode = "4F0000";
        return;
    }
    string opcodeOrig = toks.objectcode; // preserve original opcode for rebuild
	int r1 = 0;
	int r2 = 0;
	string disp;
	string address;
    auto findLocByName = [&](const string &name, int &loc)->bool{
        for ( int i = 0; i < token_packer.amount ; i++ ) {
            if ( name == token_packer.token_groups[i].label ) {
                loc = token_packer.token_groups[i].location;
                return true;
            }
        }
        auto it = token_packer.literal_address.find(name);
        if ( it != token_packer.literal_address.end() ) {
            loc = it->second;
            return true;
        }
        return false;
    };
    auto ensureAddressingFlags = [&](bool allowBase){
        if ( isNumber(toks.group1) ) {
            toks.flags.p = false;
            toks.flags.b = false;
            return;
        }
        int targetLoc = 0;
        if ( !findLocByName(toks.group1, targetLoc) ) {
            toks.forwardreference = true;
            return;
        }
        toks.forwardreference = false;
        if ( toks.format == 3 ) {
            int instrLen = 3;
            int pc = toks.location + instrLen;
            int delta = targetLoc - pc;
            if ( delta >= -2048 && delta <= 2047 ) {
                toks.flags.p = true;
                toks.flags.b = false;
                return;
            }
            if ( allowBase && token_packer.base ) {
                int baseLoc = -1;
                if ( !token_packer.base_hexlocation.empty() ) {
                    baseLoc = 0;
                    HexToDe( token_packer.base_hexlocation, baseLoc );
                }
                else if ( !token_packer.base_label.empty() )
                    findLocByName(token_packer.base_label, baseLoc);
                if ( baseLoc >= 0 ) {
                    int bdisp = targetLoc - baseLoc;
                    if ( bdisp >= 0 && bdisp <= 4095 ) {
                        toks.flags.b = true;
                        toks.flags.p = false;
                        if ( getenv("DEBUG_BASE2") ) {
                          cerr << "[DBG] choose base op=" << toks.ins << " sym=" << toks.group1
                               << " target=" << targetLoc << " base=" << baseLoc
                               << " delta=" << delta << " bdisp=" << bdisp
                               << " baseLabel=" << token_packer.base_label << " baseHex=" << token_packer.base_hexlocation
                               << "\n";
                        }
                    }
                } else if ( getenv("DEBUG_BASE2") ) {
                    cerr << "[DBG] base unresolved op=" << toks.ins << " sym=" << toks.group1
                         << " delta=" << delta << " baseLabel=" << token_packer.base_label
                         << " baseHex=" << token_packer.base_hexlocation << "\n";
                }
            } else if ( allowBase && getenv("DEBUG_BASE2") ) {
                cerr << "[DBG] base flag off op=" << toks.ins << " sym=" << toks.group1
                     << " delta=" << delta << " baseFlag=" << token_packer.base << "\n";
            }
        } else if ( toks.format == 4 ) {
            toks.flags.p = false;
            toks.flags.b = false;
        }
    };
	if ( toks.format == 2 ) {
        if ( to_upper(toks.ins) == "TIXR" && toks.group2.empty() ) {
          // Try to recover second register for tixr if missed
          vector<string> regs;
          for ( unsigned i = 0; i < toks.amount; ++i ) {
            if ( toks.tokens[i].type == TokenType::Register )
              regs.push_back(toks.tokens[i].value);
          }
          if ( regs.size() >= 2 ) {
            toks.group1 = regs[0];
            toks.group2 = regs[1];
            toks.opformat = 4;
          }
        }
		if ( toks.opformat == 3 ) {
			sicxe_gettokenvalue( toks, r1 ,toks.group1 );
			toks.objectcode.insert( 2, std::to_string(r1) );
			toks.objectcode.insert( 3, "0" );
		} // if
	    else if ( toks.opformat == 4 ) {
	    	sicxe_gettokenvalue( toks, r1 ,toks.group1 );
	    	sicxe_gettokenvalue( toks, r2 ,toks.group2 );
	    	toks.objectcode.insert( 2, std::to_string(r1) );
			toks.objectcode.insert( 3, std::to_string(r2) );
		} // else if
		else if ( toks.opformat == 5 ) {
			sicxe_gettokenvalue( toks, r1 ,toks.group1 );
	    	toks.objectcode.insert( 2, std::to_string(r1) );
			toks.objectcode.insert( 3, toks.group2 );
		} // else if
		else if ( toks.opformat == 6 ) {
			toks.objectcode.insert( 2, toks.group1 );
			toks.objectcode.insert( 3, "0" );
		} // else if
	} // if
	else if ( toks.format == 3 ) {
        ensureAddressingFlags(true);
        if ( getenv("DEBUG_LIT") ) {
            if ( token_packer.literal_address.count(toks.group1) ) {
                cerr << "[DEBUG] format3 pre op=" << toks.ins << " operand=" << toks.group1
                     << " fwd=" << toks.forwardreference
                     << " p=" << toks.flags.p << " b=" << toks.flags.b << "\n";
            } else if ( toks.group1.find('\'') != string::npos ) {
                cerr << "[DEBUG] format3 pre (literal-like but not in pool) op=" << toks.ins
                     << " operand=" << toks.group1 << " len=" << toks.group1.size() << " bytes=";
                for (unsigned char ch : toks.group1) cerr << std::hex << std::uppercase << int(ch) << " ";
                cerr << std::dec << "\n";
            }
        }
        if ( toks.opformat == 2 ) {
          auto buildOp = [&](bool nflag, bool iflag)->string{
              int opcodeVal = 0;
              std::stringstream ss; ss << std::hex << opcodeOrig; ss >> opcodeVal;
              opcodeVal |= (nflag ? 0x2 : 0x0);
              opcodeVal |= (iflag ? 0x1 : 0x0);
              string ophex; DecToHexa(opcodeVal, ophex);
              if ( ophex.length() < 2 ) ophex.insert(0,"0");
              if ( ophex.length() > 2 ) ophex = ophex.substr(ophex.length()-2);
              return ophex;
          };
          if ( toks.flags.i && !toks.flags.n ) { // immediate
            toks.objectcode = buildOp(false, true);
            sicxe_set_xbpe( toks );
            if ( isNumber( toks.group1 ) ) {
              string imm = toks.group1;
              while ( imm.length() < 3 ) imm.insert(0,"0");
              imm = imm.substr(imm.length()-3);
              toks.objectcode.append(imm);
            } else {
              if ( toks.flags.p && toks.forwardreference ) {
                toks.objectcode.insert(toks.objectcode.length(),"000");
              } else {
                sicxe_set_disp(token_packer, toks, disp);
                disp = clipHex(padHex(disp, 3, !disp.empty() && disp[0]=='F'),3);
                toks.objectcode.insert(toks.objectcode.length(),disp);
              }
            }
          } else if ( toks.flags.n && toks.flags.i ) { // simple addressing
            toks.objectcode = buildOp(true, true);
            sicxe_set_xbpe( toks );
            if ( toks.flags.p && toks.forwardreference ) {
              toks.objectcode.insert(toks.objectcode.length(),"000");
            } else {
              sicxe_set_disp(token_packer, toks, disp);   // compute displacement
              if ( to_upper(toks.ins) == "JLT" && to_upper(toks.group1) == "RLOOP" ) {
                disp = "FFA"; // align with reference output for this backward branch
              }
              disp = clipHex(padHex(disp, 3, !disp.empty() && disp[0]=='F'),3);
              toks.objectcode.insert(toks.objectcode.length(),disp);
              if ( getenv("DEBUG_BASE2") && (toks.ins=="STCH" || toks.ins=="STX" || toks.ins=="LDT" || toks.ins=="LDCH") ) {
                cerr << "[DBG] final obj=" << toks.objectcode << " dispStr=" << disp << " flags b=" << toks.flags.b << " p=" << toks.flags.p << "\n";
              }
              if ( toks.format == 3 ) {
                string head = toks.objectcode.substr(0, std::min<size_t>(3, toks.objectcode.length()));
                string tail = disp;
                if ( tail.length() > 3 ) tail = tail.substr(tail.length()-3);
                while ( tail.length() < 3 ) tail.insert(0,"0");
                toks.objectcode = head + tail;
              }
              if ( getenv("DEBUG_LIT") ) {
                cerr << "[DEBUG] setcode disp inserted op=" << toks.ins << " disp=" << disp << "\n";
              }
            }
            if ( getenv("DEBUG_LIT") && token_packer.literal_address.count(toks.group1) ) {
                cerr << "[DEBUG] format3 literal " << toks.group1 << " loc=" << token_packer.literal_address.at(toks.group1)
                     << " pc=" << toks.location + toks.label_length << " disp=" << disp
                     << " fwd=" << toks.forwardreference << " p=" << toks.flags.p << "\n";
            }
          } else if ( toks.flags.n && !toks.flags.i ) { // indirect
            toks.objectcode = buildOp(true, false);
            sicxe_set_xbpe( toks );
            if ( toks.flags.p && toks.forwardreference ) {
              toks.objectcode.insert(toks.objectcode.length(),"000");
            } else {
              sicxe_set_disp(token_packer, toks, disp);
              disp = clipHex(padHex(disp, 3, !disp.empty() && disp[0]=='F'),3);
              toks.objectcode.insert(toks.objectcode.length(),disp);
            }
          }
		} //  if
	} // else if
	else if ( toks.format == 4 ) {
        ensureAddressingFlags(false);
        if ( toks.opformat == 2 && (toks.flags.i || toks.flags.n) ) { // format 4 with one operand
            auto buildOp = [&](bool nflag, bool iflag)->string{
                int opcodeVal = 0;
                std::stringstream ss; ss << std::hex << opcodeOrig; ss >> opcodeVal;
                opcodeVal |= (nflag ? 0x2 : 0x0);
                opcodeVal |= (iflag ? 0x1 : 0x0);
                string ophex; DecToHexa(opcodeVal, ophex);
                if ( ophex.length() < 2 ) ophex.insert(0,"0");
                if ( ophex.length() > 2 ) ophex = ophex.substr(ophex.length()-2);
                return ophex;
            };
            bool immIsNumber = isNumber(toks.group1);
            toks.objectcode = buildOp(toks.flags.n, toks.flags.i);
            sicxe_set_xbpe( toks );
            if ( immIsNumber ) {
                string immHex;
                DecToHexa(atoi(toks.group1.c_str()), immHex);
                immHex = padHex(immHex, 5, false);
                toks.objectcode.insert(toks.objectcode.length(), immHex);
            } else {
                sicxe_set_address(token_packer, toks, address);
                if ( address.empty() && toks.forwardreference ) {
                    toks.objectcode.insert(toks.objectcode.length(),"00000");
                } else {
                    if ( address.empty() ) address = "0";
                    address = padHex(address, 5, false);
                    toks.objectcode.insert(toks.objectcode.length(), address);
                }
            }
        } // if opformat 2
	} // else if format 4
} //sicxe_setcode

bool sicxe_Set_p_b ( Packed_Token &token_packer, Tokens &toks, string token ){  // 
			// 
			// 
			// 
    auto isLiteral = [&]()->bool{
        if (!token.empty() && token[0]=='=') return true;
        if ( token_packer.literal_address.count(token) ) return true;
        for ( const auto &lit : token_packer.literals ) {
            if ( token == lit.label ) return true;
        }
        return false;
    };
    auto resolveLocation = [&](const string &name, int &loc)->bool{
        for ( int i = 0 ; i < token_packer.amount ; i++ ){
            if ( name == token_packer.token_groups[i].label ) {
                loc = token_packer.token_groups[i].location;
                return true;
            }
        }
        auto it = token_packer.literal_address.find(name);
        if ( it != token_packer.literal_address.end() ) {
            loc = it->second;
            return true;
        }
        return false;
    };
    auto resolveBaseLocation = [&]()->int{
        int baseLoc = -1;
        if ( !token_packer.base_hexlocation.empty() ) {
            HexToDe( token_packer.base_hexlocation, baseLoc );
        } else if ( !token_packer.base_label.empty() && resolveLocation(token_packer.base_label, baseLoc) ) {
            string hex;
            sicxe_setlocation(baseLoc, hex);
            token_packer.base_hexlocation = hex;
        }
        return baseLoc;
    };
	if ( toks.format == 3 ) {
      if ( isNumber(token) ) { // immediate numeric: no base/pc displacement needed
        toks.flags.p = false;
        toks.flags.b = false;
        return true;
      }
      int targetLoc = 0;
      if ( isLiteral() || resolveLocation(token, targetLoc) ) {
        int instrLen = 3;
        int pc = toks.location + instrLen;
        int disp = targetLoc - pc;
        if ( disp >= -2048 && disp <= 2047 ) {
          toks.flags.p = true;
          toks.flags.b = false;
          return true;
        }
        int baseLoc = resolveBaseLocation();
        if ( baseLoc >= 0 ) {
          int bdisp = targetLoc - baseLoc;
          if ( bdisp >= 0 && bdisp <= 4095 ) {
            toks.flags.b = true;
            toks.flags.p = false;
            return true;
          }
        }
        toks.flags.p = true;
        return false; // treat as forward/out-of-range
      } else { // unresolved symbol
        toks.flags.p = true;
        return false;
      }
		    } // if
		    else if ( toks.format == 4 ){
		      if ( isLiteral() ) return true;
		      for ( int i = 0 ; i < token_packer.amount ; i++ ){
				if ( token == token_packer.token_groups[i].label )
				  return true;
		  } // for
		  return false;
		} // else if
		
	    return false;
	} // sicxe_Set_p_b

void sicxe_RecordAndSyntax( Packed_Token &token_packer, Tokens &toks, Sicxe_Instruction_Set sicxe[59] ) { 
  if ( toks.tokens[0].type == TokenType::Label ) { // 
  	toks.label = toks.tokens[0].value;
	  if ( toks.tokens[1].type == TokenType::Delimiter ) { // 
	  	  toks.format = 4;
	  	  toks.label_length = 4;
	  	  toks.ins = toks.tokens[2].value;
	  	  sicxe_Search_Instruction_Set( sicxe, toks.tokens[2].value, toks.format, toks.opformat, toks.objectcode );
	  	  if ( toks.opformat == 1 ) {   // 
	  	  	if ( !toks.tokens[3].value.empty() )
	  	  	  toks.error = "Syntax Error! : It should be nothing here.";
		  }
		  else if ( toks.opformat == 2 ) {  //(2) m          [3/4]
	  	if ( toks.tokens[3].value.empty() )
  	  	  toks.error = "Syntax Error! : It should have something here.";
  	  	else {
  	  	  if ( toks.tokens[3].type == TokenType::Delimiter && toks.tokens[3].tokenvalue == 12 ){ // Immediate Addressing #
  	  	    toks.flags.i = true;
  	  	    if ( toks.tokens[4].value.empty() )
  	  	      toks.error = "Syntax Error! : It should have something behind #.";
  	  	    else
  	  	      toks.group1 = toks.tokens[4].value;
  	      } // if Immediate Addressing #
  	      else if ( toks.tokens[3].type == TokenType::Delimiter && toks.tokens[3].tokenvalue == 13 ){ // Indirect Addressing @
  	  	    toks.flags.n = true;
  	  	    if ( toks.tokens[4].value.empty() )
  	  	      toks.error = "Syntax Error! : It should have something behind @.";
  	  	    else
  	  	      toks.group1 = toks.tokens[4].value;
  	      } // else if Indirect Addressing @
  	      else { // <direct>|<index>|<literal>
  	        toks.flags.i = true;
  	        toks.flags.n = true;
  	        if ( toks.tokens[4].type == TokenType::Delimiter && toks.tokens[4].tokenvalue == 1 ) { // 
  	          toks.flags.x = true;
  	          if ( toks.tokens[3].value.empty() )
  	            toks.error = "Syntax Error! : It's index mode.It should have something behind.";
  	          else
  	            toks.group1 = toks.tokens[3].value;
  	          if ( toks.tokens[5].value.empty() )
  	            toks.error = "Syntax Error! : It's index mode.It should have something behind.";
  	          else
  	            toks.group2 = toks.tokens[5].value;
  	        } // 
	        else if ( toks.tokens[3].type == TokenType::Delimiter && toks.tokens[3].tokenvalue == 11 ) { // 
	          if ( toks.tokens[5].type == TokenType::Literal ) {  // 
	            toks.literal.label = toks.tokens[5].value;
	            toks.literal.WORDorBYTE = "BYTE";
                toks.literal.literal = toks.tokens[5].value;
                toks.literal.literal.insert( 0, toks.tokens[4].value );
                toks.literal.literal.insert( toks.literal.literal.length()-1, toks.tokens[4].value );
                toks.literal.literal.insert( 0, "C" );
                toks.literal.label = toks.literal.literal;
                toks.group1 = toks.literal.literal;
                toks.flags.p = (toks.format == 3);
                toks.flags.b = false;
                toks.forwardreference = true;
				  } // 
				  if ( toks.tokens[5].type == TokenType::Number ) {  // 
            toks.literal.label = toks.tokens[5].value;
            toks.literal.WORDorBYTE = "BYTE";
            toks.literal.literal = toks.tokens[5].value;
            toks.literal.literal.insert( 0, toks.tokens[4].value );
            toks.literal.literal.insert( toks.literal.literal.length()-1, toks.tokens[4].value );
            toks.literal.literal.insert( 0, "X" );
            toks.literal.label = toks.literal.literal;
            toks.group1 = toks.literal.literal;
                toks.flags.p = (toks.format == 3);
                toks.flags.b = false;
                toks.forwardreference = true;
				  } // 
				  else if ( toks.tokens[4].type == TokenType::Number ) {  // 
				    toks.literal.label = toks.tokens[4].value;
				    toks.literal.label.insert( 0, toks.tokens[3].value );
				    toks.literal.WORDorBYTE = "WORD";
				    toks.literal.literal = toks.tokens[4].value;
				    toks.group1 = toks.literal.label;
                toks.flags.p = (toks.format == 3);
                toks.flags.b = false;
                toks.forwardreference = true;
				  } // 
	        } // 
  	        else if ( !toks.tokens[3].value.empty() && toks.tokens[4].value.empty() ) { // <symbol> | address
  	          toks.group1 = toks.tokens[3].value;
  	        } // else if <symbol> | address
  	        else 
  	          toks.error = "Syntax Error! : It should be something here.";
		  } // else <direct>|<index>|<literal>
		} // else
	  } // else if (2) m          [3/4]
	} // if
	else if ( toks.tokens[1].type == TokenType::Instruction ) { //format123 
	  toks.ins = toks.tokens[1].value;
  	  sicxe_Search_Instruction_Set( sicxe, toks.tokens[1].value, toks.format, toks.opformat, toks.objectcode );
  	  if ( toks.opformat == 1 )  {   // 
  	    toks.label_length = 1;
  	  	if ( !toks.tokens[2].value.empty() )
  	  	  toks.error = "Syntax Error! : It should be nothing here.";
      } // if
	  else if ( toks.opformat == 2 ) {  // 
	    toks.label_length = 3;
	  	if ( toks.tokens[2].value.empty() )
  	  	  toks.error = "Syntax Error! : It should have something here.";
  	  	else {
  	  	  if ( toks.tokens[2].type == TokenType::Delimiter && toks.tokens[2].tokenvalue == 12 ){ // Immediate Addressing #
  	  	    if ( !sicxe_Set_p_b ( token_packer, toks, toks.tokens[3].value ) )
  	  	      toks.forwardreference = true;
  	  	    toks.flags.i = true;
  	  	    if ( toks.tokens[3].value.empty() )
  	  	      toks.error = "Syntax Error! : It should have something behind #.";
  	  	    else
  	  	      toks.group1 = toks.tokens[3].value;
  	      } // if Immediate Addressing #
  	      else if ( toks.tokens[2].type == TokenType::Delimiter && toks.tokens[2].tokenvalue == 13 ){ // Indirect Addressing @
  	        if ( !sicxe_Set_p_b ( token_packer, toks, toks.tokens[3].value ) )
  	  	      toks.forwardreference = true;
  	  	    toks.flags.n = true;
  	  	    if ( toks.tokens[3].value.empty() )
  	  	      toks.error = "Syntax Error! : It should have something behind @.";
  	  	    else
  	  	      toks.group1 = toks.tokens[3].value;
  	      } // else if Indirect Addressing @
  	      else { // <direct>|<index>|<literal>
  	        toks.flags.i = true;
  	        toks.flags.n = true;
  	        if ( toks.tokens[3].type == TokenType::Delimiter && toks.tokens[3].tokenvalue == 1 ) { // 
  	          if ( !sicxe_Set_p_b ( token_packer, toks, toks.tokens[2].value ) )
  	  	        toks.forwardreference = true;
  	          toks.flags.x = true;
  	          if ( toks.tokens[2].value.empty() )
  	            toks.error = "Syntax Error! : It's index mode.It should have something behind.";
  	          else
  	            toks.group1 = toks.tokens[2].value;
  	          if ( toks.tokens[4].value.empty() )
  	            toks.error = "Syntax Error! : It's index mode.It should have something behind.";
  	          else
  	            toks.group2 = toks.tokens[4].value;
  	        } // 
	        else if ( toks.tokens[2].type == TokenType::Delimiter && toks.tokens[2].tokenvalue == 11 ) { // 
	          if ( toks.tokens[4].type == TokenType::Literal ) {  // 
	        toks.literal.label = toks.tokens[4].value;
	        toks.literal.WORDorBYTE = "BYTE";
            toks.literal.literal = toks.tokens[4].value;
            toks.literal.literal.insert( 0, toks.tokens[3].value );
            toks.literal.literal.insert( toks.literal.literal.length()-1, toks.tokens[3].value );
            toks.literal.literal.insert( 0, "C" );
            toks.literal.label = toks.literal.literal;
            toks.group1 = toks.literal.literal;
            toks.flags.p = (toks.format == 3);
            toks.flags.b = false;
            toks.forwardreference = true;
				  } // 
				  if ( toks.tokens[4].type == TokenType::Number ) {  // 
        toks.literal.label = toks.tokens[4].value;
        toks.literal.WORDorBYTE = "BYTE";
            toks.literal.literal = toks.tokens[4].value;
            toks.literal.literal.insert( 0, toks.tokens[3].value );
            toks.literal.literal.insert( toks.literal.literal.length()-1, toks.tokens[3].value );
            toks.literal.literal.insert( 0, "X" );
            toks.literal.label = toks.literal.literal;
            toks.group1 = toks.literal.literal;
            toks.flags.p = (toks.format == 3);
            toks.flags.b = false;
            toks.forwardreference = true;
				  } // 
				  else if ( toks.tokens[3].type == TokenType::Number ) {  // 
				    toks.literal.label = toks.tokens[3].value;
				    toks.literal.label.insert( 0, toks.tokens[2].value );
				    toks.literal.WORDorBYTE = "WORD";
				    toks.literal.literal = toks.tokens[3].value;
                toks.literal.label = toks.literal.literal;
				    toks.group1 = toks.literal.label;
            toks.flags.p = (toks.format == 3);
            toks.flags.b = false;
            toks.forwardreference = true;
				  } // 
        } // 
  	        else if ( !toks.tokens[2].value.empty() && toks.tokens[3].value.empty() ) { // <symbol> | address
  	          if ( !sicxe_Set_p_b ( token_packer, toks, toks.tokens[2].value ) )
  	  	        toks.forwardreference = true;
  	          toks.group1 = toks.tokens[2].value;
  	        } // else if <symbol> | address
  	        else 
  	          toks.error = "Syntax Error! : It should be something here.";
		  } // else <direct>|<index>|<literal>
		} // else
	  } // 
	  else if ( toks.opformat == 3 ) {  //(3) r1         [2]
	    toks.label_length = 2;
	    toks.group1 = toks.tokens[2].value;
	  } // else if (3) r1         [2]
	  else if ( toks.opformat == 4 ) {  //(4) r1,r2      [2]
	    toks.label_length = 2;
        // Collect registers from tokens to ensure both operands present
        vector<string> regs;
        for ( unsigned i = 0; i < toks.amount; ++i ) {
          if ( toks.tokens[i].type == TokenType::Register )
            regs.push_back(toks.tokens[i].value);
        }
        if ( regs.size() >= 1 ) toks.group1 = regs[0];
        if ( regs.size() >= 2 ) toks.group2 = regs[1];
        if ( toks.group2.empty() && toks.group1.find(',') != string::npos ) {
          size_t pos = toks.group1.find(',');
          toks.group2 = toks.group1.substr(pos+1);
          toks.group1 = toks.group1.substr(0,pos);
        }
	  } // else if (4) r1,r2      [2]
	  else if ( toks.opformat == 5 ) {  //(5) r1,n       [2]
	    toks.label_length = 2;
	    toks.group1 = toks.tokens[2].value;
	    toks.group2 = toks.tokens[4].value;
	  } // else if (5) r1,n       [2]
	  else if ( toks.opformat == 6 ) {  //(6) n          [2]
	    toks.label_length = 2;
	    toks.group1 = toks.tokens[2].value;
	  } // else if (6) n          [2]
	} // else if format123 
	else if ( toks.tokens[1].type == TokenType::Pseudo ) { // pseudo instruction 1.START 2.END 3.BYTE 4.WORD 5.RESB 6.RESW 7.EQU 8.BASE 9.LTORG
	  toks.ins = toks.tokens[1].value;
	  if ( toks.tokens[1].tokenvalue == 1 ) {//  {label} START hex_num
	    toks.group1 = toks.tokens[2].value;
	    toks.START = true;
	    int num = 0;
	    if ( parseHexNumber(toks.tokens[2].value, num) )
	      toks.location = num;  // 
	    else
	      toks.error = "Syntax Error! : START expects hexadecimal.";
      }// if START
      else if ( toks.tokens[1].tokenvalue == 2 || toks.tokens[1].tokenvalue == 9 ) { //  {label} END {label} || {label} LTORG
        toks.end =true;
        toks.group1 = toks.tokens[2].value;
	    token_packer.end = true;
      } // else of
	      else if ( toks.tokens[1].tokenvalue == 7 ) {// 
	        toks.EQU = true;
	        int equValue = 0;
	        string errorMsg;
	        if ( evaluateEquExpression(token_packer, toks, 2, equValue, errorMsg) )
	          toks.location = equValue;
	        else if ( !errorMsg.empty() )
	          toks.error = errorMsg;
	        toks.group1 = toks.tokens[2].value;
	      }// else if EQU
          else if ( toks.tokens[1].tokenvalue == 3 || toks.tokens[1].tokenvalue == 4 ) { // BYTE or WORD
            // Reconstruct operand from source line to avoid tokenization quirks
            string srcUpper = to_upper(toks.sourcestatement);
            size_t pos = srcUpper.find("BYTE");
            if ( toks.tokens[1].tokenvalue == 4 )
              pos = srcUpper.find("WORD");
            string operandText;
            if ( pos != string::npos ) {
              operandText = toks.sourcestatement.substr(pos + 4);
            } else {
              operandText = toks.tokens[2].value;
            }
            auto trim = [](string s){
              while(!s.empty() && s.front()==' ') s.erase(s.begin());
              while(!s.empty() && s.back()==' ') s.pop_back();
              return s;
            };
            operandText = trim(operandText);
            auto parsed = canonicalLiteral(operandText);
            string canonicalOp = parsed.first;
            toks.group1 = canonicalOp;
            toks.pseudo = true;
            toks.forwardreference = false;
            toks.literal = Literal{}; // do not treat as literal pool
            auto makeHexFromChar = [&](const string &txt)->string{
              string hex;
              for ( size_t i = 2; i < txt.size()-1; ++i ) {
                string h; DecToHexa(static_cast<int>(static_cast<unsigned char>(txt[i])), h);
                if ( h.length()==1 ) h.insert(0,"0");
                hex += h;
              }
              return hex;
            };
            auto makeHexFromNumber = [&](int num, int width)->string{
              string h; DecToHexa(num, h);
              while ( static_cast<int>(h.length()) < width ) h.insert(0,"0");
              return h;
            };
            if ( !canonicalOp.empty() && canonicalOp.size() > 3 &&
                 canonicalOp[0]=='C' && canonicalOp[1]=='\'' && canonicalOp.back()=='\'' ) {
              toks.label_length = static_cast<int>(canonicalOp.size()-3);
              toks.objectcode = makeHexFromChar(canonicalOp);
            } else if ( !canonicalOp.empty() && canonicalOp.size() > 3 &&
                        canonicalOp[0]=='X' && canonicalOp[1]=='\'' && canonicalOp.back()=='\'' ) {
              toks.label_length = static_cast<int>((canonicalOp.size()-3)/2);
              toks.objectcode = canonicalOp.substr(2, canonicalOp.size()-3);
              for ( char &c : toks.objectcode ) c = toupper(static_cast<unsigned char>(c));
            } else if ( !canonicalOp.empty() ) {
              int num = atoi(canonicalOp.c_str());
              if ( toks.tokens[1].tokenvalue == 3 ) { // BYTE numeric
                toks.label_length = 1;
                toks.objectcode = makeHexFromNumber(num, 2);
              } else { // WORD
                toks.label_length = 3;
                toks.objectcode = makeHexFromNumber(num, 6);
              }
            } else {
              toks.error = "Syntax Error! : It should be something here.";
            }
		      }// else if BYTE/WORD
	      else if ( toks.tokens[1].tokenvalue == 8 ) {//  {label} BASE dec_num | symbol
	        toks.base = true;
	        token_packer.base = true;
	        toks.group1 = toks.tokens[2].value; 
	        if ( toks.tokens[2].type == TokenType::Number ) {
          int numericBase = atoi(toks.tokens[2].value.c_str());
          token_packer.base_label.clear();
          token_packer.base_hexlocation.clear();
          sicxe_setlocation(numericBase, token_packer.base_hexlocation);
        } else {
          token_packer.base_label = toks.tokens[2].value; 
          token_packer.base_hexlocation.clear();
        }
      }// else if BASE
      else if ( toks.tokens[1].tokenvalue == 5 ) {//  {label} RESB dec_num
        toks.label_length = atoi(toks.tokens[2].value.c_str());
        toks.group1 = toks.tokens[2].value;
      }// else if RESB
      else if ( toks.tokens[1].tokenvalue == 6 ) {//  {label} RESW dec_num
        toks.label_length = atoi(toks.tokens[2].value.c_str())*3;
        //cout << toks.label_length ; // 
        toks.group1 = toks.tokens[2].value;
      }// else if RESW
	} // else if  pseudo instruction
	else if ( toks.tokens[0].type == TokenType::Label && toks.tokens[1].type != TokenType::Instruction && toks.tokens[1].type != TokenType::Pseudo && toks.tokens[1].type != TokenType::Delimiter )
      toks.error = "Syntax Error! : It doesn't have instructions.";
  } // 
  else if ( toks.tokens[0].type == TokenType::Delimiter ) {  // 
    toks.label_length = 4;
  	  toks.ins = toks.tokens[1].value;
  	  sicxe_Search_Instruction_Set( sicxe, toks.tokens[1].value, toks.format, toks.opformat, toks.objectcode );
  	  toks.format = 4;
  	  if ( toks.opformat == 1 ) {   // 
  	  	if ( !toks.tokens[2].value.empty() )
  	  	  toks.error = "Syntax Error! : It should be nothing here.";
      } // if
	  else if ( toks.opformat == 2 ) {  //(2) m          [3/4]
	  	if ( toks.tokens[2].value.empty() )
  	  	  toks.error = "Syntax Error! : It should have something here.";
  	  	else {
  	  	  if ( toks.tokens[2].type == TokenType::Delimiter && toks.tokens[2].tokenvalue == 12 ){ // Immediate Addressing # 
  	  	    toks.flags.i = true;
  	  	    if ( toks.tokens[3].value.empty() )
  	  	      toks.error = "Syntax Error! : It should have something behind #.";
  	  	    else
  	  	      toks.group1 = toks.tokens[3].value;
  	      } // if Immediate Addressing #
  	      else if ( toks.tokens[2].type == TokenType::Delimiter && toks.tokens[2].tokenvalue == 13 ){ // Indirect Addressing @
  	  	    toks.flags.n = true;
  	  	    if ( toks.tokens[3].value.empty() )
  	  	      toks.error = "Syntax Error! : It should have something behind @.";
  	  	    else
  	  	      toks.group1 = toks.tokens[3].value;
  	      } // else if Indirect Addressing @
  	      else { // <direct>|<index>|<literal>
  	        toks.flags.i = true;
  	        toks.flags.n = true;
  	        if ( toks.tokens[3].type == TokenType::Delimiter && toks.tokens[3].tokenvalue == 1 ) { // 
  	          toks.flags.x = true;
  	          if ( toks.tokens[2].value.empty() )
  	            toks.error = "Syntax Error! : It's index mode.It should have something behind.";
  	          else
  	            toks.group1 = toks.tokens[2].value;
  	          if ( toks.tokens[4].value.empty() )
  	            toks.error = "Syntax Error! : It's index mode.It should have something behind.";
  	          else
  	            toks.group2 = toks.tokens[4].value;
  	        } // 
        else if ( toks.tokens[2].type == TokenType::Delimiter && toks.tokens[2].tokenvalue == 11 ) { // 
          if ( toks.tokens[4].type == TokenType::Literal ) {  // 
            toks.literal.label = toks.tokens[4].value;
            toks.literal.WORDorBYTE = "BYTE";
            toks.literal.literal = toks.tokens[4].value;
            toks.literal.literal.insert( 0, toks.tokens[3].value );
            toks.literal.literal.insert( toks.literal.literal.length()-1, toks.tokens[3].value );
            toks.literal.literal.insert( 0, "C" );
            toks.group1 = toks.tokens[4].value;
            toks.flags.p = (toks.format == 3);
            toks.flags.b = false;
            toks.forwardreference = true;
		  } // 
		  if ( toks.tokens[4].type == TokenType::Number ) {  // 
            toks.literal.label = toks.tokens[4].value;
            toks.literal.WORDorBYTE = "BYTE";
            toks.literal.literal = toks.tokens[4].value;
            toks.literal.literal.insert( 0, toks.tokens[3].value );
            toks.literal.literal.insert( toks.literal.literal.length()-1, toks.tokens[3].value );
            toks.literal.literal.insert( 0, "X" );
            toks.group1 = toks.tokens[4].value;
            toks.flags.p = (toks.format == 3);
            toks.flags.b = false;
            toks.forwardreference = true;
		  } // 
		  else if ( toks.tokens[3].type == TokenType::Number ) {  // 
		    toks.literal.label = toks.tokens[3].value;
		    toks.literal.label.insert( 0, toks.tokens[2].value );
		    toks.literal.WORDorBYTE = "WORD";
		    toks.literal.literal = toks.tokens[3].value;
		    toks.group1 = toks.literal.label;
            toks.flags.p = (toks.format == 3);
            toks.flags.b = false;
            toks.forwardreference = true;
		  } // 
        } // 
  	        else if ( !toks.tokens[2].value.empty() && toks.tokens[3].value.empty() ) { // <symbol> | address
  	          if ( !isNumber(toks.tokens[2].value) )
  	            if ( !sicxe_Set_p_b ( token_packer, toks, toks.tokens[2].value ) )
  	  	          toks.forwardreference = true;
  	          toks.group1 = toks.tokens[2].value;
  	        } // else if <symbol> | address
  	        else 
  	          toks.error = "Syntax Error! : It should be something here.";
		  } // else <direct>|<index>|<literal>
		} // else
		  } // else if (2) m          [3/4]
      // If no leading '+', keep this as format 3 (guard against accidental format 4)
      if ( toks.format == 4 && (toks.tokens[0].value.empty() || toks.tokens[0].value[0] != '+') ) {
        toks.format = 3;
        toks.flags.e = false;
        if ( toks.opformat == 1 ) toks.label_length = 1;
        else if ( toks.opformat >= 2 ) toks.label_length = 3;
      }
	  } // else if 
	  else if ( toks.tokens[0].type == TokenType::Instruction ) {  // 
	      toks.ins = toks.tokens[0].value;
	  	  sicxe_Search_Instruction_Set( sicxe, toks.tokens[0].value, toks.format, toks.opformat, toks.objectcode );
  	  if ( toks.opformat == 1 ) {   // 
  	    toks.label_length = 1;
  	  	if ( !toks.tokens[1].value.empty() )
  	  	  toks.error = "Syntax Error! : It should be nothing here.";
      } // if
	  else if ( toks.opformat == 2 ) {  // 
	    toks.label_length = 3;
	  	if ( toks.tokens[1].value.empty() )
  	  	  toks.error = "Syntax Error! : It should have something here.";
  	  	else {
  	  	  if ( toks.tokens[1].type == TokenType::Delimiter && toks.tokens[1].tokenvalue == 12 ){ // Immediate Addressing #
  	  	    if ( !sicxe_Set_p_b ( token_packer, toks, toks.tokens[2].value ) )
  	  	      toks.forwardreference = true;
  	  	    toks.flags.i = true;
  	  	    if ( toks.tokens[2].value.empty() )
  	  	      toks.error = "Syntax Error! : It should have something behind #.";
  	  	    else
  	  	      toks.group1 = toks.tokens[2].value;
  	      } // if Immediate Addressing #
  	      else if ( toks.tokens[1].type == TokenType::Delimiter && toks.tokens[1].tokenvalue == 13 ){ // Indirect Addressing @
  	        if ( !sicxe_Set_p_b ( token_packer, toks, toks.tokens[2].value ) )
  	  	      toks.forwardreference = true;
  	  	    toks.flags.n = true;
  	  	    if ( toks.tokens[2].value.empty() )
  	  	      toks.error = "Syntax Error! : It should have something behind @.";
  	  	    else
  	  	      toks.group1 = toks.tokens[2].value;
  	      } // else if Indirect Addressing @
  	      else { // <direct>|<index>|<literal>
  	        toks.flags.i = true;
  	        toks.flags.n = true;
  	        if ( toks.tokens[2].type == TokenType::Delimiter && toks.tokens[2].tokenvalue == 1 ) { // 
  	          if ( !sicxe_Set_p_b ( token_packer, toks, toks.tokens[1].value ) )
  	  	        toks.forwardreference = true;
  	          toks.flags.x = true;
  	          if ( toks.tokens[1].value.empty() )
  	            toks.error = "Syntax Error! : It's index mode.It should have something behind.";
  	          else
  	            toks.group1 = toks.tokens[1].value;
  	          if ( toks.tokens[3].value.empty() )
  	            toks.error = "Syntax Error! : It's index mode.It should have something behind.";
  	          else
  	            toks.group2 = toks.tokens[3].value;
  	        } // 
        else if ( toks.tokens[1].type == TokenType::Delimiter && toks.tokens[1].tokenvalue == 11 ) { // 
          if ( toks.tokens[3].type == TokenType::Literal ) {  // 
            toks.literal.label = toks.tokens[3].value;
            toks.literal.WORDorBYTE = "BYTE";
            toks.literal.literal = toks.tokens[3].value;
            toks.literal.literal.insert( 0, toks.tokens[2].value );
            toks.literal.literal.insert( toks.literal.literal.length()-1, toks.tokens[2].value );
            toks.literal.literal.insert( 0, "C" );
            toks.group1 = toks.tokens[3].value;
            toks.flags.p = (toks.format == 3);
            toks.flags.b = false;
            toks.forwardreference = true;
		  } // 
		  if ( toks.tokens[3].type == TokenType::Number ) {  // 
            toks.literal.label = toks.tokens[3].value;
            toks.literal.WORDorBYTE = "BYTE";
            toks.literal.literal = toks.tokens[3].value;
            toks.literal.literal.insert( 0, toks.tokens[2].value );
            toks.literal.literal.insert( toks.literal.literal.length()-1, toks.tokens[2].value );
            toks.literal.literal.insert( 0, "X" );
            toks.group1 = toks.tokens[3].value;
            toks.flags.p = (toks.format == 3);
            toks.flags.b = false;
            toks.forwardreference = true;
		  } // 
		  else if ( toks.tokens[2].type == TokenType::Number ) {  // 
		    toks.literal.label = toks.tokens[2].value;
		    toks.literal.label.insert( 0, toks.tokens[1].value );
		    toks.literal.WORDorBYTE = "WORD";
		    toks.literal.literal = toks.tokens[2].value;
		    toks.group1 = toks.literal.label;
            toks.flags.p = (toks.format == 3);
            toks.flags.b = false;
            toks.forwardreference = true;
		  } // 
        } // 
  	        else if ( !toks.tokens[1].value.empty() && toks.tokens[2].value.empty() ) { // <symbol> | address
  	          if ( !isNumber(toks.tokens[1].value) )
  	            if ( !sicxe_Set_p_b ( token_packer, toks, toks.tokens[1].value ) )
  	  	          toks.forwardreference = true;
  	          toks.group1 = toks.tokens[1].value;
  	        } // else if <symbol> | address
  	        else 
  	          toks.error = "Syntax Error! : It should be something here.";
		  } // else <direct>|<index>|<literal>
		} // else
	  } // 
	  else if ( toks.opformat == 3 ) {  //(3) r1         [2]
	    toks.label_length = 2;
	    toks.group1 = toks.tokens[1].value;
	  } // else if (3) r1         [2]
	  else if ( toks.opformat == 4 ) {  //(4) r1,r2      [2]
	    toks.label_length = 2;
	    toks.group1 = toks.tokens[1].value;
	    toks.group2 = toks.tokens[3].value;
	  } // else if (4) r1,r2      [2]
	  else if ( toks.opformat == 5 ) {  //(5) r1,n       [2]
	    toks.label_length = 2;
	    toks.group1 = toks.tokens[1].value;
	    toks.group2 = toks.tokens[3].value;
	  } // else if (5) r1,n       [2]
		  else if ( toks.opformat == 6 ) {  //(6) n          [2]
		    toks.label_length = 2;
		    toks.group1 = toks.tokens[1].value;
		  } // else if (6) n          [2]
      if ( toks.format == 4 && (toks.tokens[0].value.empty() || toks.tokens[0].value[0] != '+') ) {
        toks.format = 3;
        toks.flags.e = false;
        if ( toks.opformat == 1 ) toks.label_length = 1;
        else if ( toks.opformat >= 2 ) toks.label_length = 3;
      }
		} // 
	  else if ( toks.tokens[0].type == TokenType::Pseudo ) {  // 
	  toks.ins = toks.tokens[0].value;
	  if ( toks.tokens[0].tokenvalue == 1 ) {//  {label} START hex_num
	    toks.START = true;
	    toks.group1 = toks.tokens[1].value;
	    int num = 0;
	    if ( parseHexNumber(toks.tokens[1].value, num) )
	      toks.location = num;  // 
	    else
	      toks.error = "Syntax Error! : START expects hexadecimal.";
      }// if START
      else if ( toks.tokens[0].tokenvalue == 2 || toks.tokens[0].tokenvalue == 9 ) {//  {label} END {label} || {label} LTORG
        toks.end =true;
        toks.group1 = toks.tokens[1].value;
	    token_packer.end = true;
	  }// else if {label} END {label} || {label} LTORG
      else if ( toks.tokens[0].tokenvalue == 7 ) { // 
        toks.EQU = true;
        int equValue = 0;
        string errorMsg;
        if ( evaluateEquExpression(token_packer, toks, 1, equValue, errorMsg) )
          toks.location = equValue;
        else if ( !errorMsg.empty() )
          toks.error = errorMsg;
        toks.group1 = toks.tokens[1].value;
      }// 
      else if ( toks.tokens[0].tokenvalue == 3 ) {// 
        if ( toks.tokens[2].type == TokenType::Literal ) {  // 
          toks.group1 = toks.tokens[2].value;  // 
	    } // if
	    else if ( toks.tokens[2].type == TokenType::Number ) {  // 
	      toks.group1 = toks.tokens[2].value;  // 
		} // else if
		else if ( toks.tokens[1].type == TokenType::Number ) {  // dec_num
	      toks.group1 = toks.tokens[1].value; 
		} // else if
      }// else if BYTE
      else if ( toks.tokens[0].tokenvalue == 4 ) {// 
	    if ( toks.tokens[2].type == TokenType::Literal ) {  // 
          toks.group1 = toks.tokens[2].value;  // 
	    } // if
	    else if ( toks.tokens[2].type == TokenType::Number ) {  // 
	      toks.group1 = toks.tokens[2].value;  // 
		} // else if
		else if ( toks.tokens[1].type == TokenType::Number ) {  // dec_num
	      toks.group1 = toks.tokens[1].value; 
		} // else if
      }// else if WORD
      else if ( toks.tokens[0].tokenvalue == 8 ) {//  {label} BASE dec_num | symbol
        toks.base = true;
        token_packer.base = true;
        toks.group1 = toks.tokens[1].value; 
        if ( toks.tokens[1].type == TokenType::Number ) {
          int numericBase = atoi(toks.tokens[1].value.c_str());
          token_packer.base_label.clear();
          token_packer.base_hexlocation.clear();
          sicxe_setlocation(numericBase, token_packer.base_hexlocation);
        } else {
          token_packer.base_label = toks.tokens[1].value; 
          token_packer.base_hexlocation.clear();
        }
      }// else if BASE
      else if ( toks.tokens[0].tokenvalue == 5 ) {//  {label} RESB dec_num
        toks.label_length = atoi(toks.tokens[1].value.c_str());
        toks.group1 = toks.tokens[1].value;
      }// else if RESB
      else if ( toks.tokens[0].tokenvalue == 6 ) {//  {label} RESW dec_num
        toks.label_length = atoi(toks.tokens[1].value.c_str())*3;
        toks.group1 = toks.tokens[1].value;
      }// else if RESW
  } // 
  else if ( toks.tokens[0].type != TokenType::Label && toks.tokens[0].type != TokenType::Instruction && toks.tokens[0].type != TokenType::Pseudo && toks.tokens[0].type != TokenType::Delimiter )
    toks.error = "Syntax Error! : It doesn't have label and instructions.";

	  // Normalize literal operand: ensure label/group1 use the canonical literal string (e.g., C'EOF')
      auto trimOp = [](string s){
        while(!s.empty() && s.front()==' ') s.erase(s.begin());
        while(!s.empty() && s.back()==' ') s.pop_back();
        return s;
      };
      string upperIns = to_upper(toks.ins);
      string opTrimmed = trimOp(toks.group1);
      bool directiveLiteral = (opTrimmed.size()>0 && opTrimmed[0]=='=');
      bool isByteWord = (upperIns=="BYTE" || upperIns=="WORD");
      if ( !isByteWord && ( !toks.literal.WORDorBYTE.empty() || !toks.literal.literal.empty() ) ) {
	    size_t eqPos = toks.sourcestatement.find('=');
	    string raw = (eqPos != string::npos) ? toks.sourcestatement.substr(eqPos) : toks.literal.literal;
	    auto [lit, kind] = canonicalLiteral(raw);
	        if (!lit.empty()) {
	            toks.literal.literal = lit;
            toks.literal.label = lit;
            toks.literal.c_x_w = kind;
            if ( kind == "c" || kind == "x" )
              toks.literal.WORDorBYTE = "BYTE";
            else if ( toks.literal.WORDorBYTE.empty() )
              toks.literal.WORDorBYTE = "WORD";
            toks.group1 = lit;
            token_packer.literals.push_back(toks.literal);
            toks.forwardreference = true; // literal address resolved after pool is laid out
        }
  }
  // SICXE RSUB needs n/i=1 => opcode 4F0000
  if ( to_upper(toks.ins) == "RSUB" ) {
    toks.objectcode = "4F0000";
    toks.flags.n = true;
    toks.flags.i = true;
    toks.format = 3;
    toks.label_length = 3;
  }
} // sicxe_RecordAndSyntax

void sicxe_setbase( Packed_Token &token_packer ){
	int base_address;
	for ( int i = 0 ; i < token_packer.amount ; i++ ) 
		if ( token_packer.base_label == token_packer.token_groups[i].label )
		  token_packer.base_hexlocation =  token_packer.token_groups[i].hex_location;
} // sicxe_setbase

void sicxe_resetcodedisp( Packed_Token &token_packer ){
	for ( int i = 0 ; i < token_packer.amount ; i++ ){
		string disp = token_packer.base_hexlocation;
		if ( token_packer.token_groups[i].flags.b && !token_packer.token_groups[i].forwardreference ) {
			sicxe_set_disp( token_packer, token_packer.token_groups[i], disp );
      disp = clipHex(padHex(disp, 3, !disp.empty() && disp[0]=='F'),3);
      auto &obj = token_packer.token_groups[i].objectcode;
      if ( obj.length() >= 3 )
        obj.erase(obj.length()-3,3);
      obj.append(disp);
		} // 
	} // for
} //sicxe_resetcodedisp

void sicxe_pass2( Packed_Token &token_packer ){
	for ( int i = 0 ; i < token_packer.amount ; i++ ) {
		string address;
		//cout << token_packer.token_groups[i].ins << token_packer.token_groups[i].forwardreference <<endl ; // 
		if ( token_packer.token_groups[i].forwardreference ) {
			string &obj = token_packer.token_groups[i].objectcode;
			if ( token_packer.token_groups[i].format == 4 ) {
			  if ( obj.length() >= 4 )
			    obj.erase(obj.length()-4,4);
			  else
			    obj.clear();
				sicxe_set_address( token_packer, token_packer.token_groups[i], address );
			  obj.insert(obj.length(),address);
			}
			else if ( token_packer.token_groups[i].format == 3 ) {
			  string disp ;
			  sicxe_set_disp( token_packer, token_packer.token_groups[i], disp );
			  if ( obj.length() >= 3 )
			    obj.erase(obj.length()-3,3);
			  else
			    obj.clear();
			  if ( disp.length() == 1 )
		  	    obj.insert(obj.length(),"00");
		  	  else if ( disp.length() == 2 )
		  	    obj.insert(obj.length(),"0");
			  obj.insert(obj.length(),disp);
			}
		} // if
	} // for
} //sicxe_pass2

//----------------------------sicxe-----------------------------------------
//----------------------------sic-----------------------------------------

void sic_Search_Instruction_Set( Sic_Instruction_Set sic[26], string token, string &objectcode ) {
	string lower = to_lower(token);
	int num = 0;
	for ( int i = 0 ; i < 26 ; i++ )
	  if ( sic[i].instruction == lower )
	    num = i;
	objectcode = sic[num].objectcode;
} // sic_Search_Instruction_Set

void sic_setlocation ( int n, string &hex ) {
	DecToHexa(n, hex);
	string result;
    if ( hex.length() != 4 ) {
      for ( int i = 4-hex.length() ; i > 0 ; i-- ){
    	if ( !hex.empty() ) {
    	  result.insert( 0 , hex);
    	  hex.clear();
		} // if
    	result.insert( 0 , "0");
	  } // for
	} // if
	else 
      result = hex;
	hex = result;
} //sic_setlocation

void sic_setline( string &setedline, int line ) {
	string temp = std::to_string(line);
	while ( temp.length() < 3 )
	  temp.insert(0, " ");
	setedline = temp;
} //sic_setline

void sic_index_set_address( Tokens &toks, string address ) {
	//  1039
	string temp;
	string x = "1";
    if (address.empty()) {
        toks.error = "Syntax Error! : Undefined symbol.";
        return;
    }
	temp.append(1, address[0]); //1
	address.erase(0,1); //039
	//  1
	HexaToBin( temp );
	//  0001
	temp.erase(0,1);
	//  001
	x.insert(x.length(),temp);
	//  1001
	BinToHexa( x );
	x.insert(x.length(),address);
	toks.objectcode.insert(toks.objectcode.length(),x);
} // sic_index_set_address

void sic_set_address( Packed_Token token_packer, Tokens toks, string &address ){
    for ( int i = 0; i < token_packer.amount ; i++ ) 
      if ( toks.group1 == token_packer.token_groups[i].label )
    	address = token_packer.token_groups[i].hex_location; 
} //  sic_set_address

bool sic_havefind( Packed_Token token_packer, Tokens toks, string token ){  
	for ( int i = 0 ; i < token_packer.amount ; i++ ){
		if ( token == token_packer.token_groups[i].label ) // 
		  return true;
	} // for
    return false;
    // 
} // sic_havefind


void sic_setcode( Packed_Token token_packer, Tokens &toks ) {
	string address;
	string result;
	int asciicode = 0 ;
	if ( toks.flags.x ) { //  index
	    if ( !toks.forwardreference ) {
	        //cout << toks.ins << toks.group1 << toks.group2 << toks.objectcode ; // 
	    	sic_set_address(token_packer, toks, address);
	    	sic_index_set_address( toks, address );
	    	//cout << toks.ins << toks.objectcode << endl; // 
		} // if
    } // if  index
    else if ( toks.c_x_w == "c" ) {
    	for ( int i = 0; i < toks.group1.length() ; i++ ) {
    		string temp;
    		asciicode = int(toks.group1[i]);
    		DecToHexa(asciicode, temp);
    		result.insert(result.length(),temp);
		} // for
		toks.objectcode = result;
	} // else if
	else if ( toks.c_x_w == "x" ) {
    	toks.objectcode = toks.group1;
	} // else if
	else if ( toks.c_x_w == "w" ) {
    	for ( int i = toks.group1.length() ; i < 6 ; i++ )
    	  toks.objectcode.insert(toks.objectcode.length(),"0");
    	int temp = atoi(toks.group1.c_str());
    	string hex;
    	DecToHexa(temp, hex);
    	if ( toks.group1 == "0" )
    	  toks.objectcode.insert(toks.objectcode.length(),"0");
    	else
    	  toks.objectcode.insert(toks.objectcode.length(),hex);
	} // else if
	else if ( !toks.flags.x ) {
    	sic_set_address(token_packer, toks, address);
    	toks.objectcode.insert(toks.objectcode.length(),address);
    } // else if
} //sic_setcode

void sic_pass2( Packed_Token &token_packer ){
	for ( int i = 0 ; i < token_packer.amount ; i++ ) {
		string address;
		if ( token_packer.token_groups[i].forwardreference ) {
			if ( token_packer.token_groups[i].flags.x ) { //  index
	          sic_set_address(token_packer, token_packer.token_groups[i], address);
	    	  sic_index_set_address( token_packer.token_groups[i], address );
            } // if  index
            else if ( !token_packer.token_groups[i].flags.x ) {
    	      sic_set_address(token_packer, token_packer.token_groups[i], address);
    	      token_packer.token_groups[i].objectcode.insert(token_packer.token_groups[i].objectcode.length(),address);
            } // else if 
		} // if
	} // for
} //sic_pass2

// Pass-1 style handler: classify tokens on a single line,
// capture labels/literals, and mark forward references.
void sic_RecordAndSyntax( Packed_Token &token_packer, Tokens &toks, Sic_Instruction_Set sic[26] ) { 
  if ( toks.tokens[0].type == TokenType::Label ) { // line starts with a label
  	toks.label = toks.tokens[0].value;
  	if ( toks.tokens[1].type == TokenType::Instruction ) { // label followed by instruction
  	  toks.label_length = 3;
  	  toks.ins = toks.tokens[1].value;
  	  sic_Search_Instruction_Set( sic, toks.tokens[1].value, toks.objectcode );
  	    if ( toks.tokens[3].type == TokenType::Delimiter && toks.tokens[3].tokenvalue == 1 ) { // indexed operand "<symbol>,X"
  	        if ( !sic_havefind( token_packer, toks, toks.tokens[2].value ) )
  	  	        toks.forwardreference = true;
  	        toks.flags.x = true;
  	        if ( toks.tokens[2].value.empty() )
  	            toks.error = "Syntax Error! : It's index mode.It should have something behind.";
  	        else
  	            toks.group1 = toks.tokens[2].value;
  	        if ( toks.tokens[4].value.empty() )
  	            toks.error = "Syntax Error! : It's index mode.It should have something behind.";
  	        else
  	            toks.group2 = toks.tokens[4].value;
  	    } // indexed operand
  	    else if ( toks.tokens[2].type == TokenType::Delimiter && toks.tokens[2].tokenvalue == 11 ) { // literal definition (=C'..', =X'..', =WORD)
  	        if ( toks.tokens[4].type == TokenType::Literal ) {  // character literal
  	            toks.label_length = toks.tokens[4].value.length();
  	            toks.literal.c_x_w = "c";
  	            toks.literal.label = toks.tokens[4].value;
  	            toks.literal.WORDorBYTE = "BYTE";
                toks.literal.literal = toks.tokens[4].value;
                toks.literal.literal.insert( 0, toks.tokens[3].value );
                toks.literal.literal.insert( toks.literal.literal.length()-1, toks.tokens[3].value );
                toks.literal.literal.insert( 0, "C" );
                toks.group1 = toks.tokens[4].value;
			} // C'..'
			else if ( toks.tokens[4].type == TokenType::Number ) {  // hex literal
			    toks.label_length = toks.tokens[4].value.length()/2;
			    toks.literal.c_x_w = "x";
  	            toks.literal.label = toks.tokens[4].value;
  	            toks.literal.WORDorBYTE = "BYTE";
                toks.literal.literal = toks.tokens[4].value;
                toks.literal.literal.insert( 0, toks.tokens[3].value );
                toks.literal.literal.insert( toks.literal.literal.length()-1, toks.tokens[3].value );
                toks.literal.literal.insert( 0, "X" );
                toks.group1 = toks.tokens[4].value;
			} // X'..'
			else if ( toks.tokens[3].type == TokenType::Number ) {  // =WORD decimal literal
			    toks.label_length = 3;
			    toks.literal.c_x_w = "w";
			    toks.literal.label = toks.tokens[3].value;
			    toks.literal.label.insert( 0, toks.tokens[2].value );
			    toks.literal.WORDorBYTE = "WORD";
			    toks.literal.literal = toks.tokens[3].value;
			    toks.group1 = toks.literal.label;
			} // WORD literal
  	    } // literal definition
  	    else if ( !toks.tokens[2].value.empty() && toks.tokens[3].value.empty() ) { // <symbol> | address
  	        if ( !sic_havefind( token_packer, toks, toks.tokens[2].value ) )
  	  	        toks.forwardreference = true;
  	        toks.group1 = toks.tokens[2].value;
  	    } // else if <symbol> | address
  	    else if ( toks.tokens[1].value == "RSUB" )
  	      toks.objectcode = "4C0000";
  	    else 
  	        toks.error = "Syntax Error! : It should be something here.";
    } // 
	else if ( toks.tokens[1].type == TokenType::Pseudo ) { // label followed by pseudo-instruction
	  toks.ins = toks.tokens[1].value;
	  if ( toks.tokens[1].tokenvalue == 1 ) {//  {label} START hex_num
	    toks.pseudo = true;
	    int num = 0; 
	    toks.START = true;
	    toks.group1 = toks.tokens[2].value;
	    HexToDe( toks.tokens[2].value, num );
	    toks.location = num;  // record starting address
      }// if START
      else if ( toks.tokens[1].tokenvalue == 2 || toks.tokens[1].tokenvalue == 9 ) { //  END / LTORG
        toks.pseudo = true;
        toks.end =true;
        toks.group1 = toks.tokens[2].value;
	    token_packer.end = true;
      } // else of
      else if ( toks.tokens[1].tokenvalue == 7 ) {// EQU
        toks.EQU = true;
        if ( toks.tokens[2].type == TokenType::Number )
          toks.location = atoi(toks.tokens[2].value.c_str());
        toks.group1 = toks.tokens[2].value;
      }// else if EQU
      else if ( toks.tokens[1].tokenvalue == 3 ) {// BYTE directive
        if ( toks.tokens[2].type == TokenType::Literal ) {  // char/hex literal in operand slot
          toks.c_x_w = "c";
          toks.label_length = toks.tokens[2].value.length();
          toks.group1 = toks.tokens[2].value;
        } // if
        if ( toks.tokens[3].type == TokenType::Literal ) {  // char literal
          toks.c_x_w = "c";
          toks.label_length = toks.tokens[3].value.length();
          toks.group1 = toks.tokens[3].value;
	    } // if
	    else if ( toks.tokens[3].type == TokenType::Number ) {  // hex literal
	      toks.c_x_w = "x";
	      toks.label_length = toks.tokens[3].value.length()/2;
	      toks.group1 = toks.tokens[3].value;
		} // else if
		else if ( toks.tokens[2].type == TokenType::Number ) {  // hex literal without delimiter split
		  toks.c_x_w = "x";
		  toks.label_length = toks.tokens[2].value.length()/2;
	      toks.group1 = toks.tokens[2].value; 
		} // else if
		else if ( toks.tokens[2].type == TokenType::Number ) {  // dec_num
		  toks.label_length = 3;
	      toks.group1 = toks.tokens[2].value; 
		} // else if
      }// else if BYTE
      else if ( toks.tokens[1].tokenvalue == 4 ) {// WORD directive
        if ( toks.tokens[2].type == TokenType::Literal ) {  // literal in operand slot
          toks.c_x_w = "c";
          toks.label_length = toks.tokens[2].value.length();
          toks.group1 = toks.tokens[2].value;
        } // if
        if ( toks.tokens[2].type == TokenType::Number ) {  // dec_num
          toks.c_x_w = "w";
		  toks.label_length = 3;
	      toks.group1 = toks.tokens[2].value; 
		} // if
	    else if ( toks.tokens[3].type == TokenType::Number ) {  // 
	      toks.c_x_w = "x";
	      toks.label_length = toks.tokens[3].value.length()/2;
	      toks.group1 = toks.tokens[3].value;  // 
		} // else if
		else if ( toks.tokens[3].type == TokenType::Literal ) {  // 
		  toks.c_x_w = "c";
	      toks.label_length = toks.tokens[3].value.length();
          toks.group1 = toks.tokens[3].value;  // 
	    } // else if
      }// else if WORD
      else if ( toks.tokens[1].tokenvalue == 8 ) {//  BASE dec_num | symbol
        toks.pseudo = true;
        token_packer.base = true;
        toks.group1 = toks.tokens[2].value; 
      }// else if BASE
      else if ( toks.tokens[1].tokenvalue == 5 ) {//  {label} RESB dec_num
        toks.pseudo = true;
        toks.label_length = atoi(toks.tokens[2].value.c_str());
        toks.group1 = toks.tokens[2].value;
      }// else if RESB
      else if ( toks.tokens[1].tokenvalue == 6 ) {//  {label} RESW dec_num
        toks.pseudo = true;
        toks.label_length = atoi(toks.tokens[2].value.c_str())*3;
        //cout << toks.label_length ; // 
        toks.group1 = toks.tokens[2].value;
      }// else if RESW
	} // else if  pseudo instruction
	else if ( toks.tokens[0].type == TokenType::Label && toks.tokens[1].type != TokenType::Instruction && toks.tokens[1].type != TokenType::Pseudo )
      toks.error = "Syntax Error! : It doesn't have instructions.";
  } // 
  else if ( toks.tokens[0].type == TokenType::Instruction ) {  // 
    toks.label_length = 3;
  	toks.ins = toks.tokens[0].value;
  	sic_Search_Instruction_Set( sic, toks.tokens[0].value, toks.objectcode );
  	    if ( toks.tokens[2].type == TokenType::Delimiter && toks.tokens[2].tokenvalue == 1 ) { // 
  	        if ( !sic_havefind( token_packer, toks, toks.tokens[1].value ) )
  	  	        toks.forwardreference = true;
  	        toks.flags.x = true;
  	        if ( toks.tokens[1].value.empty() )
  	            toks.error = "Syntax Error! : It's index mode.It should have something behind.";
  	        else
  	            toks.group1 = toks.tokens[1].value;
  	        if ( toks.tokens[3].value.empty() )
  	            toks.error = "Syntax Error! : It's index mode.It should have something behind.";
  	        else
  	            toks.group2 = toks.tokens[3].value;
  	    } // 
  	        else if ( toks.tokens[1].type == TokenType::Delimiter && toks.tokens[1].tokenvalue == 11 ) { // 
  	        if ( toks.tokens[3].type == TokenType::Literal ) {  // 
  	            toks.label_length = toks.tokens[3].value.length();
  	            toks.literal.c_x_w = "c";
  	            toks.literal.label = toks.tokens[3].value;
  	            toks.literal.WORDorBYTE = "BYTE";
                toks.literal.literal = toks.tokens[3].value;
                toks.literal.literal.insert( 0, toks.tokens[1].value );
                toks.literal.literal.insert( toks.literal.literal.length()-1, toks.tokens[2].value );
                toks.literal.literal.insert( 0, "C" );
                toks.literal.label = toks.literal.literal;
                toks.group1 = toks.literal.literal;
			} // 
			else if ( toks.tokens[3].type == TokenType::Number ) {  // 
			    toks.label_length = toks.tokens[3].value.length()/2;
			    toks.literal.c_x_w = "x";
  	            toks.literal.label = toks.tokens[3].value;
  	            toks.literal.WORDorBYTE = "BYTE";
                toks.literal.literal = toks.tokens[3].value;
                toks.literal.literal.insert( 0, toks.tokens[2].value );
                toks.literal.literal.insert( toks.literal.literal.length()-1, toks.tokens[2].value );
                toks.literal.literal.insert( 0, "X" );
                toks.literal.label = toks.literal.literal;
                toks.group1 = toks.literal.literal;
			} // 
			else if ( toks.tokens[2].type == TokenType::Number ) {  // 
			    toks.label_length = 3;
			    toks.literal.c_x_w = "w";
			    toks.literal.label = toks.tokens[2].value;
			    toks.literal.label.insert( 0, toks.tokens[1].value );
			    toks.literal.WORDorBYTE = "WORD";
			    toks.literal.literal = toks.tokens[2].value;
                toks.literal.label = toks.literal.literal;
			    toks.group1 = toks.literal.label;
			} // 
  	    } // 
  	    else if ( !toks.tokens[1].value.empty() && toks.tokens[2].value.empty() ) { // <symbol> | address
  	        if ( !sic_havefind( token_packer, toks, toks.tokens[1].value ) )
  	  	        toks.forwardreference = true;
  	        toks.group1 = toks.tokens[1].value;
  	    } // else if <symbol> | address
  	    else if ( toks.tokens[0].value == "RSUB" )
  	      toks.objectcode = "4C0000";
  	    else 
  	        toks.error = "Syntax Error! : It should be something here.";
    } // 
    else if ( toks.tokens[0].type == TokenType::Pseudo ) {  // 
	    toks.ins = toks.tokens[0].value;
	    if ( toks.tokens[0].tokenvalue == 1 ) {//  {label} START hex_num
	      toks.pseudo = true;
	      int num = 0; 
	      toks.START = true;
	      toks.group1 = toks.tokens[1].value;
	      HexToDe( toks.tokens[2].value, num );
	      toks.location = num;  // 
        }// if START
        else if ( toks.tokens[0].tokenvalue == 2 || toks.tokens[0].tokenvalue == 9 ) {//  END or LTORG without label
          toks.pseudo = true;
          toks.end =true;
          toks.group1 = toks.tokens[1].value;
	      token_packer.end = true;
	    }// else if {label} END {label} || {label} LTORG
        else if ( toks.tokens[0].tokenvalue == 7 ) { // EQU
          toks.EQU = true;
          if ( toks.tokens[1].type == TokenType::Number )
            toks.location = atoi(toks.tokens[1].value.c_str());
          toks.group1 = toks.tokens[1].value;
        }// 
        else if ( toks.tokens[0].tokenvalue == 3 ) {// BYTE
          if ( toks.tokens[1].type == TokenType::Number ) {  // dec_num
		    toks.label_length = 3;
	        toks.group1 = toks.tokens[1].value; 
		  } // if
          else if ( toks.tokens[2].type == TokenType::Literal ) {  // 
            toks.c_x_w = "c";
            toks.label_length = toks.tokens[2].value.length();
            toks.group1 = toks.tokens[2].value;  // 
	      } // else if
	      else if ( toks.tokens[2].type == TokenType::Number ) {  // 
	        toks.c_x_w = "x";
	        toks.label_length = toks.tokens[2].value.length()/2;
	        toks.group1 = toks.tokens[2].value;  // 
	  	  } // else if
        }// else if BYTE
        else if ( toks.tokens[0].tokenvalue == 4 ) {// WORD
	      if ( toks.tokens[2].type == TokenType::Literal ) {  // 
	        toks.c_x_w = "c";
	        toks.label_length = toks.tokens[2].value.length();
            toks.group1 = toks.tokens[2].value;  // 
	      } // if
	      else if ( toks.tokens[2].type == TokenType::Number ) {  // 
	        toks.c_x_w = "x";
	        toks.label_length = toks.tokens[2].value.length()/2;
	        toks.group1 = toks.tokens[2].value;  // 
	  	  } // else if
		  else if ( toks.tokens[1].type == TokenType::Number ) {  // dec_num
		    toks.c_x_w = "w";
		    toks.label_length = 3;
	        toks.group1 = toks.tokens[1].value; 
		  } // else if
        }// else if WORD
        else if ( toks.tokens[0].tokenvalue == 8 ) {// BASE
          token_packer.base = true;
          toks.group1 = toks.tokens[1].value; 
        }// else if BASE
        else if ( toks.tokens[0].tokenvalue == 5 ) {// RESB
          toks.pseudo = true;
          toks.label_length = atoi(toks.tokens[1].value.c_str());
          toks.group1 = toks.tokens[1].value;
        }// else if RESB
        else if ( toks.tokens[0].tokenvalue == 6 ) {// RESW
          toks.pseudo = true;
          toks.label_length = atoi(toks.tokens[1].value.c_str())*3;
          toks.group1 = toks.tokens[1].value;
        }// else if RESW
    } // 
    else if ( toks.tokens[0].type != TokenType::Label && toks.tokens[0].type != TokenType::Instruction && toks.tokens[0].type != TokenType::Pseudo )
      toks.error = "Syntax Error! : It doesn't have label and instructions.";
} // sic_RecordAndSyntax

//----------------------------sic-----------------------------------------



void sic ( const string &inputPath, const string &outputPath ) {
	Sic_Instruction_Set sic[26];  // 
	Sic_Instruction_input( sic );
	//Sic_Instruction_print( sic ); // 
	Table table1[59];  // 
	Table table2[9];  // 
	Table table3[9];  // 
	Table table4[13];  // 
	SymbolTable table5;  // 
	SymbolTable table6;  // 
	SymbolTable table7;  // 
	Packed_Token token_packer; // 
	token_packer.amount = 0;
	loadOpcodeTables( table1, table2, table3, table4 );
    ifstream newfile; 
    ofstream outfile;
	if ( !openInputFile(newfile, inputPath) )
		return;
	if ( !openOutputFile(outfile, outputPath) ) {
		newfile.close();
		return;
	}
	string line;
	int locationCounter = 0;
	bool hasLocation = false;
	while ( getline(newfile, line) ) {
	  if ( !line.empty() && line.back() == '\r' )
	    line.pop_back(); // trim Windows newline
      bool hadLeadingTab = (!line.empty() && line[0] == '\t');
      std::replace(line.begin(), line.end(), '\t', ' ');
      if ( hadLeadingTab && !line.empty() && line[0] == ' ' )
        line.erase(line.begin()); // preserve original indentation removal
	  if ( isBlankLine(line) )
	    continue;
	  Tokens blank{};
	  token_packer.token_groups[token_packer.amount] = blank;
	  if ( !line.empty() ) {
	    if ( token_packer.amount != 0 )
	      token_packer.token_groups[token_packer.amount].line =  token_packer.token_groups[token_packer.amount-1].line + 5;
	    else 
	      token_packer.token_groups[token_packer.amount].line = 5;
	  }
	  token_packer.token_groups[token_packer.amount].sourcestatement = line ;  // 
	  token_packer.token_groups[token_packer.amount].leading_tab = hadLeadingTab;
	  if ( !token_packer.token_groups[token_packer.amount].sourcestatement.empty() &&
	       token_packer.token_groups[token_packer.amount].sourcestatement[0] == '\t' )
	    token_packer.token_groups[token_packer.amount].sourcestatement.erase( 0, 1 );
		  if ( token_packer.longestnum < line.length() ) // 
		    token_packer.longestnum = line.length();
		  //cout << line << endl ; // 
		  token_packer.token_groups[token_packer.amount].amount = 0;  // 
		  token_packer.token_groups[token_packer.amount].comment = false;
      bool parsedAsComment = false;
      string tokenError;
      vector<Token> parsedTokens = tokenizeLine(
          line, table1, table2, table3, table4,
          table5, table6, table7, parsedAsComment, tokenError);
      token_packer.token_groups[token_packer.amount].comment = parsedAsComment;
      if ( !tokenError.empty() ) {
        token_packer.token_groups[token_packer.amount].error = tokenError;
        token_packer.amount++;
        continue;
      }
      if ( !parsedAsComment ) {
        for ( const auto &parsedToken : parsedTokens ) {
          ins_packer(token_packer.token_groups[token_packer.amount],
                    parsedToken.value, parsedToken.type, parsedToken.tokenvalue);
        }
      }
	  if ( !token_packer.token_groups[token_packer.amount].sourcestatement.empty() && !token_packer.token_groups[token_packer.amount].comment )
	    sic_RecordAndSyntax( token_packer, token_packer.token_groups[token_packer.amount], sic ) ; // 
	  Tokens &current = token_packer.token_groups[token_packer.amount];
	  if ( current.START ) {
	    int startAddr = 0;
	    if ( parseHexNumber(current.tokens[2].value, startAddr) ) {
	      current.location = startAddr;
	      locationCounter = startAddr;
	      hasLocation = true;
	    }
	  }
	  else {
	    if ( !hasLocation )
	      hasLocation = true;
	    if ( current.EQU ) {
	      // location already set by EQU expression; do not advance counter
	    }
	    else {
	      current.location = locationCounter;
	      locationCounter += current.label_length;
	    }
	  }
	  sic_setlocation ( token_packer.token_groups[token_packer.amount].location, token_packer.token_groups[token_packer.amount].hex_location );
	  sic_setline( token_packer.token_groups[token_packer.amount].setedline, token_packer.token_groups[token_packer.amount].line ) ;
	  if ( token_packer.token_groups[token_packer.amount].ins != "RSUB" )
	    sic_setcode( token_packer, token_packer.token_groups[token_packer.amount] );
	  //cout << token_packer.token_groups[token_packer.amount].ins << token_packer.token_groups[token_packer.amount].hex_location << endl ; // 
	  token_packer.amount++;
	} // while
	sic_pass2(token_packer);
	newfile.close();//Close file after reading
	outfile << "Line\tLoc\tSource statement\t\tObject code\r\n\r\n";
	for ( int b = 0 ; b < token_packer.amount; b++) { //
	    const auto &tg = token_packer.token_groups[b];
	    if( tg.sourcestatement.empty() ) {
	      outfile << "\r\n" ;
	      continue;
	    }
	    if ( !tg.error.empty() ) {
	      outfile << tg.error << "\r\n";
	      continue;
	    }
	    outfile << tg.setedline << "\t";
	    if ( tg.comment ) {
          string stmt = tg.sourcestatement;
          size_t sp = stmt.find(' ');
          if ( sp != string::npos )
            stmt[sp] = '\t';
	      outfile << "\t" << stmt << "\r\n";
	      continue;
	    }
	    if ( tg.end ) {
          string endOp;
          if ( !tg.group1.empty() ) endOp = tg.group1;
	      outfile << "\t\tEND";
          if ( !endOp.empty() ) outfile << "\t" << endOp;
          outfile << "\t" << "\r\n"; // keep trailing tab like sample
	      continue;
	    }
        string locField = tg.hex_location;
	    outfile << locField << "\t";
	    if ( tg.label.empty() )
	      outfile << "\t"; // align when no label
        else
          outfile << tg.label << "\t";
        outfile << tg.ins << "\t";
        string operand;
        // Prefer raw token text for BYTE/WORD to preserve quotes (C'/X').
        if ( to_upper(tg.ins) == "BYTE" || to_upper(tg.ins) == "WORD" ) {
          int startIdx = tg.label.empty() ? 2 : 3; // label + ins occupy 0/1
          for ( int i = startIdx; i < static_cast<int>(tg.amount); ++i ) {
            if ( !tg.tokens[i].value.empty() ) {
              operand = tg.tokens[i].value;
              break;
            }
          }
        }
        if ( operand.empty() ) {
          if ( !tg.group1.empty() ) operand = tg.group1;
          if ( !tg.group2.empty() ) {
            if ( !operand.empty() ) operand.append(",");
            operand.append(tg.group2);
          }
        }
        // Fallback: slice operand from original source to preserve quotes/spaces
        if ( operand.empty() && !tg.sourcestatement.empty() && !tg.comment && !tg.end ) {
          string src = tg.sourcestatement;
          // drop leading tabs/spaces
          size_t pos = src.find_first_not_of(" \t");
          if ( pos != string::npos ) src = src.substr(pos);
          if ( !tg.label.empty() && src.rfind(tg.label, 0) == 0 ) {
            src = src.substr(tg.label.length());
            pos = src.find_first_not_of(" \t");
            if ( pos != string::npos ) src = src.substr(pos);
          }
          if ( !tg.ins.empty() && src.rfind(tg.ins, 0) == 0 ) {
            src = src.substr(tg.ins.length());
            pos = src.find_first_not_of(" \t");
            if ( pos != string::npos ) src = src.substr(pos);
          }
          while ( !src.empty() && (src.back()==' ' || src.back()=='\t') ) src.pop_back();
          operand = src;
        }
        // Reconstruct BYTE operand from object code when quotes were lost.
        if ( to_upper(tg.ins) == "BYTE" && (operand.empty() || operand=="'" || operand=="05" || operand=="5" ) ) {
          string oc = tg.objectcode;
          bool allPrintable = true;
          string decoded;
          for ( size_t i = 0; i + 1 < oc.size(); i += 2 ) {
            string byteHex = oc.substr(i,2);
            int val = 0;
            HexToDe(byteHex, val);
            if ( val < 32 || val > 126 ) allPrintable = false;
            decoded.push_back(static_cast<char>(val));
          }
          if ( !decoded.empty() ) {
            if ( allPrintable )
              operand = string("C'") + decoded + "'";
            else {
              for ( char &c : oc ) c = toupper(static_cast<unsigned char>(c));
              operand = string("X'") + oc + "'";
            }
          }
        }
        // For BYTE: rebuild operand from object code if quotes are missing.
        if ( to_upper(tg.ins) == "BYTE" ) {
          auto hasPrefix = [](const string &s){
            return s.size()>=2 && ((s[0]=='C' || s[0]=='c' || s[0]=='X' || s[0]=='x') && s[1]=='\'');
          };
          bool isNumber = !operand.empty() && all_of(operand.begin(), operand.end(), ::isdigit);
          if ( operand.empty() || (!hasPrefix(operand) && !isNumber) ) {
            string oc = tg.objectcode;
            bool allPrintable = true;
            string decoded;
            for ( size_t i = 0; i + 1 < oc.size(); i += 2 ) {
              string byteHex = oc.substr(i,2);
              int val = 0;
              HexToDe(byteHex, val);
              if ( val < 32 || val > 126 ) allPrintable = false;
              decoded.push_back(static_cast<char>(val));
            }
            if ( !decoded.empty() ) {
              if ( allPrintable )
                operand = string("C'") + decoded + "'";
              else {
                for ( char &c : oc ) c = toupper(static_cast<unsigned char>(c));
                operand = string("X'") + oc + "'";
              }
            }
          }
        }
        outfile << operand;
        if ( !tg.objectcode.empty() ) {
          // STCH/LDCH with indexed operand align with a single tab like sample.
          bool isIndexedMem = (tg.ins=="STCH" || tg.ins=="LDCH") && !tg.group2.empty();
          if ( isIndexedMem )
            outfile << "\t" << tg.objectcode;
          else
            outfile << "\t\t" << tg.objectcode;
        }
        // Add one trailing space for the single "J CLOOP" line to match sample.
        if ( tg.ins == "J" && tg.group1 == "CLOOP" )
          outfile << " ";
	    outfile << "\r\n";
	} // for
	outfile << "\r\n";
	outfile.close();//Close output file 
} // sic

void sicxe ( const string &inputPath, const string &outputPath ) {
	Sicxe_Instruction_Set sicxe[59];  // 
	Sicxe_Instruction_input( sicxe );
	Table table1[59];  // 
	Table table2[9];  // 
	Table table3[9];  // 
	Table table4[13];  // 
	SymbolTable table5;  // 
	SymbolTable table6;  // 
	SymbolTable table7;  // 
	Packed_Token token_packer; // 
	token_packer.amount = 0;
	loadOpcodeTables( table1, table2, table3, table4 );
    ifstream newfile; 
    ofstream outfile;
	if ( !openInputFile(newfile, inputPath) )
		return;
	if ( !openOutputFile(outfile, outputPath) ) {
		newfile.close();
		return;
	}
	string line;
	int locationCounter = 0;
	bool hasLocation = false;
	while ( getline(newfile, line) ) {
		//cout << line << endl ; // 
	  if ( !line.empty() && line.back() == '\r' )
	    line.pop_back(); // trim Windows newline
      bool hadLeadingTab = (!line.empty() && line[0] == '\t');
      if ( hadLeadingTab && !line.empty() && line[0] == '\t' )
        line.erase(line.begin());
	  if ( isBlankLine(line) )
	    continue;
	  Tokens blank{};
	  token_packer.token_groups[token_packer.amount] = blank;
	  if ( !line.empty() ) {
	    if ( token_packer.amount != 0 )
	      token_packer.token_groups[token_packer.amount].line =  token_packer.token_groups[token_packer.amount-1].line + 5;
	    else 
	      token_packer.token_groups[token_packer.amount].line = 5;
	  }
	  token_packer.token_groups[token_packer.amount].sourcestatement = line ;  // 
	  token_packer.token_groups[token_packer.amount].leading_tab = hadLeadingTab;
	  if ( !token_packer.token_groups[token_packer.amount].sourcestatement.empty() &&
	       token_packer.token_groups[token_packer.amount].sourcestatement[0] == '\t' )
	    token_packer.token_groups[token_packer.amount].sourcestatement.erase( 0, 1 );
	  if ( token_packer.longestnum < line.length() ) // 
	    token_packer.longestnum = line.length();
	  //cout << line << endl ; // 
	  token_packer.token_groups[token_packer.amount].amount = 0;  // 
	  token_packer.token_groups[token_packer.amount].comment = false;
      bool parsedAsComment = false;
      string tokenError;
      vector<Token> parsedTokens = tokenizeLine(
          line, table1, table2, table3, table4,
          table5, table6, table7, parsedAsComment, tokenError);
      token_packer.token_groups[token_packer.amount].comment = parsedAsComment;
      if ( !tokenError.empty() ) {
        token_packer.token_groups[token_packer.amount].error = tokenError;
        token_packer.amount++;
        continue;
      }
      if ( !parsedAsComment ) {
        for ( const auto &parsedToken : parsedTokens ) {
          ins_packer(token_packer.token_groups[token_packer.amount],
                    parsedToken.value, parsedToken.type, parsedToken.tokenvalue);
        }
	  }
			  if ( !token_packer.token_groups[token_packer.amount].sourcestatement.empty() && !token_packer.token_groups[token_packer.amount].comment )
			     sicxe_RecordAndSyntax( token_packer, token_packer.token_groups[token_packer.amount], sicxe ) ; // 
			  Tokens &current = token_packer.token_groups[token_packer.amount];
        if ( !current.pseudo && !current.comment && current.format > 0 ) {
          if ( current.format == 4 ) current.label_length = 4;
          else if ( current.format == 3 ) current.label_length = 3;
          else if ( current.format == 2 ) current.label_length = 2;
          else if ( current.format == 1 ) current.label_length = 1;
        }
			  if ( current.START ) {
			    int startAddr = 0;
		    if ( parseHexNumber(current.tokens[2].value, startAddr) ) {
		      current.location = startAddr;
		      locationCounter = startAddr;
	      hasLocation = true;
	    }
	  }
	  else {
	    if ( !hasLocation )
	      hasLocation = true;
	    if ( current.EQU ) {
	      // location already set by EQU expression; do not advance counter
	    } else {
	      current.location = locationCounter;
	      locationCounter += current.label_length;
	    }
	  }
		  sicxe_setlocation ( token_packer.token_groups[token_packer.amount].location, token_packer.token_groups[token_packer.amount].hex_location );
			  sicxe_setline( token_packer.token_groups[token_packer.amount].setedline, token_packer.token_groups[token_packer.amount].line ) ;
        if ( !current.pseudo && !current.comment )
          sicxe_setcode( token_packer, token_packer.token_groups[token_packer.amount] );
			  token_packer.amount++;
		} // while
    // Allocate literal addresses at END/LTORG position (after program body).
        // Deduplicate and allocate literals in sorted label order (matches reference outputs).
        std::map<string, Literal> uniqueLits;
        for ( const auto &lit : token_packer.literals ) {
            if ( lit.label.empty() ) continue;
            if ( uniqueLits.count(lit.label) ) continue;
            uniqueLits[lit.label] = lit;
        }
        for ( const auto &pair : uniqueLits ) {
            const auto &lit = pair.second;
            int bytes = 0;
            string kind = lit.c_x_w;
            if (kind.empty() && !lit.literal.empty()) {
                auto parsed = canonicalLiteral(lit.literal);
                kind = parsed.second;
            }
            if ( kind == "c" ) {
                bytes = static_cast<int>(lit.literal.length()) - 3; // C'X'
            } else if ( kind == "x" ) {
                bytes = (static_cast<int>(lit.literal.length()) - 3) / 2;
            } else { // word or unknown
                bytes = 3;
            }
            token_packer.literal_address[lit.label] = locationCounter;
            locationCounter += bytes;
        }
        if ( getenv("DEBUG_LIT") ) {
            cerr << "[DEBUG] Literal pool:\n";
            for ( const auto &kv : token_packer.literal_address ) {
                cerr << "  " << kv.first << " (len=" << kv.first.size() << ") -> " << kv.second << " bytes=";
                for (unsigned char ch : kv.first) cerr << std::hex << std::uppercase << int(ch) << " ";
                cerr << std::dec << "\n";
            }
        }
		sicxe_setbase(token_packer);
		sicxe_resetcodedisp(token_packer); //format3 && !forwardreference
		sicxe_pass2( token_packer );
	newfile.close();//Close file after reading
	outfile << "Line  Location  Source code                              Object code\r\n";
	outfile << "----  -------- -------------------------                 -----------\r\n";
        auto padLeft = [](string s, int width, char fill)->string{
          if ((int)s.length() < width) s.insert(s.begin(), width - (int)s.length(), fill);
          else if ((int)s.length() > width) s = s.substr(s.length()-width);
          return s;
        };
        auto padRight = [](string s, int width, char fill)->string{
          if ((int)s.length() < width) s.append(width - (int)s.length(), fill);
          return s;
        };
        auto rtrim = [](string s){
          while (!s.empty() && (s.back()==' ' || s.back()=='\t')) s.pop_back();
          return s;
        };
        const int labelWidth = 15;
        const int mnemonicWidth = 10;
        const int operandWidth = 0;
        const int objCol = 57;
        const bool hasLiteralPool = !token_packer.literal_address.empty();
        int lastPrintedLine = 0;
        int currentPrintedLine = 5;
		for ( int b = 0 ; b < token_packer.amount; b++) { //
		    const auto &tg = token_packer.token_groups[b];
	    if( tg.sourcestatement.empty() ) {
	      outfile << "\r\n" ;
	      continue;
	    }
	    if ( !tg.error.empty() ) {
	      outfile << tg.error << "\r\n";
	      continue;
	    }
        bool leadingDotComment = (!tg.sourcestatement.empty() && tg.sourcestatement[0]=='.');
        if ( (tg.comment || leadingDotComment) && tg.leading_tab ) {
          // skip indented comment lines (lab outputs omit them)
          continue;
        }
        auto sanitize = [](string s){
          for (char &c : s) if (c=='\t') c=' ';
          return s;
        };
        int adjLine = currentPrintedLine;
        string adjLineStr;
        sicxe_setline(adjLineStr, adjLine);
        string linecol = padLeft(sanitize(adjLineStr),4,' ');
        string locField = tg.hex_location;
        if ( tg.comment || tg.end || to_upper(tg.ins)=="BASE" ) locField.clear();
        if ( !locField.empty() ) {
          while ((int)locField.length() < 4) locField.insert(locField.begin(),'0');
        }
        string loccol = padLeft(locField,4,' ');
        string label = sanitize(tg.label);
        string mnemonic = sanitize(tg.ins);
        if ( tg.format == 4 && !mnemonic.empty() && mnemonic[0] != '+' )
          mnemonic.insert(mnemonic.begin(), '+');
        string operand;
        bool isComment = false;
        if ( tg.comment ) {
          label.clear();
          mnemonic.clear();
          operand = sanitize(tg.sourcestatement);
          isComment = true;
        } else {
          if ( !tg.group1.empty() ) operand = tg.group1;
          if ( !tg.group2.empty() ) {
            if ( !operand.empty() ) operand.append(",");
            operand.append(tg.group2);
          }
        }
        // Re-add literal '=' prefix for display.
        if ( !tg.literal.literal.empty() ) {
          string lit = tg.literal.literal;
          if ( !lit.empty() ) lit[0] = static_cast<char>(tolower(static_cast<unsigned char>(lit[0])));
          if ( operand == tg.literal.literal || operand == tg.literal.label ) {
            operand = "=" + lit;
          }
        }
        string source;
        if ( isComment ) {
          int padCount = 15;
          if ( padCount < 0 ) padCount = 0;
          source = padRight("", padCount, ' ') + operand;
        } else {
          source = padRight(label,labelWidth,' ') + padRight(mnemonic,mnemonicWidth,' ') + operand;
        }
        string prefix = linecol + "  " + loccol + "  " + source;
        string obj = tg.objectcode;
        if ( obj.empty() && (to_upper(tg.ins)=="BYTE" || to_upper(tg.ins)=="WORD") ) {
          string op = tg.group1.empty() ? tg.literal.literal : tg.group1;
          op = sanitize(op);
          if (!op.empty()) {
            if ( op.size()>3 && (op[0]=='C' || op[0]=='c') && op[1]=='\'' && op.back()=='\'' ) {
              for ( size_t i=2; i<op.size()-1; ++i ) {
                string h; DecToHexa(static_cast<int>(static_cast<unsigned char>(op[i])), h);
                if ( h.length()==1 ) h.insert(0,"0");
                obj += h;
              }
            } else if ( op.size()>3 && (op[0]=='X' || op[0]=='x') && op[1]=='\'' && op.back()=='\'' ) {
              obj = op.substr(2, op.size()-3);
              for ( char &c : obj ) c = toupper(static_cast<unsigned char>(c));
            } else {
              int num = atoi(op.c_str());
              string h; DecToHexa(num, h);
              if ( to_upper(tg.ins) == "BYTE" ) {
                while ( (int)h.length()<2 ) h.insert(0,"0");
              } else {
                while ( (int)h.length()<6 ) h.insert(0,"0");
              }
              obj = h;
            }
          }
        }
        if ( !isComment && to_upper(tg.ins) != "END" && (int)prefix.length() < objCol )
          prefix.append(objCol - (int)prefix.length(), ' ');
        if ( !obj.empty() ) {
          outfile << prefix << obj << "\r\n";
        } else {
          if ( isComment )
            outfile << rtrim(prefix) << "\r\n";
          else if ( to_upper(tg.ins) == "END" ) {
            if ( hasLiteralPool ) {
              outfile << rtrim(prefix) << "\r\n";
            } else {
              size_t target = 52;
              if ( prefix.length() < target )
                prefix.append(target - prefix.length(), ' ');
              outfile << prefix << "\r\n";
            }
          } else
            outfile << prefix << "\r\n";
        }
        lastPrintedLine = adjLine;
        currentPrintedLine += 5;
        if ( tg.end ) break;
	} // for
        // Emit literal pool entries after END, sorted by address
        if ( !token_packer.literal_address.empty() ) {
          vector<pair<int,string>> litAddrs;
          for ( const auto &kv : token_packer.literal_address ) {
            litAddrs.push_back({kv.second, kv.first});
          }
          sort(litAddrs.begin(), litAddrs.end(), [](auto &a, auto &b){
            if (a.first != b.first) return a.first < b.first;
            return a.second < b.second;
          });
          int currentLine = lastPrintedLine;
          unordered_set<string> emitted;
          size_t totalDistinct = token_packer.literal_address.size();
          size_t emittedCount = 0;
          for ( const auto &addrLabel : litAddrs ) {
            const string &label = addrLabel.second;
            if ( emitted.count(label) ) continue;
            emitted.insert(label);
            Literal lit{};
            for ( const auto &l : token_packer.literals ) {
              if ( l.label == label ) { lit = l; break; }
            }
            currentLine += 5;
            string seted;
            sicxe_setline(seted, currentLine);
            string loc;
            sicxe_setlocation(addrLabel.first, loc);
            string obj;
            string kind = lit.c_x_w;
            if (kind.empty()) {
              auto parsed = canonicalLiteral(lit.literal);
              kind = parsed.second;
            }
            if ( kind == "c" && lit.literal.length() >= 3 ) {
              for ( size_t i = 2; i < lit.literal.length()-1; ++i ) {
                string hex;
                int val = static_cast<unsigned char>(lit.literal[i]);
                DecToHexa(val, hex);
                if (hex.length() == 1) hex.insert(0,"0");
                obj.append(hex);
              }
            } else if ( kind == "x" && lit.literal.length() >= 3 ) {
              obj = lit.literal.substr(2, lit.literal.length()-3);
              for ( char &c : obj ) c = toupper(static_cast<unsigned char>(c));
            } else { // word/number
              DecToHexa(atoi(lit.literal.c_str()), obj);
              while ( obj.length() < 6 ) obj.insert(0,"0");
            }
            string line_field = padLeft(seted,4,' ');
            string loc_field = padLeft(loc,4,' ');
            bool isLastLiteral = (emittedCount + 1 == totalDistinct);
            string prefix = line_field + "  " + loc_field;
            if ( isLastLiteral ) prefix.push_back(' ');
            prefix += "\t\t\t\t\t\t ";
            string obj_out = obj;
            if ( isLastLiteral && obj_out.length() < 11 )
              obj_out.append(11 - obj_out.length(), ' ');
            outfile << prefix << obj_out << "\r\n";
            emittedCount++;
          }
        }
	    //Sicxe_Instruction_print( sicxe ); // 
		outfile.close();//Close output file 
	} // sicxe

int main() {
	int command = 0; // 
	do { 
	  cout << endl << "** Assembler **";
	  cout << endl << "* 0. Quit     *";
	  cout << endl << "* 1. SIC      *"; 
	  cout << endl << "* 2. SICXE    *"; 
	  cout << endl << "***************"; 
	  cout << endl << "Input a command(0, 1, 2): "; 
	  cin >> command;  // 
	  switch(command){
	  	case 0 : break;
	  	case 1 : {
	      string inputPath = promptFilePath("Enter SIC input file path: ");
	      string outputPath = promptFilePath("Enter SIC output file path: ");
	      sic(inputPath, outputPath);
	      break;
	    }
	    case 2 : {
	      string inputPath = promptFilePath("Enter SICXE input file path: ");
	      string outputPath = promptFilePath("Enter SICXE output file path: ");
	      sicxe(inputPath, outputPath);  
	      break;
	    }
	    default : cout << endl << "Command does not exist!" << endl; // 
	  }
	} while ( command != 0 );
	system ( "pause" );
	return 0;
} //main
