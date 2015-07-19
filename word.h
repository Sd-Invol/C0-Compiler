#include <iostream>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <vector>
#include <string>
#include <cctype>
#include <cassert>
#include <set>
#include <map>
using namespace std;

const int SIZE = 4096;

#define IDEN 0
#define INTCON 1
#define CHARCON 2
#define FLOATCON 3
#define CONSTTK 4
#define INTTK 5
#define CHARTK 6
#define VOIDTK 7
#define MAINTK 8
#define IFTK 9
#define ELSETK 10
#define WHILETK 11
#define FORTK 12
#define SCANFTK 13
#define PRINTFTK 14
#define RETURNTK 15
#define PLUS 16
#define MINU 17
#define MULT 18
#define DIV 19
#define LSS 20
#define LEQ 21
#define GRE 22
#define GEQ 23
#define EQL 24
#define NEQ 25
#define ASSIGN 26
#define SEMICN 27
#define COMMA 28
#define STRCON 29
#define COLON 30
#define QMARK 31
#define DQMARK 32
#define LPARENT 33
#define RPARENT 34
#define LBRACK 35
#define RBRACK 36
#define LBRACE 37
#define RBRACE 38

const char _type[40][10] = {
"IDEN","INTCON","CHARCON","FLOATCON","CONSTTK","INTTK","CHARTK",
"VOIDTK","MAINT","IFTK","ELSETK","WHILETK","FORTK","SCANFTK",
"PRINTFTK","RETURNTK","PLUS","MINU","MULT","DIV","LSS","LEQ",
"GRE","GEQ","EQL","NEQ","ASSIGN","SEMICN","COMMA","STRCON",
"COLON","QMARK","DQMARK","LPARENT","RPARENT","LBRACK","RBRACK",
"LBRACE","BRACE"
};
char _word[40][40] = {
"IDEN","INTCON","CHARCON","FLOATCON","const","int","char",
"void","main","if","else","while","for","scanf",
"printf","return","+","-","*","/","<","<=",
">",">=","==","!=","=",";",",","STRCON",
":","'","\"","(",")","[","]",
"{","}"
};

string ERROR_msg[] = {
    "" ,
    "missing ';'",
    "missing '='" ,
    "undefined ident" ,
    "duplicated declaration" ,
    "missing '\''" , // 5
    "missing '\"" ,
    "char out of range" ,
    "lexical error" ,
    "missing ident" ,
    "syntax error" , // 10
    "missing ')'" ,
    "arguments error" ,
    "missing ']'" ,
    "missing '('" ,
    "missing '}'" , // 15
    "missing '['" ,
    "missing '{'" ,
    "return type error" ,
    "missing main" ,
    "const assign" // 20
};

vector< pair<string, int> > key_word;
vector< pair<string, int> >::iterator key_word_it;
void init() {
    key_word.push_back(make_pair("main" , MAINTK));
    key_word.push_back(make_pair("if" , IFTK));
    key_word.push_back(make_pair("else" , ELSETK));
    key_word.push_back(make_pair("while" , WHILETK));
    key_word.push_back(make_pair("for" , FORTK));
    key_word.push_back(make_pair("scanf" , SCANFTK));
    key_word.push_back(make_pair("printf" , PRINTFTK));
    key_word.push_back(make_pair("return" , RETURNTK));
    key_word.push_back(make_pair("const" , CONSTTK));
    key_word.push_back(make_pair("int" , INTTK));
    key_word.push_back(make_pair("char" , CHARTK));
    key_word.push_back(make_pair("void" , VOIDTK));
    sort(key_word.begin() , key_word.end());
}
const char quad_type[][10] = {
"=","=:","+","-","*","/","<","<=",">",">=","==","!=",
"lab","jmp","jz",":=","in","out","put","fun","ret","push","pop","begin","end","=c="
};
