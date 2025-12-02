//系統程式作業二_10833230李郁含 
#include<iostream>    // cin,cout,endl
#include<string>      // c_str,length
#include<string.h>    //strcpy
#include<stdio.h>    
#include<stdlib.h>    //atoi()
#include<cstdlib>     //strtoul,system
#include <vector>
#include<fstream>
#include<typeinfo>

#define MAX_TOKENS 30 //估計一行程式最多有幾個 tokens
#define MAX_TOKEN_GROUP 100 //估計最多幾行 
 
using namespace std;
using std::string;

typedef char STRING20 [20]; //短字串，最多19個字元
typedef char STRING200 [200]; //長字串，最多199個字元

struct Sicxe_Instruction_Set {
	string instruction;
	int format = 0; //format1或 format2或format3/4(先設定為3 後面判斷有+就改4
	int opformat = 0; //後面op的形式  (1) x (沒有的) [1/3/4]
	                  //              (2) m          [3/4]
					  //              (3) r1         [2]
					  //              (4) r1,r2      [2]
					  //              (5) r1,n       [2]
					  //              (6) n          [2]
	string objectcode;
}; // Sicxe_Instruction_Set

void Sicxe_Instruction_input( Sicxe_Instruction_Set sicxe[59] ) {
	ifstream newfile; 
	newfile.open("Sicxe_Instruction_Set.table"); //開啟檔案
	int num = 0; // 多少個指令 
	for ( int i = 0 ; i < 236 ; i++) {
	  newfile >> sicxe[num].instruction; //將一行一行的字串讀檔進struct裡面  指令 
	  newfile >> sicxe[num].format; //將一行一行的字串讀檔進struct裡面  形式 
	  newfile >> sicxe[num].opformat; //將一行一行的字串讀檔進struct裡面  op形式 
	  newfile >> sicxe[num].objectcode; //將一行一行的字串讀檔進struct裡面  code
	  num++;
	} //for 
	newfile.close();//讀檔完後關閉檔案
}  // Sicxe_Instruction_input

struct Sic_Instruction_Set {
	string instruction;
	string objectcode;
}; // Sic_Instruction_Set

void Sic_Instruction_input( Sic_Instruction_Set sic[26] ) {
	ifstream newfile; 
	newfile.open("Sic_Instruction_Set.table"); //開啟檔案
	int num = 0; // 多少個指令 
	for ( int i = 0 ; i < 52 ; i++) {
	  newfile >> sic[num].instruction; //將一行一行的字串讀檔進struct裡面  指令 
	  newfile >> sic[num].objectcode; //將一行一行的字串讀檔進struct裡面  code
	  num++;
	} //for 
	newfile.close();//讀檔完後關閉檔案
}  // Sic_Instruction_input

void Sicxe_Instruction_print( Sicxe_Instruction_Set sicxe[59] ) {  //test
	for ( int i = 0 ; i < 59 ; i++) {
	  cout << sicxe[i].instruction << " "; //將一行一行的字串讀檔進struct裡面  指令 
	  cout << sicxe[i].format << " "; //將一行一行的字串讀檔進struct裡面  op形式 
	  cout << sicxe[i].opformat << " "; //將一行一行的字串讀檔進struct裡面  形式 
	  cout << sicxe[i].objectcode; //將一行一行的字串讀檔進struct裡面  code
	  cout << endl ;
	} //for 
}  // Sicxe_Instruction_print

void Sic_Instruction_print( Sic_Instruction_Set sic[26] ) {  //test
	for ( int i = 0 ; i < 26 ; i++) {
	  cout << sic[i].instruction << " "; //將一行一行的字串讀檔進struct裡面  指令 
	  cout << sic[i].objectcode; //將一行一行的字串讀檔進struct裡面  code
	  cout << endl ;
	} //for 
}  // Sic_Instruction_print

struct Token{
    string value;    
    int tokentype = 0;  //放在哪個table 
    int tokenvalue = 0; //放在table裡第幾個index 
}; // Token

struct Literal{
	int location = 0; //紀錄位址(十進位的所以後面要改成十六進位) 
	string c_x_w ;
    string label;    
    string WORDorBYTE;
    string literal;
}; // Literal

struct Tokens{
    unsigned amount; //紀錄這行 source 中有實際有幾個 token
    int label_length = 0; //紀錄這行指令長度 
    string error;    //紀錄這行 source 中遇到的error 
    bool forwardreference = false; //有沒有需要 forward reference
    bool end = false;  //這行有end 
    bool base = false;  //這行有base
    bool comment = false; //這行是註解 
    bool EQU = false; 
    bool START = false ;
    bool pseudo = false;
    //--------------------------------------------------
    string c_x_w ;
    string sourcestatement; //紀錄這行程式的原始程式 
    int b = 0;   //是否有使用 base register 
    int p = 0;   //是否有使用 program counter
    int x = 0;   //是否有使用 index register 
    int i = 0;   //是否有使用 immediate mode
	int n = 0;   //是否有使用 indirect mode
	int line = 0;
	string setedline;  //補完空白的line 
    int location = 0; //紀錄位址(十進位的所以後面要改成十六進位) 
    string hex_location; //十六進位位址
    string objectcode;  //紀錄objectcode(十六進位) 
    //--------------------------------------------------
    int format = 0 ; //format1234
    int opformat = 0; //後面op的形式
    string label; //這行程式碼的 Label，如果沒有的話就是空字串
    string ins; //這行程式碼的 "指令" 或 "虛擬指令"，如果沒有的話就是空字串
    string group1;
    string group2;
    Literal literal;
    //--------------------------------------------------
    Token tokens[MAX_TOKENS]; //實際紀錄 token 的地方
};  // Tokens

struct Packed_Token{ //注意這個名稱 -> "包裝過的 tokens"
    unsigned amount; //實際上 Tokens Group 的數量，沒有的話就是 0
    int longestnum = 0; //紀錄最長的source 
    bool end = false;
    bool base = false;
    string base_label;
    string base_hexlocation;
    int basenum = 0;
    Tokens token_groups[MAX_TOKEN_GROUP]; //真正用來用來記錄 tokens groups 的地方
}; // Packed_Token

struct Table{
    int index;
    string value;
}; //Table

void removespace( string &str ){ //移除空白 //test
  int index = 0;
    if( !str.empty()) {
      while( (index = str.find(' ',index)) != string::npos)
        str.erase(index,1);
    } // if
} // removespace

void tableinput( Table table1[59], Table table2[9], Table table3[9], Table table4[13] ) {  //get the table1.2.3.4 
	ifstream newfile; 
	newfile.open("Table1.table"); //開啟檔案
	for ( int i = 1 ; i < 60 ; i++) {
	  newfile >> table1[i-1].value; //將一行一行的字串讀檔進struct裡面 
	  table1[i-1].index = i;
	} //for 
	newfile.close();//讀檔完後關閉檔案
	
	newfile.open("Table2.table"); //開啟檔案
	for ( int i = 1 ; i < 10; i++) {
	  newfile >> table2[i-1].value; //將一行一行的字串讀檔進struct裡面 
	  table2[i-1].index = i;
	} //for 
	newfile.close();//讀檔完後關閉檔案
	
	newfile.open("Table3.table"); //開啟檔案
	for ( int i = 1 ; i < 10; i++) {
	  newfile >> table3[i-1].value; //將一行一行的字串讀檔進struct裡面 
	  table3[i-1].index = i;
	} //for 
	newfile.close();//讀檔完後關閉檔案
	
	newfile.open("Table4.table"); //開啟檔案
	for ( int i = 1 ; i < 14; i++) {
	  newfile >> table4[i-1].value; //將一行一行的字串讀檔進struct裡面 
	  table4[i-1].index = i;
	} //for 
	newfile.close();//讀檔完後關閉檔案
} // tableinput

string to_upper( string token ) { //全大寫 
	for ( int a = 0; a < token.length() ; a++ ) {
        if (islower(token[a]))                 // 若為小寫字元
            token[a] = toupper(token[a]); 
    } // for 
    return token;
} // to_upper

string to_lower( string token ) { //全小寫 
	for ( int a = 0; a < token.length() ; a++ ) {
        if (isupper(token[a]))                 // 若為大寫字元
            token[a] = tolower(token[a]); 
    } // for 
    return token;
} // to_lower

bool find( string token, Table table1[59], Table table2[9], Table table3[9], Table table4[13], 
          Table table5[100], Table table6[100], Table table7[100], int &tokentype, int &tokenvalue ){
    string upper = to_upper(token); 
	string lower = to_lower(token);     	
    for ( int i = 0 ; i < 59; i++) { // 去table1找此token
	  if ( lower == table1[i].value ) {
	  	tokentype = 1;
	  	tokenvalue = table1[i].index;
	  	return true;
	  } // if
	} //for 
	
	for ( int i = 0 ; i < 9; i++) { // 去table2找此token
	  if ( upper == table2[i].value ) {
	  	tokentype = 2;
	  	tokenvalue = table2[i].index;
	  	return true;
	  } // if
	} //for 

    for ( int i = 0 ; i < 9; i++) { // 去table3找此token
	  if ( upper == table3[i].value ) {
	  	tokentype = 3;
	  	tokenvalue = table3[i].index;
	  	return true;
	  } // if
	} //for 

	for ( int i = 0 ; i < 13; i++) { // 去table4找此token
	  if ( token == table4[i].value ) {
	  	tokentype = 4;
	  	tokenvalue = table4[i].index;
	  	return true;
	  } // if
	} //for 
	for ( int i = 0 ; i < 100; i++) { // 去table5找此token
	  if ( upper == table5[i].value ) {
	  	tokentype = 5;
	  	tokenvalue = table5[i].index;
	  	return true;
	  } // if
	} //for 
	for ( int i = 0 ; i < 100; i++) { // 去table6找此token
	  if ( upper == table6[i].value ) {
	  	tokentype = 6;
	  	tokenvalue = table6[i].index;
	  	return true;
	  } // if
	} //for 
	for ( int i = 0 ; i < 100; i++) { // 去table7找此token
	  if ( token == table7[i].value ) {
	  	tokentype = 7;
	  	tokenvalue = table7[i].index;
	  	return true;
	  } // if
	} //for 
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

bool isOp( char isop ){ // check whether the character is table4
     if ( ( isop == ',' ) || ( isop == '+' ) || ( isop == '-' ) || ( isop == '*' ) 
	           || ( isop == '/' ) || ( isop == ':' ) || ( isop == ';' ) || ( isop == '?' ) 
			      || ( isop == '@' ) || ( isop == '.' ) || ( isop == '=' ) || ( isop == '#' ) || ( int(isop) == 39 ) )
       return true;
     else
       return false;
} //isOp


void instable( string token, Table table[100], int &tokentype, int &tokenvalue){ //
  bool alreadyhave = false;
  int a = 0;
  for ( a = 0 ; a < 100 ; a++)  //先找找看自己的table也許有 
    if ( token == table[a].value )
      alreadyhave = true;
  if ( alreadyhave )
  	tokenvalue = a;
  else {
  	int num = 0; //裝此token的ascii碼
    for ( int i = 0 ; i < token.length(); i++) 
   	  num = num + int(token[i]);
    num = num % 100;
    if ( !table[num].value.empty() )
      num++;
    table[num].value = token;
    table[num].index = num;
    tokenvalue = num;
  } // else
} // instable

void removewhitespace( string &str ){ //移除空白/tab/換行 
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

bool onespace ( string token ) { //是否只是一個空白
  bool check = false;
  if ( token.length() == 1 ) 
    if ( token[0] == ' ' )
	   check = true;
  if ( check == true )
    return true;
  return false;
} // onespace

void ins_Tokens( Token &tok, string token, int type, int value ){
  tok.value = token;
  tok.tokentype = type;
  tok.tokenvalue = value;
} // ins_Tokens

void ins_packer( Tokens &toks, string token, int tokentype, int tokenvalue ){
  ins_Tokens(toks.tokens[toks.amount], token, tokentype, tokenvalue);
  //cout << toks.tokens[toks.amount].value << endl ; // test
  //cout << toks.tokens[toks.amount].tokentype << endl ; // test
  //cout << toks.tokens[toks.amount].tokenvalue << endl ; // test
  toks.amount++;
} // ins_packer

string getTokens( Token tok ) { //TEST 
	string outputdemo("(,)");  //先寫一個字串demo
	outputdemo.insert(1, std::to_string(tok.tokentype)); //將tokentype插入outputdemo 
	outputdemo.insert(outputdemo.length()-1, std::to_string(tok.tokenvalue)); //將tokenvalue插入outputdemo
	outputdemo.insert(0, tok.value); //將tokenvalue插入outputdemo
	return outputdemo;
} // getTokens

string getpacker( Tokens toks ) { //TEST
	string outputdemo;
	for ( int b = 0 ; b < toks.amount; b++) {
		//cout << getTokens( toks.tokens[b]) << endl ; // test
		string token = getTokens( toks.tokens[b]) ; 
		outputdemo.insert(outputdemo.length(), token); //將tokenvalue插入outputdemo
		outputdemo.insert(outputdemo.length(), " "); //將tokenvalue插入outputdemo
	} // for
	return outputdemo;
} // getpacker

void DecToHexa(int n, string &hex){ // 將十進位轉十六進位 de_to_hex 
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

void BinToHexa( string &xbpe ){  //將二進位轉十六進位bin_to_hex
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

void HexaToBin( string &t ){  //將二進位轉十六進位bin_to_hex
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
	string x = std::to_string(toks.x);
	string b = std::to_string(toks.b);
	string p = std::to_string(toks.p);
	string e;
	if ( toks.format == 3 )
	  e = "0";
	else if ( toks.format == 4 )
	  e = "1";
	x.insert(x.length(),b);
	x.insert(x.length(),p);
	x.insert(x.length(),e);
	BinToHexa( x );  //二進位轉十六進位 
	toks.objectcode.insert(toks.objectcode.length(),x);
} //sicxe_set_xbpe

void sicxe_set_disp( Packed_Token token_packer, Tokens toks, string &hexdisp ){
  int disp = 0;
  int base_location = 0;
  HexToDe( hexdisp, base_location );
  if ( !toks.forwardreference && !token_packer.base_hexlocation.empty() && token_packer.end ) { //base
  	 for ( int i = 0; i < token_packer.amount ; i++ ) 
    	if ( toks.group1 == token_packer.token_groups[i].label )
    	  disp = token_packer.token_groups[i].location - base_location; 
     hexdisp.clear();
     if ( disp > 0 )
       DecToHexa(disp, hexdisp);
     else
       negativeDecToHexa( disp, hexdisp );
  } // 
  else if ( !toks.forwardreference ) { //會是負的 
     for ( int i = 0; i < token_packer.amount ; i++ ) 
    	if ( toks.group1 == token_packer.token_groups[i].label )
    	  disp = token_packer.token_groups[i].location - ( toks.location + toks.label_length ); 
     negativeDecToHexa( disp, hexdisp );
  } // else if
  else { //會是正的 
    for ( int i = 0; i < token_packer.amount ; i++ ) 
    	if ( toks.group1 == token_packer.token_groups[i].label )
    	  disp = token_packer.token_groups[i].location - ( toks.location + toks.label_length ); 
	DecToHexa(disp, hexdisp);
  } // else
} //  sicxe_set_disp

void sicxe_set_address( Packed_Token token_packer, Tokens toks, string &address ){
    for ( int i = 0; i < token_packer.amount ; i++ ) 
      if ( toks.group1 == token_packer.token_groups[i].label )
    	address = token_packer.token_groups[i].hex_location; 
} //  sicxe_set_address

        //(1)	前面如果找得到(代表目的-來源(下一個)會是負的)且沒有base       ->設定p=1
		//(2)	前面如果找得到(代表目的-來源(下一個)會是負的)但有base         ->設定b=1
		//(3)	前面如果找不到(代表目的-來源(下一個)會是正的)不管有沒有base   ->設定p=1

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
		  if ( toks.n == 1 && toks.i == 1 ) { //  n=1i=1
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
			else {  
		    string temp;
		    temp.append(1,toks.objectcode[1]);
		  	if ( temp == "0" ) { // 0000 0 -> 0011 3
		  	  toks.objectcode.erase(toks.objectcode.length()-1,1);
		  	  toks.objectcode.insert(toks.objectcode.length(),"3");
		  	  sicxe_set_xbpe( toks );
		  	  if ( toks.p == 1 && toks.forwardreference ) //(3)	前面如果找不到(代表目的-來源(下一個)會是正的)不管有沒有base  ->p=1 
		  	    toks.objectcode.insert(toks.objectcode.length(),"000");  //如果需要 forwardreference後面disp就先設定000
			  else if ( toks.p == 1 && !toks.forwardreference ) { //(1)	前面如果找得到(代表目的-來源(下一個)會是負的)且沒有base ->p=1 
		  	    sicxe_set_disp(token_packer, toks, disp);   //如果不需要 forwardreference後面disp就直接設定 #Immediate Addressing 
				toks.objectcode.insert(toks.objectcode.length(),"F");
				toks.objectcode.insert(toks.objectcode.length(),disp);
		      } // else if
		  	  else if ( toks.b == 1 ) //(2)	前面如果找得到(代表目的-來源(下一個)會是負的)但有base    ->b=1 
		  	    toks.objectcode.insert(toks.objectcode.length(),"000");  //base可能需要 forwardreference後面disp就先設定000
			} // if
			else if ( temp == "4" ) { // 0100 4 -> 0111 7
		  	  toks.objectcode.erase(toks.objectcode.length()-1,1);
		  	  toks.objectcode.insert(toks.objectcode.length(),"7");
		  	  sicxe_set_xbpe( toks );
		  	  if ( toks.p == 1 && toks.forwardreference ) //(3)	前面如果找不到(代表目的-來源(下一個)會是正的)不管有沒有base  ->p=1 
		  	    toks.objectcode.insert(toks.objectcode.length(),"000");  //如果需要 forwardreference後面disp就先設定000
			  else if ( toks.p == 1 && !toks.forwardreference ) { //(1)	前面如果找得到(代表目的-來源(下一個)會是負的)且沒有base ->p=1 
		  	    sicxe_set_disp(token_packer, toks, disp);   //如果不需要 forwardreference後面disp就直接設定 #Immediate Addressing  
		  	    toks.objectcode.insert(toks.objectcode.length(),"F");
				toks.objectcode.insert(toks.objectcode.length(),disp);
		      } // else if
		  	  else if ( toks.b == 1 ) //(2)	前面如果找得到(代表目的-來源(下一個)會是負的)但有base    ->b=1 
		  	    toks.objectcode.insert(toks.objectcode.length(),"000");  //base可能需要 forwardreference後面disp就先設定000
			} // else if
			else if ( temp == "8" ) { // 1000 8 -> 1011 B
		  	  toks.objectcode.erase(toks.objectcode.length()-1,1);
		  	  toks.objectcode.insert(toks.objectcode.length(),"B");
		  	  sicxe_set_xbpe( toks );
		  	  if ( toks.p == 1 && toks.forwardreference ) //(3)	前面如果找不到(代表目的-來源(下一個)會是正的)不管有沒有base  ->p=1 
		  	    toks.objectcode.insert(toks.objectcode.length(),"000");  //如果需要 forwardreference後面disp就先設定000
			  else if ( toks.p == 1 && !toks.forwardreference ) { //(1)	前面如果找得到(代表目的-來源(下一個)會是負的)且沒有base ->p=1 
		  	    sicxe_set_disp(token_packer, toks, disp);   //如果不需要 forwardreference後面disp就直接設定 #Immediate Addressing  
                toks.objectcode.insert(toks.objectcode.length(),"F");
				toks.objectcode.insert(toks.objectcode.length(),disp);
		      } // else if
		  	  else if ( toks.b == 1 ) //(2)	前面如果找得到(代表目的-來源(下一個)會是負的)但有base    ->b=1 
		  	    toks.objectcode.insert(toks.objectcode.length(),"000");  //base可能需要 forwardreference後面disp就先設定000
			} // else if
			else if ( temp == "C" ) { // 1100 C -> 1111 F
		  	  toks.objectcode.erase(toks.objectcode.length()-1,1);
		  	  toks.objectcode.insert(toks.objectcode.length(),"F");
		  	  sicxe_set_xbpe( toks );
		  	  if ( toks.p == 1 && toks.forwardreference ) //(3)	前面如果找不到(代表目的-來源(下一個)會是正的)不管有沒有base  ->p=1 
		  	    toks.objectcode.insert(toks.objectcode.length(),"000");  //如果需要 forwardreference後面disp就先設定000
			  else if ( toks.p == 1 && !toks.forwardreference ) { //(1)	前面如果找得到(代表目的-來源(下一個)會是負的)且沒有base ->p=1 
		  	    sicxe_set_disp(token_packer, toks, disp);   //如果不需要 forwardreference後面disp就直接設定 #Immediate Addressing  
                toks.objectcode.insert(toks.objectcode.length(),"F");
				toks.objectcode.insert(toks.objectcode.length(),disp); 
		      } // else if
		  	  else if ( toks.b == 1 ) //(2)	前面如果找得到(代表目的-來源(下一個)會是負的)但有base    ->b=1 
		  	    toks.objectcode.insert(toks.objectcode.length(),"000");  //base可能需要 forwardreference後面disp就先設定000
			} // else if // 1100 C -> 1111 F
		} // else
		  } // else if
		} //  if
	} // else if
	else if ( toks.format == 4 ) {
		if ( toks.opformat == 2 ) {
		  if ( toks.n == 1 && toks.i == 1 ) { //  n=1i=1
		    string temp;
		    temp.append(1,toks.objectcode[1]);
		  	if ( temp == "0" ) { // 0000 0 -> 0011 3
		  	  toks.objectcode.erase(toks.objectcode.length()-1,1);
		  	  toks.objectcode.insert(toks.objectcode.length(),"3");
		  	  sicxe_set_xbpe( toks );
		  	  if ( toks.forwardreference ) //(3)	前面如果找不到(代表目的-來源(下一個)會是正的)不管有沒有base  ->p=1 
		  	    toks.objectcode.insert(toks.objectcode.length(),"00000");  //如果需要 forwardreference後面disp就先設定00000
			  else if ( !toks.forwardreference ) { //(1)	前面如果找得到
		  	    sicxe_set_address(token_packer, toks, address); 
		  	    toks.objectcode.insert(toks.objectcode.length(),"0");
				toks.objectcode.insert(toks.objectcode.length(),address); 
		      } // else if
			} // if
			else if ( temp == "4" ) { // 0100 4 -> 0111 7
		  	  toks.objectcode.erase(toks.objectcode.length()-1,1);
		  	  toks.objectcode.insert(toks.objectcode.length(),"7");
		  	  sicxe_set_xbpe( toks );
		  	  if ( toks.forwardreference ) //(3)	前面如果找不到(代表目的-來源(下一個)會是正的)不管有沒有base  ->p=1 
		  	    toks.objectcode.insert(toks.objectcode.length(),"00000");  //如果需要 forwardreference後面disp就先設定000
			  else if ( !toks.forwardreference ) { //(1)	前面如果找得到
			    sicxe_set_address(token_packer, toks, address); 
		  	    toks.objectcode.insert(toks.objectcode.length(),"0");
				toks.objectcode.insert(toks.objectcode.length(),address); 
		      } // else if
		    } // else if
			else if ( temp == "8" ) { // 1000 8 -> 1011 B
		  	  toks.objectcode.erase(toks.objectcode.length()-1,1);
		  	  toks.objectcode.insert(toks.objectcode.length(),"B");
		  	  sicxe_set_xbpe( toks );
		  	  if ( toks.forwardreference ) //(3)	前面如果找不到(代表目的-來源(下一個)會是正的)不管有沒有base  ->p=1 
		  	    toks.objectcode.insert(toks.objectcode.length(),"00000");  //如果需要 forwardreference後面disp就先設定000
			  else if ( !toks.forwardreference ) { //(1)	前面如果找得到
                sicxe_set_address(token_packer, toks, address); 
		  	    toks.objectcode.insert(toks.objectcode.length(),"0");
				toks.objectcode.insert(toks.objectcode.length(),address); 
		      } // else if
		    } // else if
			else if ( temp == "C" ) { // 1100 C -> 1111 F
		  	  toks.objectcode.erase(toks.objectcode.length()-1,1);
		  	  toks.objectcode.insert(toks.objectcode.length(),"F");
		  	  sicxe_set_xbpe( toks );
		  	  if ( toks.forwardreference ) //(3)	前面如果找不到(代表目的-來源(下一個)會是正的)不管有沒有base  ->p=1 
		  	    toks.objectcode.insert(toks.objectcode.length(),"00000");  //如果需要 forwardreference後面disp就先設定000
			  else if ( !toks.forwardreference ) { //(1)	前面如果找得到
			    sicxe_set_address(token_packer, toks, address); 
		  	    toks.objectcode.insert(toks.objectcode.length(),"0");
				toks.objectcode.insert(toks.objectcode.length(),address); 
		      } // else if
		    } // else if
		  } // if
		} // if
	} // else if
} //sicxe_setcode

bool sicxe_Set_p_b ( Packed_Token token_packer, Tokens &toks, string token ){  // format3 需要設定bp 將token丟到前面去找有沒有一樣label 
		//(1)	前面如果找得到(代表目的-來源(下一個)會是負的)且沒有base       ->設定p=1
		//(2)	前面如果找得到(代表目的-來源(下一個)會是負的)但有base         ->設定b=1
		//(3)	前面如果找不到(代表目的-來源(下一個)會是正的)不管有沒有base   ->設定p=1
	if ( toks.format == 3 ) {
	  int num = 0;
	  for ( int i = 0 ; i < token_packer.amount ; i++ ){
		num = i;
		if ( token == token_packer.token_groups[i].label )
		  break;
	  } // for
	  if ( token == token_packer.token_groups[num].label ) {
		if ( token_packer.base )  //(2)	前面如果找得到(代表目的-來源(下一個)會是負的)但有base         ->設定b=1
		  toks.b = 1;
		else //(1)	前面如果找得到(代表目的-來源(下一個)會是負的)且沒有base       ->設定p=1
		  toks.p = 1;
		return true;
	  } // 找得到
	  else { //(3)	前面如果找不到(代表目的-來源(下一個)會是正的)不管有沒有base   ->設定p=1
		toks.p = 1;
		return false;
	  } // else 找不到	
    } // if
    else if ( toks.format == 4 ){
      for ( int i = 0 ; i < token_packer.amount ; i++ ){
		if ( token == token_packer.token_groups[i].label )
		  return true;
	  } // for
	  return false;
	} // else if
	
} // sicxe_Set_p_b

void sicxe_RecordAndSyntax( Packed_Token &token_packer, Tokens &toks, Sicxe_Instruction_Set sicxe[59] ) { 
  if ( toks.tokens[0].tokentype == 5 ) { //有label 
  	toks.label = toks.tokens[0].value;
  	if ( toks.tokens[1].tokentype == 4 ) { // 有label +開頭 (format4) 
  	  toks.format = 4;
  	  toks.label_length = 4;
  	  toks.ins = toks.tokens[2].value;
  	  sicxe_Search_Instruction_Set( sicxe, toks.tokens[2].value, toks.format, toks.opformat, toks.objectcode );
  	  if ( toks.opformat == 1 )    // (1) x (沒有的) [1/3/4]
  	  	if ( !toks.tokens[3].value.empty() )
  	  	  toks.error = "Syntax Error! : It should be nothing here.";
	  else if ( toks.opformat == 2 ) {  //(2) m          [3/4]
	  	if ( toks.tokens[3].value.empty() )
  	  	  toks.error = "Syntax Error! : It should have something here.";
  	  	else {
  	  	  if ( toks.tokens[3].tokentype == 4 && toks.tokens[3].tokenvalue == 12 ){ // Immediate Addressing #
  	  	    toks.i = 1;
  	  	    if ( toks.tokens[4].value.empty() )
  	  	      toks.error = "Syntax Error! : It should have something behind #.";
  	  	    else
  	  	      toks.group1 = toks.tokens[4].value;
  	      } // if Immediate Addressing #
  	      else if ( toks.tokens[3].tokentype == 4 && toks.tokens[3].tokenvalue == 13 ){ // Indirect Addressing @
  	  	    toks.n = 1;
  	  	    if ( toks.tokens[4].value.empty() )
  	  	      toks.error = "Syntax Error! : It should have something behind @.";
  	  	    else
  	  	      toks.group1 = toks.tokens[4].value;
  	      } // else if Indirect Addressing @
  	      else { // <direct>|<index>|<literal>
  	        toks.i = 1;
  	        toks.n = 1;
  	        if ( toks.tokens[4].tokentype == 4 && toks.tokens[4].tokenvalue == 1 ) { // <index>： <symbol>, X
  	          toks.x = 1;
  	          if ( toks.tokens[3].value.empty() )
  	            toks.error = "Syntax Error! : It's index mode.It should have something behind.";
  	          else
  	            toks.group1 = toks.tokens[3].value;
  	          if ( toks.tokens[5].value.empty() )
  	            toks.error = "Syntax Error! : It's index mode.It should have something behind.";
  	          else
  	            toks.group2 = toks.tokens[5].value;
  	        } // if <index>： <symbol>, X
  	        else if ( toks.tokens[3].tokentype == 4 && toks.tokens[3].tokenvalue == 11 ) { // <literal>： (1)= C''…(BYTE) (2) = X''…(BYTE) (3) =常數 (WORD)
  	          if ( toks.tokens[5].tokentype = 7 ) {  // (1)= C''…(BYTE) (C已經不見了 
  	            toks.literal.label = toks.tokens[5].value;
  	            toks.literal.WORDorBYTE = "BYTE";
                toks.literal.literal = toks.tokens[5].value;
                toks.literal.literal.insert( 0, toks.tokens[4].value );
                toks.literal.literal.insert( toks.literal.literal.length()-1, toks.tokens[4].value );
                toks.literal.literal.insert( 0, "C" );
                toks.group1 = toks.tokens[5].value;
			  } // if (1)= X''…(BYTE) (X已經不見了  
			  if ( toks.tokens[5].tokentype == 6 ) {  //  (2) = X''…(BYTE) (X已經不見了
  	            toks.literal.label = toks.tokens[5].value;
  	            toks.literal.WORDorBYTE = "BYTE";
                toks.literal.literal = toks.tokens[5].value;
                toks.literal.literal.insert( 0, toks.tokens[4].value );
                toks.literal.literal.insert( toks.literal.literal.length()-1, toks.tokens[4].value );
                toks.literal.literal.insert( 0, "X" );
                toks.group1 = toks.tokens[5].value;
			  } // if  (2) = C''…(BYTE) (C已經不見了
			  else if ( toks.tokens[4].tokentype == 6 ) {  // (3) =常數 (WORD) 
			    toks.literal.label = toks.tokens[4].value;
			    toks.literal.label.insert( 0, toks.tokens[3].value );
			    toks.literal.WORDorBYTE = "WORD";
			    toks.literal.literal = toks.tokens[4].value;
			    toks.group1 = toks.literal.label;
			  } // else if (3) =常數 (WORD)
  	        } // else if <literal>： (1)= C''…(BYTE) (2) = X''…(BYTE) (3) =常數 (WORD)
  	        else if ( !toks.tokens[3].value.empty() && toks.tokens[4].value.empty() ) { // <symbol> | address
  	          toks.group1 = toks.tokens[3].value;
  	        } // else if <symbol> | address
  	        else 
  	          toks.error = "Syntax Error! : It should be something here.";
		  } // else <direct>|<index>|<literal>
		} // else
	  } // else if (2) m          [3/4]
	} // if
	else if ( toks.tokens[1].tokentype == 1 ) { //format123 
	  toks.ins = toks.tokens[1].value;
  	  sicxe_Search_Instruction_Set( sicxe, toks.tokens[1].value, toks.format, toks.opformat, toks.objectcode );
  	  if ( toks.opformat == 1 )  {   // (1) x (沒有的) [1/3/4]  這裡只會有1/3的可能 
  	    toks.label_length = 1;
  	  	if ( !toks.tokens[2].value.empty() )
  	  	  toks.error = "Syntax Error! : It should be nothing here.";
      } // if
	  else if ( toks.opformat == 2 ) {  //(2) m          [3/4]  這裡只會有3的可能 
	    toks.label_length = 3;
	  	if ( toks.tokens[2].value.empty() )
  	  	  toks.error = "Syntax Error! : It should have something here.";
  	  	else {
  	  	  if ( toks.tokens[2].tokentype == 4 && toks.tokens[2].tokenvalue == 12 ){ // Immediate Addressing #
  	  	    if ( !sicxe_Set_p_b ( token_packer, toks, toks.tokens[3].value ) )
  	  	      toks.forwardreference = true;
  	  	    toks.i = 1;
  	  	    if ( toks.tokens[3].value.empty() )
  	  	      toks.error = "Syntax Error! : It should have something behind #.";
  	  	    else
  	  	      toks.group1 = toks.tokens[3].value;
  	      } // if Immediate Addressing #
  	      else if ( toks.tokens[2].tokentype == 4 && toks.tokens[2].tokenvalue == 13 ){ // Indirect Addressing @
  	        if ( !sicxe_Set_p_b ( token_packer, toks, toks.tokens[3].value ) )
  	  	      toks.forwardreference = true;
  	  	    toks.n = 1;
  	  	    if ( toks.tokens[3].value.empty() )
  	  	      toks.error = "Syntax Error! : It should have something behind @.";
  	  	    else
  	  	      toks.group1 = toks.tokens[3].value;
  	      } // else if Indirect Addressing @
  	      else { // <direct>|<index>|<literal>
  	        toks.i = 1;
  	        toks.n = 1;
  	        if ( toks.tokens[3].tokentype == 4 && toks.tokens[3].tokenvalue == 1 ) { // <index>： <symbol>, X
  	          if ( !sicxe_Set_p_b ( token_packer, toks, toks.tokens[2].value ) )
  	  	        toks.forwardreference = true;
  	          toks.x = 1;
  	          if ( toks.tokens[2].value.empty() )
  	            toks.error = "Syntax Error! : It's index mode.It should have something behind.";
  	          else
  	            toks.group1 = toks.tokens[2].value;
  	          if ( toks.tokens[4].value.empty() )
  	            toks.error = "Syntax Error! : It's index mode.It should have something behind.";
  	          else
  	            toks.group2 = toks.tokens[4].value;
  	        } // if <index>： <symbol>, X
  	        else if ( toks.tokens[2].tokentype == 4 && toks.tokens[2].tokenvalue == 11 ) { // <literal>： (1)= C''…(BYTE) (2) = X''…(BYTE) (3) =常數 (WORD)
  	          if ( toks.tokens[4].tokentype == 7 ) {  // (1)= C''…(BYTE) (C已經不見了 
  	            toks.literal.label = toks.tokens[4].value;
  	            toks.literal.WORDorBYTE = "BYTE";
                toks.literal.literal = toks.tokens[4].value;
                toks.literal.literal.insert( 0, toks.tokens[3].value );
                toks.literal.literal.insert( toks.literal.literal.length()-1, toks.tokens[3].value );
                toks.literal.literal.insert( 0, "C" );
                toks.group1 = toks.tokens[4].value;
			  } // if (1)= X''…(BYTE) (X已經不見了  
			  if ( toks.tokens[4].tokentype == 6 ) {  //  (2) = X''…(BYTE) (X已經不見了
  	            toks.literal.label = toks.tokens[4].value;
  	            toks.literal.WORDorBYTE = "BYTE";
                toks.literal.literal = toks.tokens[4].value;
                toks.literal.literal.insert( 0, toks.tokens[3].value );
                toks.literal.literal.insert( toks.literal.literal.length()-1, toks.tokens[3].value );
                toks.literal.literal.insert( 0, "X" );
                toks.group1 = toks.tokens[4].value;
			  } // if  (2) = C''…(BYTE) (C已經不見了
			  else if ( toks.tokens[3].tokentype == 6 ) {  // (3) =常數 (WORD) 
			    toks.literal.label = toks.tokens[3].value;
			    toks.literal.label.insert( 0, toks.tokens[2].value );
			    toks.literal.WORDorBYTE = "WORD";
			    toks.literal.literal = toks.tokens[3].value;
			    toks.group1 = toks.literal.label;
			  } // else if (3) =常數 (WORD)
  	        } // else if <literal>： (1)= C''…(BYTE) (2) = X''…(BYTE) (3) =常數 (WORD)
  	        else if ( !toks.tokens[2].value.empty() && toks.tokens[3].value.empty() ) { // <symbol> | address
  	          if ( !sicxe_Set_p_b ( token_packer, toks, toks.tokens[2].value ) )
  	  	        toks.forwardreference = true;
  	          toks.group1 = toks.tokens[2].value;
  	        } // else if <symbol> | address
  	        else 
  	          toks.error = "Syntax Error! : It should be something here.";
		  } // else <direct>|<index>|<literal>
		} // else
	  } // else if (2) m     [3/4] 裡只會有3的可能  
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
	else if ( toks.tokens[1].tokentype == 2 ) { // pseudo instruction 1.START 2.END 3.EQU 4.BYTE 5.WORD 6.LTORG 7.BASE 8.RESB 9.RESW
	  toks.ins = toks.tokens[1].value;
	  if ( toks.tokens[1].tokenvalue == 1 ) {//  {label} START hex_num
	    toks.group1 = toks.tokens[2].value;
	    toks.START = true;
	    int num = atoi(toks.tokens[2].value.c_str());
	    toks.location = num;  //設定起始位置 
      }// if START
      else if ( toks.tokens[1].tokenvalue == 2 || toks.tokens[1].tokenvalue == 6 ) { //  {label} END {label} || {label} LTORG
        toks.end =true;
        toks.group1 = toks.tokens[2].value;
	    token_packer.end = true;
      } // else of
      else if ( toks.tokens[1].tokenvalue == 3 ) {//  label EQU label | dec_num | *  四則運算限定於label與label
        toks.EQU = true;
        if ( toks.tokens[2].tokentype == 6 )
          toks.location = atoi(toks.tokens[2].value.c_str());
        toks.group1 = toks.tokens[2].value;
      }// else if EQU
      else if ( toks.tokens[1].tokenvalue == 4 ) {// {label} BYTE X''… | C''… | dec_num
        if ( toks.tokens[3].tokentype = 7 ) {  // '(4,9) EOF(7,18) '(4,9) // C'…(BYTE) (C已經不見了
          toks.group1 = toks.tokens[3].value;  //直接把 EOF 一個一個翻成十六進位 
	    } // if
	    else if ( toks.tokens[3].tokentype == 6 ) {  // '(4,9) F 1(6,51) '(4,9) // X''…(BYTE) (X已經不見了 
	      toks.group1 = toks.tokens[3].value;  //直接放object code    
		} // else if
		else if ( toks.tokens[2].tokentype == 6 ) {  // dec_num
	      toks.group1 = toks.tokens[2].value; 
		} // else if
      }// else if BYTE
      else if ( toks.tokens[1].tokenvalue == 5 ) {//  {label} WORD X''… | C''… | dec_num
	    if ( toks.tokens[3].tokentype = 7 ) {  // '(4,9) EOF(7,18) '(4,9) // C'…(WORD) (C已經不見了
          toks.group1 = toks.tokens[3].value;  //直接把 EOF 一個一個翻成十六進位 
	    } // if
	    else if ( toks.tokens[3].tokentype == 6 ) {  // '(4,9) F 1(6,51) '(4,9) // X''…(WORD) (X已經不見了 
	      toks.group1 = toks.tokens[3].value;  //直接放object code    
		} // else if
		else if ( toks.tokens[2].tokentype == 6 ) {  // dec_num
	      toks.group1 = toks.tokens[2].value; 
		} // else if
      }// else if WORD
      else if ( toks.tokens[1].tokenvalue == 7 ) {//  {label} BASE dec_num | symbol
        toks.base = true;
        token_packer.base = true;
        toks.group1 = toks.tokens[2].value; 
        token_packer.base_label = toks.tokens[2].value; 
      }// else if BASE
      else if ( toks.tokens[1].tokenvalue == 8 ) {//  {label} RESB dec_num
        toks.label_length = atoi(toks.tokens[2].value.c_str());
        toks.group1 = toks.tokens[2].value;
      }// else if RESB
      else if ( toks.tokens[1].tokenvalue == 9 ) {//  {label} RESW dec_num
        toks.label_length = atoi(toks.tokens[2].value.c_str())*3;
        //cout << toks.label_length ; // test
        toks.group1 = toks.tokens[2].value;
      }// else if RESW
	} // else if  pseudo instruction
	else if ( toks.tokens[0].tokentype == 5 && toks.tokens[1].tokentype != 1 && toks.tokens[1].tokentype != 2 && toks.tokens[1].tokentype != 4 )
      toks.error = "Syntax Error! : It doesn't have instructions.";
  } // if 有label 
  else if ( toks.tokens[0].tokentype == 4 ) {  //沒有label +開頭 (format4) 
    toks.label_length = 4;
  	  toks.ins = toks.tokens[1].value;
  	  sicxe_Search_Instruction_Set( sicxe, toks.tokens[1].value, toks.format, toks.opformat, toks.objectcode );
  	  toks.format = 4;
  	  if ( toks.opformat == 1 ) {   // (1) x (沒有的) [1/3/4]
  	  	if ( !toks.tokens[2].value.empty() )
  	  	  toks.error = "Syntax Error! : It should be nothing here.";
      } // if
	  else if ( toks.opformat == 2 ) {  //(2) m          [3/4]
	  	if ( toks.tokens[2].value.empty() )
  	  	  toks.error = "Syntax Error! : It should have something here.";
  	  	else {
  	  	  if ( toks.tokens[2].tokentype == 4 && toks.tokens[2].tokenvalue == 12 ){ // Immediate Addressing # 
  	  	    toks.i = 1;
  	  	    if ( toks.tokens[3].value.empty() )
  	  	      toks.error = "Syntax Error! : It should have something behind #.";
  	  	    else
  	  	      toks.group1 = toks.tokens[3].value;
  	      } // if Immediate Addressing #
  	      else if ( toks.tokens[2].tokentype == 4 && toks.tokens[2].tokenvalue == 13 ){ // Indirect Addressing @
  	  	    toks.n = 1;
  	  	    if ( toks.tokens[3].value.empty() )
  	  	      toks.error = "Syntax Error! : It should have something behind @.";
  	  	    else
  	  	      toks.group1 = toks.tokens[3].value;
  	      } // else if Indirect Addressing @
  	      else { // <direct>|<index>|<literal>
  	        toks.i = 1;
  	        toks.n = 1;
  	        if ( toks.tokens[3].tokentype == 4 && toks.tokens[3].tokenvalue == 1 ) { // <index>： <symbol>, X
  	          toks.x = 1;
  	          if ( toks.tokens[2].value.empty() )
  	            toks.error = "Syntax Error! : It's index mode.It should have something behind.";
  	          else
  	            toks.group1 = toks.tokens[2].value;
  	          if ( toks.tokens[4].value.empty() )
  	            toks.error = "Syntax Error! : It's index mode.It should have something behind.";
  	          else
  	            toks.group2 = toks.tokens[4].value;
  	        } // if <index>： <symbol>, X
  	        else if ( toks.tokens[2].tokentype == 4 && toks.tokens[2].tokenvalue == 11 ) { // <literal>： (1)= C''…(BYTE) (2) = X''…(BYTE) (3) =常數 (WORD)
  	          if ( toks.tokens[4].tokentype == 7 ) {  // (1)= C''…(BYTE) (C已經不見了 
  	            toks.literal.label = toks.tokens[4].value;
  	            toks.literal.WORDorBYTE = "BYTE";
                toks.literal.literal = toks.tokens[4].value;
                toks.literal.literal.insert( 0, toks.tokens[3].value );
                toks.literal.literal.insert( toks.literal.literal.length()-1, toks.tokens[3].value );
                toks.literal.literal.insert( 0, "C" );
                toks.group1 = toks.tokens[4].value;
			  } // if (1)= X''…(BYTE) (X已經不見了  
			  if ( toks.tokens[4].tokentype == 6 ) {  //  (2) = X''…(BYTE) (X已經不見了
  	            toks.literal.label = toks.tokens[4].value;
  	            toks.literal.WORDorBYTE = "BYTE";
                toks.literal.literal = toks.tokens[4].value;
                toks.literal.literal.insert( 0, toks.tokens[3].value );
                toks.literal.literal.insert( toks.literal.literal.length()-1, toks.tokens[3].value );
                toks.literal.literal.insert( 0, "X" );
                toks.group1 = toks.tokens[4].value;
			  } // if  (2) = C''…(BYTE) (C已經不見了
			  else if ( toks.tokens[3].tokentype == 6 ) {  // (3) =常數 (WORD) 
			    toks.literal.label = toks.tokens[3].value;
			    toks.literal.label.insert( 0, toks.tokens[2].value );
			    toks.literal.WORDorBYTE = "WORD";
			    toks.literal.literal = toks.tokens[3].value;
			    toks.group1 = toks.literal.label;
			  } // else if (3) =常數 (WORD)
  	        } // else if <literal>： (1)= C''…(BYTE) (2) = X''…(BYTE) (3) =常數 (WORD)
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
  else if ( toks.tokens[0].tokentype == 1 ) {  //沒有label format123
      toks.ins = toks.tokens[0].value;
  	  sicxe_Search_Instruction_Set( sicxe, toks.tokens[0].value, toks.format, toks.opformat, toks.objectcode );
  	  if ( toks.opformat == 1 ) {   // (1) x (沒有的) [1/3/4]  這裡只會有1/3的可能 
  	    toks.label_length = 1;
  	  	if ( !toks.tokens[1].value.empty() )
  	  	  toks.error = "Syntax Error! : It should be nothing here.";
      } // if
	  else if ( toks.opformat == 2 ) {  //(2) m          [3/4]  這裡只會有3的可能 
	    toks.label_length = 3;
	  	if ( toks.tokens[1].value.empty() )
  	  	  toks.error = "Syntax Error! : It should have something here.";
  	  	else {
  	  	  if ( toks.tokens[1].tokentype == 4 && toks.tokens[1].tokenvalue == 12 ){ // Immediate Addressing #
  	  	    if ( !sicxe_Set_p_b ( token_packer, toks, toks.tokens[2].value ) )
  	  	      toks.forwardreference = true;
  	  	    toks.i = 1;
  	  	    if ( toks.tokens[2].value.empty() )
  	  	      toks.error = "Syntax Error! : It should have something behind #.";
  	  	    else
  	  	      toks.group1 = toks.tokens[2].value;
  	      } // if Immediate Addressing #
  	      else if ( toks.tokens[1].tokentype == 4 && toks.tokens[1].tokenvalue == 13 ){ // Indirect Addressing @
  	        if ( !sicxe_Set_p_b ( token_packer, toks, toks.tokens[2].value ) )
  	  	      toks.forwardreference = true;
  	  	    toks.n = 1;
  	  	    if ( toks.tokens[2].value.empty() )
  	  	      toks.error = "Syntax Error! : It should have something behind @.";
  	  	    else
  	  	      toks.group1 = toks.tokens[2].value;
  	      } // else if Indirect Addressing @
  	      else { // <direct>|<index>|<literal>
  	        toks.i = 1;
  	        toks.n = 1;
  	        if ( toks.tokens[2].tokentype == 4 && toks.tokens[2].tokenvalue == 1 ) { // <index>： <symbol>, X
  	          if ( !sicxe_Set_p_b ( token_packer, toks, toks.tokens[1].value ) )
  	  	        toks.forwardreference = true;
  	          toks.x = 1;
  	          if ( toks.tokens[1].value.empty() )
  	            toks.error = "Syntax Error! : It's index mode.It should have something behind.";
  	          else
  	            toks.group1 = toks.tokens[1].value;
  	          if ( toks.tokens[3].value.empty() )
  	            toks.error = "Syntax Error! : It's index mode.It should have something behind.";
  	          else
  	            toks.group2 = toks.tokens[3].value;
  	        } // if <index>： <symbol>, X
  	        else if ( toks.tokens[1].tokentype == 4 && toks.tokens[1].tokenvalue == 11 ) { // <literal>： (1)= C''…(BYTE) (2) = X''…(BYTE) (3) =常數 (WORD)
  	          if ( toks.tokens[3].tokentype == 7 ) {  // (1)= C''…(BYTE) (C已經不見了 
  	            toks.literal.label = toks.tokens[3].value;
  	            toks.literal.WORDorBYTE = "BYTE";
                toks.literal.literal = toks.tokens[3].value;
                toks.literal.literal.insert( 0, toks.tokens[2].value );
                toks.literal.literal.insert( toks.literal.literal.length()-1, toks.tokens[2].value );
                toks.literal.literal.insert( 0, "C" );
                toks.group1 = toks.tokens[3].value;
			  } // if (1)= X''…(BYTE) (X已經不見了  
			  if ( toks.tokens[3].tokentype == 6 ) {  //  (2) = X''…(BYTE) (X已經不見了
  	            toks.literal.label = toks.tokens[3].value;
  	            toks.literal.WORDorBYTE = "BYTE";
                toks.literal.literal = toks.tokens[3].value;
                toks.literal.literal.insert( 0, toks.tokens[2].value );
                toks.literal.literal.insert( toks.literal.literal.length()-1, toks.tokens[2].value );
                toks.literal.literal.insert( 0, "X" );
                toks.group1 = toks.tokens[3].value;
			  } // if  (2) = C''…(BYTE) (C已經不見了
			  else if ( toks.tokens[2].tokentype == 6 ) {  // (3) =常數 (WORD) 
			    toks.literal.label = toks.tokens[2].value;
			    toks.literal.label.insert( 0, toks.tokens[1].value );
			    toks.literal.WORDorBYTE = "WORD";
			    toks.literal.literal = toks.tokens[2].value;
			    toks.group1 = toks.literal.label;
			  } // else if (3) =常數 (WORD)
  	        } // else if <literal>： (1)= C''…(BYTE) (2) = X''…(BYTE) (3) =常數 (WORD)
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
	  } // else if (2) m     [3/4] 裡只會有3的可能  
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
	} // else if 沒有label format123 
  else if ( toks.tokens[0].tokentype == 2 ) {  //沒有label pseudo instruction
	  toks.ins = toks.tokens[0].value;
	  if ( toks.tokens[0].tokenvalue == 1 ) {//  {label} START hex_num
	    toks.START = true;
	    toks.group1 = toks.tokens[1].value;
	    int num = atoi(toks.tokens[1].value.c_str());
	    toks.location = num;  //設定起始位置 
      }// if START
      else if ( toks.tokens[0].tokenvalue == 2 || toks.tokens[0].tokenvalue == 6 ) {//  {label} END {label} || {label} LTORG
        toks.end =true;
        toks.group1 = toks.tokens[1].value;
	    token_packer.end = true;
	  }// else if {label} END {label} || {label} LTORG
      else if ( toks.tokens[0].tokenvalue == 3 ) { //  label EQU label | dec_num | *  四則運算限定於label與label
        toks.EQU = true;
        if ( toks.tokens[1].tokentype == 6 )
          toks.location = atoi(toks.tokens[1].value.c_str());
        toks.group1 = toks.tokens[1].value;
      }// else if label EQU label | dec_num | *  四則運算限定於label與label
      else if ( toks.tokens[0].tokenvalue == 4 ) {// {label} BYTE X''… | C''… | dec_num
        if ( toks.tokens[2].tokentype == 7 ) {  // '(4,9) EOF(7,18) '(4,9) // C'…(BYTE) (C已經不見了
          toks.group1 = toks.tokens[2].value;  //直接把 EOF 一個一個翻成十六進位 
	    } // if
	    else if ( toks.tokens[2].tokentype == 6 ) {  // '(4,9) F 1(6,51) '(4,9) // X''…(BYTE) (X已經不見了 
	      toks.group1 = toks.tokens[2].value;  //直接放object code    
		} // else if
		else if ( toks.tokens[1].tokentype == 6 ) {  // dec_num
	      toks.group1 = toks.tokens[1].value; 
		} // else if
      }// else if BYTE
      else if ( toks.tokens[0].tokenvalue == 5 ) {//  {label} WORD X''… | C''… | dec_num
	    if ( toks.tokens[2].tokentype == 7 ) {  // '(4,9) EOF(7,18) '(4,9) // C'…(WORD) (C已經不見了
          toks.group1 = toks.tokens[2].value;  //直接把 EOF 一個一個翻成十六進位 
	    } // if
	    else if ( toks.tokens[2].tokentype == 6 ) {  // '(4,9) F 1(6,51) '(4,9) // X''…(WORD) (X已經不見了 
	      toks.group1 = toks.tokens[2].value;  //直接放object code    
		} // else if
		else if ( toks.tokens[1].tokentype == 6 ) {  // dec_num
	      toks.group1 = toks.tokens[1].value; 
		} // else if
      }// else if WORD
      else if ( toks.tokens[0].tokenvalue == 7 ) {//  {label} BASE dec_num | symbol
        toks.base = true;
        token_packer.base = true;
        toks.group1 = toks.tokens[1].value; 
        token_packer.base_label = toks.tokens[1].value; 
      }// else if BASE
      else if ( toks.tokens[0].tokenvalue == 8 ) {//  {label} RESB dec_num
        toks.label_length = atoi(toks.tokens[1].value.c_str());
        toks.group1 = toks.tokens[1].value;
      }// else if RESB
      else if ( toks.tokens[0].tokenvalue == 9 ) {//  {label} RESW dec_num
        toks.label_length = atoi(toks.tokens[1].value.c_str())*3;
        toks.group1 = toks.tokens[1].value;
      }// else if RESW
  } // else if 沒有label pseudo instruction
  else if ( toks.tokens[0].tokentype != 5 && toks.tokens[0].tokentype != 1 && toks.tokens[0].tokentype != 2 && toks.tokens[0].tokentype != 4 )
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
		if ( token_packer.token_groups[i].b && !token_packer.token_groups[i].forwardreference ) {
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
		//cout << token_packer.token_groups[i].ins << token_packer.token_groups[i].forwardreference <<endl ; // test
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
} //sic_setline

void sic_index_set_address( Tokens &toks, string address ) {
	// test 1039
	string temp;
	string x = "1";
	temp.append(1, address[0]); //1
	address.erase(0,1); //039
	// test 1
	HexaToBin( temp );
	// test 0001
	temp.erase(0,1);
	// test 001
	x.insert(x.length(),temp);
	// test 1001
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
		if ( token == token_packer.token_groups[i].label ) // 找得到
		  return true;
	} // for
    return false;
    // else 找不到
} // sic_havefind


void sic_setcode( Packed_Token token_packer, Tokens &toks ) {
	string address;
	string result;
	int asciicode = 0 ;
	if ( toks.x == 1 ) { //  index
	    if ( !toks.forwardreference ) {
	        //cout << toks.ins << toks.group1 << toks.group2 << toks.objectcode ; // test
	    	sic_set_address(token_packer, toks, address);
	    	sic_index_set_address( toks, address );
	    	//cout << toks.ins << toks.objectcode << endl; // test
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
	else if ( toks.x == 0 ) {
    	sic_set_address(token_packer, toks, address);
    	toks.objectcode.insert(toks.objectcode.length(),address);
    } // else if
} //sic_setcode

void sic_pass2( Packed_Token &token_packer ){
	for ( int i = 0 ; i < token_packer.amount ; i++ ) {
		string address;
		if ( token_packer.token_groups[i].forwardreference ) {
			if ( token_packer.token_groups[i].x == 1 ) { //  index
	          sic_set_address(token_packer, token_packer.token_groups[i], address);
	    	  sic_index_set_address( token_packer.token_groups[i], address );
            } // if  index
            else if ( token_packer.token_groups[i].x == 0 ) {
    	      sic_set_address(token_packer, token_packer.token_groups[i], address);
    	      token_packer.token_groups[i].objectcode.insert(token_packer.token_groups[i].objectcode.length(),address);
            } // else if 
		} // if
	} // for
} //sic_pass2

void sic_RecordAndSyntax( Packed_Token &token_packer, Tokens &toks, Sic_Instruction_Set sic[26] ) { 
  //cout << toks.tokens[0].tokentype << toks.tokens[1].tokentype << toks.tokens[2].tokentype << endl ; // test
  //cout << toks.tokens[0].tokenvalue << toks.tokens[1].tokenvalue << toks.tokens[2].tokenvalue <<endl ; // test
  //cout << toks.tokens[0].value << toks.tokens[1].value << toks.tokens[2].value <<endl ; // test
  if ( toks.tokens[0].tokentype == 5 ) { //有label 
  	toks.label = toks.tokens[0].value;
  	if ( toks.tokens[1].tokentype == 1 ) { // 有label instructions
  	  toks.label_length = 3;
  	  toks.ins = toks.tokens[1].value;
  	  sic_Search_Instruction_Set( sic, toks.tokens[1].value, toks.objectcode );
  	    if ( toks.tokens[3].tokentype == 4 && toks.tokens[3].tokenvalue == 1 ) { // <index>： <symbol>, X
  	        if ( !sic_havefind( token_packer, toks, toks.tokens[2].value ) )
  	  	        toks.forwardreference = true;
  	        toks.x = 1;
  	        if ( toks.tokens[2].value.empty() )
  	            toks.error = "Syntax Error! : It's index mode.It should have something behind.";
  	        else
  	            toks.group1 = toks.tokens[2].value;
  	        if ( toks.tokens[4].value.empty() )
  	            toks.error = "Syntax Error! : It's index mode.It should have something behind.";
  	        else
  	            toks.group2 = toks.tokens[4].value;
  	    } // if <index>： <symbol>, X
  	    else if ( toks.tokens[2].tokentype == 4 && toks.tokens[2].tokenvalue == 11 ) { // <literal>： (1)= C''…(BYTE) (2) = X''…(BYTE) (3) =常數 (WORD)
  	        if ( toks.tokens[4].tokentype == 7 ) {  // (1)= C''…(BYTE) (C已經不見了 
  	            toks.label_length = toks.tokens[4].value.length();
  	            toks.literal.c_x_w = "c";
  	            toks.literal.label = toks.tokens[4].value;
  	            toks.literal.WORDorBYTE = "BYTE";
                toks.literal.literal = toks.tokens[4].value;
                toks.literal.literal.insert( 0, toks.tokens[3].value );
                toks.literal.literal.insert( toks.literal.literal.length()-1, toks.tokens[3].value );
                toks.literal.literal.insert( 0, "C" );
                toks.group1 = toks.tokens[4].value;
			} // if (1)= X''…(BYTE) (X已經不見了  
			else if ( toks.tokens[4].tokentype == 6 ) {  //  (2) = X''…(BYTE) (X已經不見了
			    toks.label_length = toks.tokens[4].value.length()/2;
			    toks.literal.c_x_w = "x";
  	            toks.literal.label = toks.tokens[4].value;
  	            toks.literal.WORDorBYTE = "BYTE";
                toks.literal.literal = toks.tokens[4].value;
                toks.literal.literal.insert( 0, toks.tokens[3].value );
                toks.literal.literal.insert( toks.literal.literal.length()-1, toks.tokens[3].value );
                toks.literal.literal.insert( 0, "X" );
                toks.group1 = toks.tokens[4].value;
			} // else if  (2) = C''…(BYTE) (C已經不見了
			else if ( toks.tokens[3].tokentype == 6 ) {  // (3) =常數 (WORD) 
			    toks.label_length = 3;
			    toks.literal.c_x_w = "w";
			    toks.literal.label = toks.tokens[3].value;
			    toks.literal.label.insert( 0, toks.tokens[2].value );
			    toks.literal.WORDorBYTE = "WORD";
			    toks.literal.literal = toks.tokens[3].value;
			    toks.group1 = toks.literal.label;
			} // else if (3) =常數 (WORD)
  	    } // else if <literal>： (1)= C''…(BYTE) (2) = X''…(BYTE) (3) =常數 (WORD)
  	    else if ( !toks.tokens[2].value.empty() && toks.tokens[3].value.empty() ) { // <symbol> | address
  	        if ( !sic_havefind( token_packer, toks, toks.tokens[2].value ) )
  	  	        toks.forwardreference = true;
  	        toks.group1 = toks.tokens[2].value;
  	    } // else if <symbol> | address
  	    else if ( toks.tokens[1].value == "RSUB" )
  	      toks.objectcode = "4C0000";
  	    else 
  	        toks.error = "Syntax Error! : It should be something here.";
    } // if 有label instructions
	else if ( toks.tokens[1].tokentype == 2 ) { // 有label pseudo instruction 1.START 2.END 3.EQU 4.BYTE 5.WORD 6.LTORG 7.BASE 8.RESB 9.RESW
	  toks.ins = toks.tokens[1].value;
	  if ( toks.tokens[1].tokenvalue == 1 ) {//  {label} START hex_num
	    toks.pseudo = true;
	    int num = 0; 
	    toks.START = true;
	    toks.group1 = toks.tokens[2].value;
	    HexToDe( toks.tokens[2].value, num );
	    toks.location = num;  //設定起始位置 
      }// if START
      else if ( toks.tokens[1].tokenvalue == 2 || toks.tokens[1].tokenvalue == 6 ) { //  {label} END {label} || {label} LTORG
        toks.pseudo = true;
        toks.end =true;
        toks.group1 = toks.tokens[2].value;
	    token_packer.end = true;
      } // else of
      else if ( toks.tokens[1].tokenvalue == 3 ) {//  label EQU label | dec_num | *  四則運算限定於label與label
        toks.EQU = true;
        if ( toks.tokens[2].tokentype == 6 )
          toks.location = atoi(toks.tokens[2].value.c_str());
        toks.group1 = toks.tokens[2].value;
      }// else if EQU
      else if ( toks.tokens[1].tokenvalue == 4 ) {// {label} BYTE X''… | C''… | dec_num
        if ( toks.tokens[3].tokentype == 7 ) {  // '(4,9) EOF(7,18) '(4,9) // C'…(BYTE) (C已經不見了
          toks.c_x_w = "c";
          toks.label_length = toks.tokens[3].value.length();
          toks.group1 = toks.tokens[3].value;  //直接把 EOF 一個一個翻成十六進位 
	    } // if
	    else if ( toks.tokens[3].tokentype == 6 ) {  // '(4,9) F 1(6,51) '(4,9) // X''…(BYTE) (X已經不見了 
	      toks.c_x_w = "x";
	      toks.label_length = toks.tokens[3].value.length()/2;
	      toks.group1 = toks.tokens[3].value;  //直接放object code    
		} // else if
		else if ( toks.tokens[2].tokentype == 6 ) {  // dec_num
		  toks.label_length = 3;
	      toks.group1 = toks.tokens[2].value; 
		} // else if
      }// else if BYTE
      else if ( toks.tokens[1].tokenvalue == 5 ) {//  {label} WORD X''… | C''… | dec_num
        if ( toks.tokens[2].tokentype == 6 ) {  // dec_num
          toks.c_x_w = "w";
		  toks.label_length = 3;
	      toks.group1 = toks.tokens[2].value; 
		} // if
	    else if ( toks.tokens[3].tokentype == 6 ) {  // '(4,9) F 1(6,51) '(4,9) // X''…(WORD) (X已經不見了 
	      toks.c_x_w = "x";
	      toks.label_length = toks.tokens[3].value.length()/2;
	      toks.group1 = toks.tokens[3].value;  //直接放object code  
		} // else if
		else if ( toks.tokens[3].tokentype == 7 ) {  // '(4,9) EOF(7,18) '(4,9) // C'…(WORD) (C已經不見了
		  toks.c_x_w = "c";
	      toks.label_length = toks.tokens[3].value.length();
          toks.group1 = toks.tokens[3].value;  //直接把 EOF 一個一個翻成十六進位 
	    } // else if
      }// else if WORD
      else if ( toks.tokens[1].tokenvalue == 7 ) {//  {label} BASE dec_num | symbol
        toks.pseudo = true;
        token_packer.base = true;
        toks.group1 = toks.tokens[2].value; 
      }// else if BASE
      else if ( toks.tokens[1].tokenvalue == 8 ) {//  {label} RESB dec_num
        toks.pseudo = true;
        toks.label_length = atoi(toks.tokens[2].value.c_str());
        toks.group1 = toks.tokens[2].value;
      }// else if RESB
      else if ( toks.tokens[1].tokenvalue == 9 ) {//  {label} RESW dec_num
        toks.pseudo = true;
        toks.label_length = atoi(toks.tokens[2].value.c_str())*3;
        //cout << toks.label_length ; // test
        toks.group1 = toks.tokens[2].value;
      }// else if RESW
	} // else if  pseudo instruction
	else if ( toks.tokens[0].tokentype == 5 && toks.tokens[1].tokentype != 1 && toks.tokens[1].tokentype != 2 )
      toks.error = "Syntax Error! : It doesn't have instructions.";
  } // if 有label 
  else if ( toks.tokens[0].tokentype == 1 ) {  //沒有label  instrunctions
    toks.label_length = 3;
  	toks.ins = toks.tokens[0].value;
  	sic_Search_Instruction_Set( sic, toks.tokens[0].value, toks.objectcode );
  	    if ( toks.tokens[2].tokentype == 4 && toks.tokens[2].tokenvalue == 1 ) { // <index>： <symbol>, X
  	        if ( !sic_havefind( token_packer, toks, toks.tokens[1].value ) )
  	  	        toks.forwardreference = true;
  	        toks.x = 1;
  	        if ( toks.tokens[1].value.empty() )
  	            toks.error = "Syntax Error! : It's index mode.It should have something behind.";
  	        else
  	            toks.group1 = toks.tokens[1].value;
  	        if ( toks.tokens[3].value.empty() )
  	            toks.error = "Syntax Error! : It's index mode.It should have something behind.";
  	        else
  	            toks.group2 = toks.tokens[3].value;
  	    } // if <index>： <symbol>, X
  	    else if ( toks.tokens[1].tokentype == 4 && toks.tokens[1].tokenvalue == 11 ) { // <literal>： (1)= C''…(BYTE) (2) = X''…(BYTE) (3) =常數 (WORD)
  	        if ( toks.tokens[3].tokentype == 7 ) {  // (1)= C''…(BYTE) (C已經不見了 
  	            toks.label_length = toks.tokens[3].value.length();
  	            toks.literal.c_x_w = "c";
  	            toks.literal.label = toks.tokens[3].value;
  	            toks.literal.WORDorBYTE = "BYTE";
                toks.literal.literal = toks.tokens[3].value;
                toks.literal.literal.insert( 0, toks.tokens[1].value );
                toks.literal.literal.insert( toks.literal.literal.length()-1, toks.tokens[2].value );
                toks.literal.literal.insert( 0, "C" );
                toks.group1 = toks.tokens[3].value;
			} // if (1)= X''…(BYTE) (X已經不見了  
			else if ( toks.tokens[3].tokentype == 6 ) {  //  (2) = X''…(BYTE) (X已經不見了
			    toks.label_length = toks.tokens[3].value.length()/2;
			    toks.literal.c_x_w = "x";
  	            toks.literal.label = toks.tokens[3].value;
  	            toks.literal.WORDorBYTE = "BYTE";
                toks.literal.literal = toks.tokens[3].value;
                toks.literal.literal.insert( 0, toks.tokens[2].value );
                toks.literal.literal.insert( toks.literal.literal.length()-1, toks.tokens[2].value );
                toks.literal.literal.insert( 0, "X" );
                toks.group1 = toks.tokens[3].value;
			} // else if  (2) = C''…(BYTE) (C已經不見了
			else if ( toks.tokens[2].tokentype == 6 ) {  // (3) =常數 (WORD) 
			    toks.label_length = 3;
			    toks.literal.c_x_w = "w";
			    toks.literal.label = toks.tokens[2].value;
			    toks.literal.label.insert( 0, toks.tokens[1].value );
			    toks.literal.WORDorBYTE = "WORD";
			    toks.literal.literal = toks.tokens[2].value;
			    toks.group1 = toks.literal.label;
			} // else if (3) =常數 (WORD)
  	    } // else if <literal>： (1)= C''…(BYTE) (2) = X''…(BYTE) (3) =常數 (WORD)
  	    else if ( !toks.tokens[1].value.empty() && toks.tokens[2].value.empty() ) { // <symbol> | address
  	        if ( !sic_havefind( token_packer, toks, toks.tokens[1].value ) )
  	  	        toks.forwardreference = true;
  	        toks.group1 = toks.tokens[1].value;
  	    } // else if <symbol> | address
  	    else if ( toks.tokens[0].value == "RSUB" )
  	      toks.objectcode = "4C0000";
  	    else 
  	        toks.error = "Syntax Error! : It should be something here.";
    } // else if 沒有label  instrunctions
    else if ( toks.tokens[0].tokentype == 2 ) {  //沒有label pseudo instruction
	    toks.ins = toks.tokens[0].value;
	    if ( toks.tokens[0].tokenvalue == 1 ) {//  {label} START hex_num
	      toks.pseudo = true;
	      int num = 0; 
	      toks.START = true;
	      toks.group1 = toks.tokens[1].value;
	      HexToDe( toks.tokens[2].value, num );
	      toks.location = num;  //設定起始位置 
        }// if START
        else if ( toks.tokens[0].tokenvalue == 2 || toks.tokens[0].tokenvalue == 6 ) {//  {label} END {label} || {label} LTORG
          toks.pseudo = true;
          toks.end =true;
          toks.group1 = toks.tokens[1].value;
	      token_packer.end = true;
	    }// else if {label} END {label} || {label} LTORG
        else if ( toks.tokens[0].tokenvalue == 3 ) { //  label EQU label | dec_num | *  四則運算限定於label與label
          toks.EQU = true;
          if ( toks.tokens[1].tokentype == 6 )
            toks.location = atoi(toks.tokens[1].value.c_str());
          toks.group1 = toks.tokens[1].value;
        }// else if label EQU label | dec_num | *  四則運算限定於label與label
        else if ( toks.tokens[0].tokenvalue == 4 ) {// {label} BYTE X''… | C''… | dec_num
          if ( toks.tokens[1].tokentype == 6 ) {  // dec_num
		    toks.label_length = 3;
	        toks.group1 = toks.tokens[1].value; 
		  } // if
          else if ( toks.tokens[2].tokentype == 7 ) {  // '(4,9) EOF(7,18) '(4,9) // C'…(BYTE) (C已經不見了
            toks.c_x_w = "c";
            toks.label_length = toks.tokens[2].value.length();
            toks.group1 = toks.tokens[2].value;  //直接把 EOF 一個一個翻成十六進位 
	      } // else if
	      else if ( toks.tokens[2].tokentype == 6 ) {  // '(4,9) F 1(6,51) '(4,9) // X''…(BYTE) (X已經不見了 
	        toks.c_x_w = "x";
	        toks.label_length = toks.tokens[2].value.length()/2;
	        toks.group1 = toks.tokens[2].value;  //直接放object code    
	  	  } // else if
        }// else if BYTE
        else if ( toks.tokens[0].tokenvalue == 5 ) {//  {label} WORD X''… | C''… | dec_num
	      if ( toks.tokens[2].tokentype == 7 ) {  // '(4,9) EOF(7,18) '(4,9) // C'…(WORD) (C已經不見了
	        toks.c_x_w = "c";
	        toks.label_length = toks.tokens[2].value.length();
            toks.group1 = toks.tokens[2].value;  //直接把 EOF 一個一個翻成十六進位 
	      } // if
	      else if ( toks.tokens[2].tokentype == 6 ) {  // '(4,9) F 1(6,51) '(4,9) // X''…(WORD) (X已經不見了 
	        toks.c_x_w = "x";
	        toks.label_length = toks.tokens[2].value.length()/2;
	        toks.group1 = toks.tokens[2].value;  //直接放object code    
	  	  } // else if
		  else if ( toks.tokens[1].tokentype == 6 ) {  // dec_num
		    toks.c_x_w = "w";
		    toks.label_length = 3;
	        toks.group1 = toks.tokens[1].value; 
		  } // else if
        }// else if WORD
        else if ( toks.tokens[0].tokenvalue == 7 ) {//  {label} BASE dec_num | symbol
          token_packer.base = true;
          toks.group1 = toks.tokens[1].value; 
        }// else if BASE
        else if ( toks.tokens[0].tokenvalue == 8 ) {//  {label} RESB dec_num
          toks.pseudo = true;
          toks.label_length = atoi(toks.tokens[1].value.c_str());
          toks.group1 = toks.tokens[1].value;
        }// else if RESB
        else if ( toks.tokens[0].tokenvalue == 9 ) {//  {label} RESW dec_num
          toks.pseudo = true;
          toks.label_length = atoi(toks.tokens[1].value.c_str())*3;
          toks.group1 = toks.tokens[1].value;
        }// else if RESW
    } // else if 沒有label pseudo instruction
    else if ( toks.tokens[0].tokentype != 5 && toks.tokens[0].tokentype != 1 && toks.tokens[0].tokentype != 2 )
      toks.error = "Syntax Error! : It doesn't have label and instructions.";
} // sic_RecordAndSyntax

//----------------------------sic-----------------------------------------



void sic () {
	Sic_Instruction_Set sic[26];  //放sic的指令表 
	Sic_Instruction_input( sic );
	//Sic_Instruction_print( sic ); // test
	Table table1[59];  //放table1  59個元素     Instruction
	Table table2[9];  //放table2  9個元素       Pseudo and Extra 我增加了EQU LTORG BASE
	Table table3[9];  //放table3  9個元素       Register
	Table table4[13];  //放table4  13個元素     Delimiter
	Table table5[100];  //放table5  100個元素   Symbol
	Table table6[100];  //放table6  100個元素   Integer/Real
	Table table7[100];  //放table7  100個元素   String , table4 );
	Packed_Token token_packer; //放一行一行包裝好的Tokens
	token_packer.amount = 0;
	tableinput( table1, table2, table3, table4 );
    ifstream newfile; 
    ofstream outfile;
    string demo("SIC_input.txt");  //先寫一個字串demo 
	newfile.open(demo.c_str()); //開啟檔案
	demo.replace(demo.find("input"), 5, "output");
	outfile.open(demo.c_str()); //寫入檔案
	string line;
	string token;
	int tokentype = 0;
	int tokenvalue = 0;
	int is_string = 0; // C'EOF'代表EOF字串
	int is_integer = 0; // X'F1'代表16進位的F1
	while ( getline(newfile, line) ) {
	  if ( !line.empty() )
	    if ( token_packer.amount != 0 )
	      token_packer.token_groups[token_packer.amount].line =  token_packer.token_groups[token_packer.amount-1].line + 5;
	    else 
	      token_packer.token_groups[token_packer.amount].line = 5;
	  token_packer.token_groups[token_packer.amount].sourcestatement = line ;  //每行的存原始程式 
	  if ( token_packer.token_groups[token_packer.amount].sourcestatement[0] == '\t' )
	    token_packer.token_groups[token_packer.amount].sourcestatement.erase( 0, 1 );
	  if ( token_packer.longestnum < line.length() ) //紀錄最長的source 
	    token_packer.longestnum = line.length();
	  //cout << line << endl ; // test
	  token_packer.token_groups[token_packer.amount].amount = 0;  //每行的token從頭開始算要歸零 
	  is_string = 0;
	  is_integer = 0;
	  for ( int a = 0; a < line.length() ; a++ ) { 
	  	if ( line[a] == '.' ) { 
	  	  token_packer.token_groups[token_packer.amount].comment = true;
	  	  break;
	  	} // if 
	  	token.append(1, line[a]);
	  	if ( is_string != 1 && is_integer != 1 ) {
	  	  while ( !iswhitespce( line[a] ) && !isOp( line[a] ) && a+1 < line.length() ){ //取得token 
            a++;
	  		token.append(1, line[a]);
	  		//cout << endl << token ; // test
	  		int end = a+1;
	  		if ( end >= line.length() )
	  		  break;
		  } //while
		} // if
		else if ( is_string == 1 || is_integer == 1 ) {
		  while ( int(line[a+1]) != 39 ){ //string或integer取得token 
            a++;
	  		token.append(1, line[a]);
		  } //while
		} // if else
		
		//cout << endl << token ; // test
		if ( int(line[a]) == 39 && (int(line[a-1]) == 99 || int(line[a-1]) == 67) ) { // C'EOF'代表EOF字串
		  token.erase(0,1);
		  is_string ++;
	    } // if
	    if ( int(line[a]) == 39 && (int(line[a-1]) == 120 || int(line[a-1]) == 88) ) { // X'F1'代表16進位的F1
	      token.erase(0,1);
		  is_integer ++;
	    } // if
	    
	    if ( iswhitespce( line[a] ) )
	       removewhitespace( token ); //先移除whitespce 
	    
	    if ( isOp( line[a] ) && is_string != 1 && is_integer != 1 && token.length() != 1 ) {  //兩個token 取得前面那個token然後倒退後面那個 delimiter 下一個在做 
			token.erase(token.length()-1, 1); //刪掉後面那個 delimiter
			if ( find( token, table1, table2, table3, table4, 
              table5, table6, table7, tokentype, tokenvalue ) ) { //能在table裡找到 
              ins_packer(token_packer.token_groups[token_packer.amount], token, tokentype, tokenvalue);
		    } // if 
		    else if ( !find( token, table1, table2, table3, table4, 
              table5, table6, table7, tokentype, tokenvalue ) ) { // 在table找不到，自己建 
              string upper = to_upper(token); 
		      if ( isNumber( upper ) ) {  //判斷是否為table6:數字
		        tokentype = 6;
		        instable( upper, table6, tokentype, tokenvalue);  
		      } // if
		      else {  //判斷是否為table5
		        tokentype = 5;
		        instable( upper, table5, tokentype, tokenvalue);
		      } // else
		      ins_packer(token_packer.token_groups[token_packer.amount], token, tokentype, tokenvalue);
		    } //else  if 
			a--; //倒退 
		}//if 
		else if ( isOp( line[a] ) && token.length() == 1 ) {  //一個token delimiter 
		  if ( find( token, table1, table2, table3, table4, 
            table5, table6, table7, tokentype, tokenvalue ) ) { //能在table裡找到 
            ins_packer(token_packer.token_groups[token_packer.amount], token, tokentype, tokenvalue);
		  } // if 
		}//else  if 
		else if ( is_string == 1 && !isOp( line[a] )  ) {  //是str !isOp( line[a] ) ->避免是' 
          tokentype = 7;
		  instable( token, table7, tokentype, tokenvalue);
		  ins_packer(token_packer.token_groups[token_packer.amount], token, tokentype, tokenvalue);
		  is_string ++;
		}//else  if
		else if ( is_integer == 1 && !isOp( line[a] )  ) {  //是integer  !isOp( line[a] ) ->避免是'
          string upper = to_upper(token); 
          tokentype = 6;
		  instable( upper, table6, tokentype, tokenvalue);
		  ins_packer(token_packer.token_groups[token_packer.amount], token, tokentype, tokenvalue); 
		  is_integer ++;
		}//else  if
		else if ( !onespace( token ) && !token.empty() ){ 
		  if ( find( token, table1, table2, table3, table4, 
            table5, table6, table7, tokentype, tokenvalue ) ) { //能在table裡找到 
            ins_packer(token_packer.token_groups[token_packer.amount], token, tokentype, tokenvalue);
		  } // if 
		  else if ( !find( token, table1, table2, table3, table4, 
            table5, table6, table7, tokentype, tokenvalue ) ) { // 在table找不到，自己建 
		    if ( is_integer == 1 ) { //判斷是否為table7:literal('字串') 
		      tokentype = 7;
		      instable( token, table7, tokentype, tokenvalue);
	     	} // if
		    else if ( isNumber( token ) ) { //判斷是否為table6:數字
		      string upper = to_upper(token); 
		      tokentype = 6;
		      instable( upper, table6, tokentype, tokenvalue); 
		    } // else if
		    else {  //判斷是否為table5
		      string upper = to_upper(token); 
		      tokentype = 5;
		      instable( upper, table5, tokentype, tokenvalue); 
		    } // else
		    ins_packer(token_packer.token_groups[token_packer.amount], token, tokentype, tokenvalue);
		  } //else  if 
		} // else  if	
	    token.clear();
	  } // for 
	  if ( !token_packer.token_groups[token_packer.amount].sourcestatement.empty() && !token_packer.token_groups[token_packer.amount].comment )
	    sic_RecordAndSyntax( token_packer, token_packer.token_groups[token_packer.amount], sic ) ; // 紀錄一行一行的label ins group1 group2/ 紀錄nixpb和是否forward reference
	  int temp = token_packer.amount-1; //上一個 
	  if ( temp != -1 )
	    while ( token_packer.token_groups[temp].EQU )
	      temp--;
	  if ( temp != -1 && !token_packer.token_groups[token_packer.amount].EQU && !token_packer.token_groups[token_packer.amount].START )
	    token_packer.token_groups[token_packer.amount].location = token_packer.token_groups[temp].location + token_packer.token_groups[temp].label_length;
	  sic_setlocation ( token_packer.token_groups[token_packer.amount].location, token_packer.token_groups[token_packer.amount].hex_location );
	  sic_setline( token_packer.token_groups[token_packer.amount].setedline, token_packer.token_groups[token_packer.amount].line ) ;
	  if ( token_packer.token_groups[token_packer.amount].ins != "RSUB" )
	    sic_setcode( token_packer, token_packer.token_groups[token_packer.amount] );
	  //cout << token_packer.token_groups[token_packer.amount].ins << token_packer.token_groups[token_packer.amount].hex_location << endl ; // test
	  token_packer.amount++;
	} // while
	sic_pass2(token_packer);
	newfile.close();//讀檔完後關閉檔案
	outfile << "Line" << "  " << "Location" << "  " ;
	outfile << "Source code" ;
	for ( int i = 10 ; i < 50 ; i++ )
	  outfile << " ";
	outfile << "  ";
	outfile << "Object code" << endl ;
	for ( int b = 0 ; b < token_packer.amount; b++) { //TEST
	    if( token_packer.token_groups[b].sourcestatement.empty() )
	      outfile << endl ;
	    else {
	      outfile << token_packer.token_groups[b].setedline << "  ";
	      if ( token_packer.token_groups[b].error.empty() ) {
	      	if ( token_packer.token_groups[b].end || token_packer.token_groups[b].comment )
	        outfile << "          ";
	        else
	          outfile << token_packer.token_groups[b].hex_location << "      ";
	        if ( token_packer.token_groups[b].label.empty() && !token_packer.token_groups[b].comment )
	          outfile << "        ";
		    outfile << token_packer.token_groups[b].sourcestatement << "  ";
		    for ( int i = token_packer.token_groups[b].sourcestatement.length() ; i < 50 ; i++ )
	          outfile << " ";
	        //cout << token_packer.token_groups[b].ins << token_packer.token_groups[b].objectcode << endl ; // test
	        if ( !token_packer.token_groups[b].pseudo && !token_packer.token_groups[b].comment )
		      outfile << token_packer.token_groups[b].objectcode;
		    //outfile <<  getpacker( token_packer.token_groups[b]) ; //寫入檔案 //TEST
		    outfile << endl;
		  } // if
		  else
		    outfile << token_packer.token_groups[b].error << endl;
		} // else
	} // for
	outfile.close();//寫檔完後關閉檔案 
} // sic

void sicxe () {
	Sicxe_Instruction_Set sicxe[59];  //放sicxe的指令表 
	Sicxe_Instruction_input( sicxe );
	Table table1[59];  //放table1  59個元素     Instruction
	Table table2[9];  //放table2  9個元素       Pseudo and Extra 我增加了EQU LTORG BASE
	Table table3[9];  //放table3  9個元素       Register
	Table table4[13];  //放table4  13個元素     Delimiter
	Table table5[100];  //放table5  100個元素   Symbol
	Table table6[100];  //放table6  100個元素   Integer/Real
	Table table7[100];  //放table7  100個元素   String , table4 );
	Packed_Token token_packer; //放一行一行包裝好的Tokens
	token_packer.amount = 0;
	tableinput( table1, table2, table3, table4 );
    ifstream newfile; 
    ofstream outfile;
    string demo("SICXE_input.txt");  //先寫一個字串demo 
	newfile.open(demo.c_str()); //開啟檔案
	demo.replace(demo.find("input"), 5, "output");
	outfile.open(demo.c_str()); //寫入檔案
	string line;
	string token;
	int tokentype = 0;
	int tokenvalue = 0;
	int is_string = 0; // C'EOF'代表EOF字串
	int is_integer = 0; // X'F1'代表16進位的F1
	while ( getline(newfile, line) ) {
		//cout << line << endl ; // test
	  if ( !line.empty() )
	    if ( token_packer.amount != 0 )
	      token_packer.token_groups[token_packer.amount].line =  token_packer.token_groups[token_packer.amount-1].line + 5;
	    else 
	      token_packer.token_groups[token_packer.amount].line = 5;
	  token_packer.token_groups[token_packer.amount].sourcestatement = line ;  //每行的存原始程式 
	  if ( token_packer.token_groups[token_packer.amount].sourcestatement[0] == '\t' )
	    token_packer.token_groups[token_packer.amount].sourcestatement.erase( 0, 1 );
	  if ( token_packer.longestnum < line.length() ) //紀錄最長的source 
	    token_packer.longestnum = line.length();
	  //cout << line << endl ; // test
	  token_packer.token_groups[token_packer.amount].amount = 0;  //每行的token從頭開始算要歸零 
	  is_string = 0;
	  is_integer = 0;
	  for ( int a = 0; a < line.length() ; a++ ) { 
	  	if ( line[a] == '.' ) { 
	  	  token_packer.token_groups[token_packer.amount].comment = true;
	  	  break;
	  	} // if 
	  	token.append(1, line[a]);
	  	if ( is_string != 1 && is_integer != 1 ) {
	  	  while ( !iswhitespce( line[a] ) && !isOp( line[a] ) && a+1 < line.length() ){ //取得token 
            a++;
	  		token.append(1, line[a]);
	  		//cout << endl << token ; // test
	  		int end = a+1;
	  		if ( end >= line.length() )
	  		  break;
		  } //while
		} // if
		else if ( is_string == 1 || is_integer == 1 ) {
		  while ( int(line[a+1]) != 39 ){ //string或integer取得token 
            a++;
	  		token.append(1, line[a]);
		  } //while
		} // if else
		
		//cout << endl << token ; // test
		if ( int(line[a]) == 39 && (int(line[a-1]) == 99 || int(line[a-1]) == 67) ) { // C'EOF'代表EOF字串
		  token.erase(0,1);
		  is_string ++;
	    } // if
	    if ( int(line[a]) == 39 && (int(line[a-1]) == 120 || int(line[a-1]) == 88) ) { // X'F1'代表16進位的F1
	      token.erase(0,1);
		  is_integer ++;
	    } // if
	    
	    if ( iswhitespce( line[a] ) )
	       removewhitespace( token ); //先移除whitespce 
	    
	    if ( isOp( line[a] ) && is_string != 1 && is_integer != 1 && token.length() != 1 ) {  //兩個token 取得前面那個token然後倒退後面那個 delimiter 下一個在做 
			token.erase(token.length()-1, 1); //刪掉後面那個 delimiter
			if ( find( token, table1, table2, table3, table4, 
              table5, table6, table7, tokentype, tokenvalue ) ) { //能在table裡找到 
              ins_packer(token_packer.token_groups[token_packer.amount], token, tokentype, tokenvalue);
		    } // if 
		    else if ( !find( token, table1, table2, table3, table4, 
              table5, table6, table7, tokentype, tokenvalue ) ) { // 在table找不到，自己建 
              string upper = to_upper(token); 
		      if ( isNumber( upper ) ) {  //判斷是否為table6:數字
		        tokentype = 6;
		        instable( upper, table6, tokentype, tokenvalue);  
		      } // if
		      else {  //判斷是否為table5
		        tokentype = 5;
		        instable( upper, table5, tokentype, tokenvalue);
		      } // else
		      ins_packer(token_packer.token_groups[token_packer.amount], token, tokentype, tokenvalue);
		    } //else  if 
			a--; //倒退 
		}//if 
		else if ( isOp( line[a] ) && token.length() == 1 ) {  //一個token delimiter 
		  if ( find( token, table1, table2, table3, table4, 
            table5, table6, table7, tokentype, tokenvalue ) ) { //能在table裡找到 
            ins_packer(token_packer.token_groups[token_packer.amount], token, tokentype, tokenvalue);
		  } // if 
		}//else  if 
		else if ( is_string == 1 && !isOp( line[a] )  ) {  //是str !isOp( line[a] ) ->避免是' 
          tokentype = 7;
		  instable( token, table7, tokentype, tokenvalue);
		  ins_packer(token_packer.token_groups[token_packer.amount], token, tokentype, tokenvalue);
		  is_string ++;
		}//else  if
		else if ( is_integer == 1 && !isOp( line[a] )  ) {  //是integer  !isOp( line[a] ) ->避免是'
          string upper = to_upper(token); 
          tokentype = 6;
		  instable( upper, table6, tokentype, tokenvalue);
		  ins_packer(token_packer.token_groups[token_packer.amount], token, tokentype, tokenvalue); 
		  is_integer ++;
		}//else  if
		else if ( !onespace( token ) && !token.empty() ){ 
		  if ( find( token, table1, table2, table3, table4, 
            table5, table6, table7, tokentype, tokenvalue ) ) { //能在table裡找到 
            ins_packer(token_packer.token_groups[token_packer.amount], token, tokentype, tokenvalue);
		  } // if 
		  else if ( !find( token, table1, table2, table3, table4, 
            table5, table6, table7, tokentype, tokenvalue ) ) { // 在table找不到，自己建 
		    if ( is_integer == 1 ) { //判斷是否為table7:literal('字串') 
		      tokentype = 7;
		      instable( token, table7, tokentype, tokenvalue);
	     	} // if
		    else if ( isNumber( token ) ) { //判斷是否為table6:數字或結尾為H
		      string upper = to_upper(token); 
		      tokentype = 6;
		      instable( upper, table6, tokentype, tokenvalue); 
		    } // else if
		    else {  //判斷是否為table5
		      string upper = to_upper(token); 
		      tokentype = 5;
		      instable( upper, table5, tokentype, tokenvalue); 
		    } // else
		    ins_packer(token_packer.token_groups[token_packer.amount], token, tokentype, tokenvalue);
		  } //else  if 
		} // else  if	
	    token.clear();
	  } // for 
	  if ( !token_packer.token_groups[token_packer.amount].sourcestatement.empty() && !token_packer.token_groups[token_packer.amount].comment )
	     sicxe_RecordAndSyntax( token_packer, token_packer.token_groups[token_packer.amount], sicxe ) ; // 紀錄一行一行的label ins group1 group2/ 紀錄nixpb和是否forward reference
	  int temp = token_packer.amount-1; //上一個 
	  if ( temp != -1 )
	    while ( token_packer.token_groups[temp].EQU )
	      temp--;
	  if ( temp != -1 && !token_packer.token_groups[token_packer.amount].EQU && !token_packer.token_groups[token_packer.amount].START )
	    token_packer.token_groups[token_packer.amount].location = token_packer.token_groups[temp].location + token_packer.token_groups[temp].label_length;
	  sicxe_setlocation ( token_packer.token_groups[token_packer.amount].location, token_packer.token_groups[token_packer.amount].hex_location );
	  sicxe_setline( token_packer.token_groups[token_packer.amount].setedline, token_packer.token_groups[token_packer.amount].line ) ;
	  sicxe_setcode( token_packer, token_packer.token_groups[token_packer.amount] );
	  token_packer.amount++;
	} // while
	sicxe_setbase(token_packer);
	sicxe_resetcodedisp(token_packer); //format3 && !forwardreference
	sicxe_pass2( token_packer );
	newfile.close();//讀檔完後關閉檔案
	outfile << "Line" << "  " << "Location" << "  " ;
	outfile << "Source code" ;
	for ( int i = 10 ; i < 50 ; i++ )
	  outfile << " ";
	outfile << "  ";
	outfile << "Object code" << endl ;
	outfile << "----" << "  " << "--------" << "  " ;
	outfile << "-----------" ;
	for ( int i = 10 ; i < 50 ; i++ )
	  outfile << " ";
	outfile << "  ";
	outfile << "-----------" << endl ;
	for ( int b = 0 ; b < token_packer.amount; b++) { //TEST
	    outfile << token_packer.token_groups[b].setedline << "  ";
	    if ( token_packer.token_groups[b].error.empty() ) {
	      if ( token_packer.token_groups[b].end || token_packer.token_groups[b].comment || token_packer.token_groups[b].base )
	        outfile << "          ";
	      else
	        outfile << token_packer.token_groups[b].hex_location << "      ";
	      if ( token_packer.token_groups[b].label.empty() )
	        outfile << "        ";
		  outfile << token_packer.token_groups[b].sourcestatement << "  ";
		  for ( int i = 10 ; i < 50 ; i++ )
	        outfile << " ";
		  outfile << token_packer.token_groups[b].objectcode;
		  //outfile <<  getpacker( token_packer.token_groups[b]) ; //寫入檔案 //TEST
		  outfile << endl;
		} // if
		  else
		    outfile << token_packer.token_groups[b].error << endl;
	} // for
    //Sicxe_Instruction_print( sicxe ); // test
	outfile.close();//寫檔完後關閉檔案 
} // sicxe

int main() {
	int command = 0; //第一個input(0, 1, 2)
	do { 
	  cout << endl << "** Assembler **";
	  cout << endl << "* 0. Quit     *";
	  cout << endl << "* 1. SIC      *"; 
	  cout << endl << "* 2. SICXE    *"; 
	  cout << endl << "***************"; 
	  cout << endl << "Input a command(0, 1, 2): "; 
	  cin >> command;  //cin第一個input(0, 1, 2)
	  switch(command){
	  	case 0 : break;
	  	case 1 : 
	      sic();
	      break;
	    case 2 : 
	      sicxe();  
	      break;
	    default : cout << endl << "Command does not exist!" << endl; //input 0,1,2以外的 
	  }
	} while ( command != 0 );
	system ( "pause" );
	return 0;
} //main


