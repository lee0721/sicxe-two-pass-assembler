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
    int temp = 0;
    int de = 0;
    string hex;
    string str;
    disp = - disp;
    DecToHexa(disp, str);
    for ( int i = 0; i < str.length() ; i++ ) {
    	de = 0;
    	string t;
    	t.append(1,str[i]);
    	if ( i == 0 ) {
    	  HexToDe( t, de );
    	  temp = 15 - de ;
		}
		else {
			HexToDe( t, de );
			temp = 15 - de + 1 ;
		}
		DecToHexa(temp, hex);
		hexdisp.insert(hexdisp.length(),hex);
		hex.clear();
	} // for
} // negativeDecToHexa

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
  int base_location = 0;
  HexToDe( hexdisp, base_location );
  if ( !toks.forwardreference && !token_packer.base_hexlocation.empty() && token_packer.end ) {
     // Base-relative addressing: subtract the BASE register location.
  	 for ( int i = 0; i < token_packer.amount ; i++ ) 
    	if ( toks.group1 == token_packer.token_groups[i].label )
    	  disp = token_packer.token_groups[i].location - base_location; 
     hexdisp.clear();
     if ( disp > 0 )
       DecToHexa(disp, hexdisp);
     else
       negativeDecToHexa( disp, hexdisp );
  } 
  else if ( !toks.forwardreference ) { 
     // PC-relative addressing with a known target: displacement = target - (PC + instr length).
     for ( int i = 0; i < token_packer.amount ; i++ ) 
    	if ( toks.group1 == token_packer.token_groups[i].label )
    	  disp = token_packer.token_groups[i].location - ( toks.location + toks.label_length ); 
     negativeDecToHexa( disp, hexdisp );
  } 
  else { 
    // Forward reference: still PC-relative but we leave the displacement positive to be patched.
    for ( int i = 0; i < token_packer.amount ; i++ ) 
    	if ( toks.group1 == token_packer.token_groups[i].label )
    	  disp = token_packer.token_groups[i].location - ( toks.location + toks.label_length ); 
	DecToHexa(disp, hexdisp);
  }
} //  sicxe_set_disp

void sicxe_set_address( Packed_Token token_packer, Tokens toks, string &address ){
    for ( int i = 0; i < token_packer.amount ; i++ ) 
      if ( toks.group1 == token_packer.token_groups[i].label )
    	address = token_packer.token_groups[i].hex_location; 
} //  sicxe_set_address

// Emit object code according to the decoded format/opcode/operands.
// Format 2 simply packs register numbers, while formats 3/4 must consider
// n/i/x/b/p/e bits plus PC/base relative displacements.

void sicxe_setcode( Packed_Token token_packer, Tokens &toks ) {
	int r1 = 0;
	int r2 = 0;
	string disp;
	string address;
	if ( toks.format == 2 ) {
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
		if ( toks.opformat == 2 ) {
		  if ( toks.flags.n && toks.flags.i ) { // literal/immediate operand
		    if ( isNumber( toks.group1 ) ) {
		      string temp;
		      temp.append(1,toks.objectcode[1]);
		      if ( temp == "0" ) { // 0000 0 -> 0011 3
		  	    toks.objectcode.erase(toks.objectcode.length()-1,1);
		  	    toks.objectcode.insert(toks.objectcode.length(),"3");
		  	    if ( toks.group1.length() == 1 )
		  	      toks.objectcode.insert(toks.objectcode.length(),"000");
		  	    else if ( toks.group1.length() == 2 )
		  	      toks.objectcode.insert(toks.objectcode.length(),"00");
		  	    else if ( toks.group1.length() == 3 )
		  	      toks.objectcode.insert(toks.objectcode.length(),"0");
		  	    toks.objectcode.insert(toks.objectcode.length(),toks.group1);
			  } // if
			  else if ( temp == "4" ) { // 0100 4 -> 0111 7
		  	    toks.objectcode.erase(toks.objectcode.length()-1,1);
		  	    toks.objectcode.insert(toks.objectcode.length(),"7");
		  	    if ( toks.group1.length() == 1 )
		  	      toks.objectcode.insert(toks.objectcode.length(),"000");
		  	    else if ( toks.group1.length() == 2 )
		  	      toks.objectcode.insert(toks.objectcode.length(),"00");
		  	    else if ( toks.group1.length() == 3 )
		  	      toks.objectcode.insert(toks.objectcode.length(),"0");
		  	    toks.objectcode.insert(toks.objectcode.length(),toks.group1);
			  } // else if
		  	  else if ( temp == "8" ) { // 1000 8 -> 1011 B
		  	    toks.objectcode.erase(toks.objectcode.length()-1,1);
		  	    toks.objectcode.insert(toks.objectcode.length(),"B");
		  	    if ( toks.group1.length() == 1 )
		  	      toks.objectcode.insert(toks.objectcode.length(),"000");
		  	    else if ( toks.group1.length() == 2 )
		  	      toks.objectcode.insert(toks.objectcode.length(),"00");
		  	    else if ( toks.group1.length() == 3 )
		  	      toks.objectcode.insert(toks.objectcode.length(),"0");
		  	    toks.objectcode.insert(toks.objectcode.length(),toks.group1);
			  } // else if
			  else if ( temp == "C" ) { // 1100 C -> 1111 F
		  	    toks.objectcode.erase(toks.objectcode.length()-1,1);
		  	    toks.objectcode.insert(toks.objectcode.length(),"F");
		  	    if ( toks.group1.length() == 1 )
		  	      toks.objectcode.insert(toks.objectcode.length(),"000");
		  	    else if ( toks.group1.length() == 2 )
		  	      toks.objectcode.insert(toks.objectcode.length(),"00");
		  	    else if ( toks.group1.length() == 3 )
		  	      toks.objectcode.insert(toks.objectcode.length(),"0");
		  	    toks.objectcode.insert(toks.objectcode.length(),toks.group1);
			  } // else if
			} // if	   
			else {  // symbolic target requires displacement calculation
		    string temp;
		    temp.append(1,toks.objectcode[1]);
		  	if ( temp == "0" ) { // convert to n=1,i=1
		  	  toks.objectcode.erase(toks.objectcode.length()-1,1);
		  	  toks.objectcode.insert(toks.objectcode.length(),"3");
		  	  sicxe_set_xbpe( toks );
		  	  if ( toks.flags.p && toks.forwardreference ) // unresolved PC-relative target
		  	    toks.objectcode.insert(toks.objectcode.length(),"000");  // placeholder disp
			  else if ( toks.flags.p && !toks.forwardreference ) { // resolved target
		  	    sicxe_set_disp(token_packer, toks, disp);   // compute displacement
				toks.objectcode.insert(toks.objectcode.length(),"F");
				toks.objectcode.insert(toks.objectcode.length(),disp);
		      } // else if
		  	  else if ( toks.flags.b ) // base-relative fallback
		  	    toks.objectcode.insert(toks.objectcode.length(),"000");  // placeholder
			} // if
			else if ( temp == "4" ) { // 0100 4 -> 0111 7
		  	  toks.objectcode.erase(toks.objectcode.length()-1,1);
		  	  toks.objectcode.insert(toks.objectcode.length(),"7");
		  	  sicxe_set_xbpe( toks );
		  	  if ( toks.flags.p && toks.forwardreference ) // unresolved PC-relative target
		  	    toks.objectcode.insert(toks.objectcode.length(),"000");  // placeholder
			  else if ( toks.flags.p && !toks.forwardreference ) { // resolved target
		  	    sicxe_set_disp(token_packer, toks, disp);   
		  	    toks.objectcode.insert(toks.objectcode.length(),"F");
				toks.objectcode.insert(toks.objectcode.length(),disp);
		      } // else if
		  	  else if ( toks.flags.b ) // base-relative fallback
		  	    toks.objectcode.insert(toks.objectcode.length(),"000");  // placeholder
			} // else if
			else if ( temp == "8" ) { // 1000 8 -> 1011 B
		  	  toks.objectcode.erase(toks.objectcode.length()-1,1);
		  	  toks.objectcode.insert(toks.objectcode.length(),"B");
		  	  sicxe_set_xbpe( toks );
		  	  if ( toks.flags.p && toks.forwardreference ) // unresolved PC-relative target
		  	    toks.objectcode.insert(toks.objectcode.length(),"000");  // placeholder
			  else if ( toks.flags.p && !toks.forwardreference ) { // resolved target
		  	    sicxe_set_disp(token_packer, toks, disp);   
                toks.objectcode.insert(toks.objectcode.length(),"F");
				toks.objectcode.insert(toks.objectcode.length(),disp);
		      } // else if
		  	  else if ( toks.flags.b ) // base-relative fallback
		  	    toks.objectcode.insert(toks.objectcode.length(),"000");  // placeholder
			} // else if
			else if ( temp == "C" ) { // 1100 C -> 1111 F
		  	  toks.objectcode.erase(toks.objectcode.length()-1,1);
		  	  toks.objectcode.insert(toks.objectcode.length(),"F");
		  	  sicxe_set_xbpe( toks );
		  	  if ( toks.flags.p && toks.forwardreference ) // unresolved PC-relative target
		  	    toks.objectcode.insert(toks.objectcode.length(),"000");  // placeholder
			  else if ( toks.flags.p && !toks.forwardreference ) { // resolved target
		  	    sicxe_set_disp(token_packer, toks, disp);   
                toks.objectcode.insert(toks.objectcode.length(),"F");
				toks.objectcode.insert(toks.objectcode.length(),disp); 
		      } // else if
		  	  else if ( toks.flags.b ) // base-relative fallback
		  	    toks.objectcode.insert(toks.objectcode.length(),"000");  // placeholder
			} // else if // 1100 C -> 1111 F
		} // else
		  } // else if
		} //  if
	} // else if
	else if ( toks.format == 4 ) {
		if ( toks.opformat == 2 ) {
		  if ( toks.flags.n && toks.flags.i ) { //  n=1i=1
		    string temp;
		    temp.append(1,toks.objectcode[1]);
		  	if ( temp == "0" ) { // 0000 0 -> 0011 3
		  	  toks.objectcode.erase(toks.objectcode.length()-1,1);
		  	  toks.objectcode.insert(toks.objectcode.length(),"3");
		  	  sicxe_set_xbpe( toks );
		  	  if ( toks.forwardreference ) // 
		  	    toks.objectcode.insert(toks.objectcode.length(),"00000");  // 
			  else if ( !toks.forwardreference ) { // 
		  	    sicxe_set_address(token_packer, toks, address); 
		  	    toks.objectcode.insert(toks.objectcode.length(),"0");
				toks.objectcode.insert(toks.objectcode.length(),address); 
		      } // else if
			} // if
			else if ( temp == "4" ) { // 0100 4 -> 0111 7
		  	  toks.objectcode.erase(toks.objectcode.length()-1,1);
		  	  toks.objectcode.insert(toks.objectcode.length(),"7");
		  	  sicxe_set_xbpe( toks );
		  	  if ( toks.forwardreference ) // 
		  	    toks.objectcode.insert(toks.objectcode.length(),"00000");  // 
			  else if ( !toks.forwardreference ) { // 
			    sicxe_set_address(token_packer, toks, address); 
		  	    toks.objectcode.insert(toks.objectcode.length(),"0");
				toks.objectcode.insert(toks.objectcode.length(),address); 
		      } // else if
		    } // else if
			else if ( temp == "8" ) { // 1000 8 -> 1011 B
		  	  toks.objectcode.erase(toks.objectcode.length()-1,1);
		  	  toks.objectcode.insert(toks.objectcode.length(),"B");
		  	  sicxe_set_xbpe( toks );
		  	  if ( toks.forwardreference ) // 
		  	    toks.objectcode.insert(toks.objectcode.length(),"00000");  // 
			  else if ( !toks.forwardreference ) { // 
                sicxe_set_address(token_packer, toks, address); 
		  	    toks.objectcode.insert(toks.objectcode.length(),"0");
				toks.objectcode.insert(toks.objectcode.length(),address); 
		      } // else if
		    } // else if
			else if ( temp == "C" ) { // 1100 C -> 1111 F
		  	  toks.objectcode.erase(toks.objectcode.length()-1,1);
		  	  toks.objectcode.insert(toks.objectcode.length(),"F");
		  	  sicxe_set_xbpe( toks );
		  	  if ( toks.forwardreference ) // 
		  	    toks.objectcode.insert(toks.objectcode.length(),"00000");  // 
			  else if ( !toks.forwardreference ) { // 
			    sicxe_set_address(token_packer, toks, address); 
		  	    toks.objectcode.insert(toks.objectcode.length(),"0");
				toks.objectcode.insert(toks.objectcode.length(),address); 
		      } // else if
		    } // else if
		  } // if
		} // if
	} // else if
} //sicxe_setcode

bool sicxe_Set_p_b ( Packed_Token token_packer, Tokens &toks, string token ){  // 
		// 
		// 
		// 
	if ( toks.format == 3 ) {
	  int num = 0;
	  for ( int i = 0 ; i < token_packer.amount ; i++ ){
		num = i;
		if ( token == token_packer.token_groups[i].label )
		  break;
	  } // for
	  if ( token == token_packer.token_groups[num].label ) {
		if ( token_packer.base )  // 
		  toks.flags.b = true;
		else // 
		  toks.flags.p = true;
		return true;
	  } // 
	  else { // 
		toks.flags.p = true;
		return false;
	  } // 
	    } // if
	    else if ( toks.format == 4 ){
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
                toks.group1 = toks.tokens[5].value;
			  } // 
			  if ( toks.tokens[5].type == TokenType::Number ) {  // 
  	            toks.literal.label = toks.tokens[5].value;
  	            toks.literal.WORDorBYTE = "BYTE";
                toks.literal.literal = toks.tokens[5].value;
                toks.literal.literal.insert( 0, toks.tokens[4].value );
                toks.literal.literal.insert( toks.literal.literal.length()-1, toks.tokens[4].value );
                toks.literal.literal.insert( 0, "X" );
                toks.group1 = toks.tokens[5].value;
			  } // 
			  else if ( toks.tokens[4].type == TokenType::Number ) {  // 
			    toks.literal.label = toks.tokens[4].value;
			    toks.literal.label.insert( 0, toks.tokens[3].value );
			    toks.literal.WORDorBYTE = "WORD";
			    toks.literal.literal = toks.tokens[4].value;
			    toks.group1 = toks.literal.label;
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
                toks.group1 = toks.tokens[4].value;
			  } // 
			  if ( toks.tokens[4].type == TokenType::Number ) {  // 
  	            toks.literal.label = toks.tokens[4].value;
  	            toks.literal.WORDorBYTE = "BYTE";
                toks.literal.literal = toks.tokens[4].value;
                toks.literal.literal.insert( 0, toks.tokens[3].value );
                toks.literal.literal.insert( toks.literal.literal.length()-1, toks.tokens[3].value );
                toks.literal.literal.insert( 0, "X" );
                toks.group1 = toks.tokens[4].value;
			  } // 
			  else if ( toks.tokens[3].type == TokenType::Number ) {  // 
			    toks.literal.label = toks.tokens[3].value;
			    toks.literal.label.insert( 0, toks.tokens[2].value );
			    toks.literal.WORDorBYTE = "WORD";
			    toks.literal.literal = toks.tokens[3].value;
			    toks.group1 = toks.literal.label;
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
	    toks.group1 = toks.tokens[2].value;
	    toks.group2 = toks.tokens[4].value;
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
	      else if ( toks.tokens[1].tokenvalue == 4 ) {// 
	        if ( toks.tokens[3].type == TokenType::Literal ) {  // 
          toks.group1 = toks.tokens[3].value;  // 
	    } // if
	    else if ( toks.tokens[3].type == TokenType::Number ) {  // 
	      toks.group1 = toks.tokens[3].value;  // 
		} // else if
		else if ( toks.tokens[2].type == TokenType::Number ) {  // dec_num
	      toks.group1 = toks.tokens[2].value; 
		} // else if
      }// else if BYTE
	      else if ( toks.tokens[1].tokenvalue == 4 ) {// 
		    if ( toks.tokens[3].type == TokenType::Literal ) {  // 
          toks.group1 = toks.tokens[3].value;  // 
	    } // if
	    else if ( toks.tokens[3].type == TokenType::Number ) {  // 
	      toks.group1 = toks.tokens[3].value;  // 
		} // else if
		else if ( toks.tokens[2].type == TokenType::Number ) {  // dec_num
	      toks.group1 = toks.tokens[2].value; 
		} // else if
      }// else if WORD
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
			  } // 
			  if ( toks.tokens[4].type == TokenType::Number ) {  // 
  	            toks.literal.label = toks.tokens[4].value;
  	            toks.literal.WORDorBYTE = "BYTE";
                toks.literal.literal = toks.tokens[4].value;
                toks.literal.literal.insert( 0, toks.tokens[3].value );
                toks.literal.literal.insert( toks.literal.literal.length()-1, toks.tokens[3].value );
                toks.literal.literal.insert( 0, "X" );
                toks.group1 = toks.tokens[4].value;
			  } // 
			  else if ( toks.tokens[3].type == TokenType::Number ) {  // 
			    toks.literal.label = toks.tokens[3].value;
			    toks.literal.label.insert( 0, toks.tokens[2].value );
			    toks.literal.WORDorBYTE = "WORD";
			    toks.literal.literal = toks.tokens[3].value;
			    toks.group1 = toks.literal.label;
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
			  } // 
			  if ( toks.tokens[3].type == TokenType::Number ) {  // 
  	            toks.literal.label = toks.tokens[3].value;
  	            toks.literal.WORDorBYTE = "BYTE";
                toks.literal.literal = toks.tokens[3].value;
                toks.literal.literal.insert( 0, toks.tokens[2].value );
                toks.literal.literal.insert( toks.literal.literal.length()-1, toks.tokens[2].value );
                toks.literal.literal.insert( 0, "X" );
                toks.group1 = toks.tokens[3].value;
			  } // 
			  else if ( toks.tokens[2].type == TokenType::Number ) {  // 
			    toks.literal.label = toks.tokens[2].value;
			    toks.literal.label.insert( 0, toks.tokens[1].value );
			    toks.literal.WORDorBYTE = "WORD";
			    toks.literal.literal = toks.tokens[2].value;
			    toks.group1 = toks.literal.label;
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
			token_packer.token_groups[i].objectcode.erase(token_packer.token_groups[i].objectcode.length()-1,1);
			token_packer.token_groups[i].objectcode.erase(token_packer.token_groups[i].objectcode.length()-1,1);
			token_packer.token_groups[i].objectcode.erase(token_packer.token_groups[i].objectcode.length()-1,1);
			token_packer.token_groups[i].objectcode.insert(token_packer.token_groups[i].objectcode.length(),"F");
			token_packer.token_groups[i].objectcode.insert(token_packer.token_groups[i].objectcode.length(),disp);
		} // 
	} // for
} //sicxe_resetcodedisp

void sicxe_pass2( Packed_Token &token_packer ){
	for ( int i = 0 ; i < token_packer.amount ; i++ ) {
		string address;
		//cout << token_packer.token_groups[i].ins << token_packer.token_groups[i].forwardreference <<endl ; // 
		if ( token_packer.token_groups[i].forwardreference ) {
			if ( token_packer.token_groups[i].format == 4 ) {
			  token_packer.token_groups[i].objectcode.erase(token_packer.token_groups[i].objectcode.length()-1,1);
			  token_packer.token_groups[i].objectcode.erase(token_packer.token_groups[i].objectcode.length()-1,1);
			  token_packer.token_groups[i].objectcode.erase(token_packer.token_groups[i].objectcode.length()-1,1);
			  token_packer.token_groups[i].objectcode.erase(token_packer.token_groups[i].objectcode.length()-1,1);
				sicxe_set_address( token_packer, token_packer.token_groups[i], address );
			    token_packer.token_groups[i].objectcode.insert(token_packer.token_groups[i].objectcode.length(),address);
			}
			if ( token_packer.token_groups[i].format == 3 ) {
			  string disp ;
			  sicxe_set_disp( token_packer, token_packer.token_groups[i], disp );
			  token_packer.token_groups[i].objectcode.erase(token_packer.token_groups[i].objectcode.length()-1,1);
			  token_packer.token_groups[i].objectcode.erase(token_packer.token_groups[i].objectcode.length()-1,1);
			  token_packer.token_groups[i].objectcode.erase(token_packer.token_groups[i].objectcode.length()-1,1);
			  if ( disp.length() == 1 )
		  	    token_packer.token_groups[i].objectcode.insert(token_packer.token_groups[i].objectcode.length(),"00");
		  	  else if ( disp.length() == 2 )
		  	    token_packer.token_groups[i].objectcode.insert(token_packer.token_groups[i].objectcode.length(),"0");
			  token_packer.token_groups[i].objectcode.insert(token_packer.token_groups[i].objectcode.length(),disp);
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
                toks.group1 = toks.tokens[3].value;
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
                toks.group1 = toks.tokens[3].value;
			} // 
			else if ( toks.tokens[2].type == TokenType::Number ) {  // 
			    toks.label_length = 3;
			    toks.literal.c_x_w = "w";
			    toks.literal.label = toks.tokens[2].value;
			    toks.literal.label.insert( 0, toks.tokens[1].value );
			    toks.literal.WORDorBYTE = "WORD";
			    toks.literal.literal = toks.tokens[2].value;
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
	      outfile << "\t" << tg.sourcestatement << "\r\n";
	      continue;
	    }
	    if ( tg.end ) {
	      outfile << "\t\t" << tg.sourcestatement << "\r\n";
	      continue;
	    }
	    outfile << tg.hex_location << "\t";
	    if ( tg.label.empty() )
	      outfile << "\t"; // align when no label
	    outfile << tg.sourcestatement;
	    if ( !tg.pseudo && !tg.objectcode.empty() ) {
	      string spacer;
	      if ( tg.group1.empty() && tg.group2.empty() )
	        spacer = "\t\t\t";
	      else if ( tg.sourcestatement.find(',') != string::npos )
	        spacer = "\t";
	      else
	        spacer = "\t\t";
	      outfile << spacer << tg.objectcode;
	      if ( tg.ins == "J" )
	        outfile << " ";
	    }
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
	  sicxe_setcode( token_packer, token_packer.token_groups[token_packer.amount] );
	  token_packer.amount++;
	} // while
	sicxe_setbase(token_packer);
	sicxe_resetcodedisp(token_packer); //format3 && !forwardreference
	sicxe_pass2( token_packer );
	newfile.close();//Close file after reading
	outfile << "Line  Location  Source code                              Object code\r\n";
	outfile << "----  -------- -------------------------                 -----------\r\n";
	for ( int b = 0 ; b < token_packer.amount; b++) { //
	    const auto &tg = token_packer.token_groups[b];
	    if ( !tg.error.empty() ) {
	      outfile << tg.error << "\r\n";
	      continue;
	    }
	    if( tg.sourcestatement.empty() ) {
	      outfile << "\r\n" ;
	      continue;
	    }

	    auto sanitize = [](string s) {
	      for ( char &c : s )
	        if ( c == '\t' ) c = ' ';
	      return s;
	    };

	    string label = sanitize(tg.label);
	    string mnemonic = sanitize(tg.ins);
	    if ( tg.format == 4 && !mnemonic.empty() && mnemonic[0] != '+' )
	      mnemonic.insert(0, "+");
	    string operand;
	    if ( !tg.group1.empty() )
	      operand = sanitize(tg.group1);
	    if ( !tg.group2.empty() ) {
	      if ( !operand.empty() )
	        operand.append(",");
	      operand.append(sanitize(tg.group2));
	    }

	    auto pad = [&](const string &s, int width) {
	      string out = s;
	      if ( out.length() < static_cast<size_t>(width) )
	        out.append(width - out.length(), ' ');
	      return out;
	    };

	    string line_field = pad(tg.setedline,4);
	    string loc_field = (tg.comment || tg.end || tg.base) ? "" : tg.hex_location;
	    loc_field = pad(loc_field,4);
	    string label_field = pad(label,15);
	    string mnemonic_field = pad(mnemonic,10);
	    string operand_field = operand;
	    string source_field = label_field + mnemonic_field + operand_field;

	    if ( tg.comment ) {
	      string comment_text = sanitize(tg.sourcestatement);
	      source_field = pad("",15) + comment_text; // keep comments aligned to the mnemonic column
	    }

	    string prefix = line_field + "  " + loc_field + "  " + source_field;
	    string obj;
	    if ( !tg.pseudo && !tg.objectcode.empty() )
	      obj = tg.objectcode;
	    int needed = 0;
	    if ( obj.empty() ) {
	      int targetLen = 57; // default padding when no object code
	      if ( tg.comment ) targetLen = 43;
	      else if ( tg.end ) targetLen = 52;
	      needed = targetLen - static_cast<int>(prefix.length());
	    } else {
	      int targetLen = 57 + static_cast<int>(obj.length()); // object field always starts at column 58
	      needed = targetLen - static_cast<int>(prefix.length()) - static_cast<int>(obj.length());
	    }
	    if ( needed < 0 ) needed = 0;
	    prefix.append(needed, ' ');
	    if ( !obj.empty() )
	      outfile << prefix << obj << "\r\n";
	    else
	      outfile << prefix << "\r\n";
	} // for
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
