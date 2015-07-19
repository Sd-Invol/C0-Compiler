#include "word.h"

#define SYMBOL_OUTPUT   0
#define QUAD_OUTPUT     1
#define ERROR_TEST      0
#define OPTIMIZATION    1

char c;
int len;
int sym;
int num;
int sign;
int type;
char ch;
char str[SIZE];
int row_cnt = 1; //行号, 1-based

FILE *mips;

int symbol_cnt;
int flag1; // deal with between varaible declear and function

bool CE;
void ERROR(int error_id) {
    cerr << "Compile Error , in row " << row_cnt << " , error_id = " << error_id << endl;
    cerr << "May be attributable to " << ERROR_msg[error_id] << '.' << endl;
    cerr << endl;
    CE = 1;
    if (!ERROR_TEST)
        exit(error_id);
    //else
    //    system("pause");
}
const int max_table_size = SIZE;
int Domain; //作用域 -1 for global
//符号表
map< pair<int , string>  , int> table_hash;
struct Table {
    string name;
    int domain;
    int object;
    // 0 const
    // 1 variable
    // 2 variable pointer
    // 3 function
    int type;
    // 0 void
    // 1 char
    // 2 int
    int adr; // address offset for variable/array;
    int val; // value for const / the number of arguments for function
    int size;// for array ; for stack size for function
}T[max_table_size];
int table_cnt;
vector<string> const_string;
vector< pair<string , int> > heap;
void declare(int o , int t) {
    int temp = Domain;
    if (o == 3)
        Domain = -1;
    if (table_hash.count(make_pair(Domain , str)))
        ERROR(4);
    if (table_hash.count(make_pair(-1 , str))) {
        int id = table_hash[make_pair(-1 , str)];
        if (T[id].object == 3)
            ERROR(4);
    }

    table_hash[make_pair(Domain , str)] = table_cnt;
    T[table_cnt].domain = Domain;
    T[table_cnt].object = o;
    T[table_cnt].name = str;
    T[table_cnt].type = t;
    if (o == 0) { // const
        if (t == 2) {
            T[table_cnt].val = num;
        }
        if (t == 1) {
            T[table_cnt].val = ch;
        }
    }
    if (o == 1) { // varaible
        if (~Domain) {
            T[table_cnt].adr = -T[Domain].size;
            T[Domain].size += 4;
        } else {
            heap.push_back(make_pair(str , 1));
            //fprintf(mips , "\t%s:\t\t.word\n" , str);
        }
    }
    if (o == 2) { // array
        if (~Domain) {
            T[table_cnt].size = num;
            T[table_cnt].adr = -T[Domain].size;
            T[Domain].size += num << 2;
        } else {
            heap.push_back(make_pair(str , num << 2));
            //fprintf(mips , "\t%s:\t\t.space\t\t%d\n" , str , num << 2);
        }
    }
    if (o == 3) { // function
        T[table_cnt].size = 0;
        T[table_cnt].val = 0;
        Domain = temp;
    }
    table_cnt ++;
    //table_pointer[Level] ++;
}
int getpos() {
    if (!table_hash.count(make_pair(Domain , str))) {
        if (!table_hash.count(make_pair(-1 , str)))
            return -1;
        return table_hash[make_pair(-1 , str)];
    }
    return table_hash[make_pair(Domain , str)];
}
void getsym() {
    sym = -1;
    while (c == ' ' || c == '\n' || c == '\t') {
        if (c == '\n')
            ++ row_cnt;
        c = getchar();
    }
    if (isalpha(c) || c == '_') {
        len = 0;
        do {
            str[len ++] = tolower(c);
            c = getchar();
        } while (len < SIZE && (isalnum(c) || c == '_'));
        str[len] = 0;
        key_word_it = lower_bound(key_word.begin() , key_word.end() , make_pair((string)str , -1));
        if (key_word_it != key_word.end() && key_word_it -> first == str) {
            //ahahhahahaha
            sym = key_word_it -> second;
            if (sym == CONSTTK)
                type = CONSTTK;
            if (sym == INTTK) {
                if (type == CONSTTK)
                    type = INTCON;
                else
                    type = sym;
            }
            if (sym == CHARTK) {
                if (type == CONSTTK)
                    type = CHARCON;
                else
                    type = sym;
            }
        } else {
            // new identity
            sym = IDEN;
            strcpy(_word[IDEN] , str);
        }
    } else if (c == '-' || c == '+') {
        // pos or neg
        //sign = c == '+' ? 1 : -1;
        if (c == '+') {
            sign = 1;
            sym = PLUS;
        } else {
            sign = -1;
            sym = MINU;
        }
        c = getchar();
    } else if (isdigit(c)) {
        // number
        num = 0;
        if (sign == 0)
            sign = 1;
        do {
            num = num * 10 + (c - '0');
            c = getchar();
        } while (isdigit(c));
        num *= sign , sign = 0;
        sym = type = INTCON;
        sprintf(_word[type] , "%d" , num);
    } else if (c == '\'') {
        // single quote
        c = getchar();
        if (c == '+' || c == '*' || isalnum(c) || c == '_'); else
            ERROR(7);
        sym = type = CHARCON;
        ch = c;
        _word[type][0] = c;
        _word[type][1] = 0;
        c = getchar();
        if (c != '\'')
            ERROR(5);
        c = getchar();
    } else if (c == '\"') {
        // double quote
        c = getchar();
        sym = type = STRCON;
        len = 0;
        while (c == 32 || c == 33 || (c >= 35 && c <= 126)) {
            str[len ++] = c;
            c = getchar();
        }
        str[len] = 0;
        strcpy(_word[STRCON] , str);
        if (c != '\"')
            ERROR(6);
        c = getchar();
    } else if (c == '=') {
        c = getchar();
        if (c == '=') {
            c = getchar();
            // equal
            sym = EQL;
        } else {
            // assign
            sym = ASSIGN;
        }
    } else if (c == '*') {
        c = getchar();
        sym = MULT;
        // multi
    } else if (c == '/') {
        c = getchar();
        sym = DIV;
        // div
    } else if (c == '!') {
        c = getchar();
        if (c == '=') {
            c = getchar();
            sym = NEQ;
            // not equal
        } else {
            ERROR(8);
        }
    } else if (c == '<') {
        c = getchar();
        if (c == '=') {
            c = getchar();
            sym = LEQ;
            // <=
        } else {
            sym = LSS;
            // <
        }
    } else if (c == '>') {
        c = getchar();
        if (c == '=') {
            c = getchar();
            sym = GEQ;
            // >=
        } else {
            sym = GRE;
            // >
        }
    } else if (c == '(') {
        //left parenthesis
        c = getchar();
        sym = LPARENT;
    } else if (c == '[') {
        //left bracket
        c = getchar();
        sym = LBRACK;
    } else if (c == '{') {
        //left brace
        c = getchar();
        sym = LBRACE;
    } else if (c == ')') {
        //right parenthesis
        c = getchar();
        sym = RPARENT;
    } else if (c == ']') {
        //right bracket
        c = getchar();
        sym = RBRACK;
    } else if (c == '}') {
        //right brace
        c = getchar();
        sym = RBRACE;
    } else if (c == ',') {
        //right brace
        c = getchar();
        sym = COMMA;
    } else if (c == ';') {
        //right brace
        c = getchar();
        sym = SEMICN;
    } else {
        if (!~c)
            return;
        ERROR(8);
    }
    //fprintf(out , "%d %s %s\n" , ++ symbol_cnt , _type[sym] , _word[sym]);
    if (SYMBOL_OUTPUT)
        printf("%d %s %s\n" , ++ symbol_cnt , _type[sym] , _word[sym]);
}
void constdeclaration() {
    while (sym == CONSTTK) {
        getsym();
        if (sym == INTTK || sym == CHARTK) {
            int intorchar = sym;
            do {
                getsym();
                if (sym != IDEN)
                    ERROR(9);
                // str
                getsym();
                if (sym != ASSIGN)
                    ERROR(2);
                getsym();
                if (sym == PLUS || sym == MINU)
                    getsym();
                if (intorchar == INTTK) {
                    if (sym != INTCON)
                        ERROR(10);
                    declare(0 , 2);
                } else {
                    if (sym != CHARCON)
                        ERROR(10);
                    declare(0 , 1);
                }
                getsym();
            } while (sym == COMMA);
        } else
            ERROR(10);
        if (sym != SEMICN)
            ERROR(1);
        getsym();
    }
}
void vardeclaration() {
    while (sym == INTTK || sym == CHARTK) {
        int intorchar = sym;
        do {
            getsym();
            if (sym != IDEN)
                ERROR(9);
            // str
            getsym();
            if (sym == LBRACK) {
                getsym();
                if (sym != INTCON)
                    ERROR(10);
                getsym();
                if (sym != RBRACK)
                    ERROR(10);
                getsym();
                if (intorchar == INTTK) {
                    declare(2 , 2);
                } else {
                    declare(2 , 1);
                }
            } else {
                if (intorchar == INTTK) {
                    declare(1 , 2);
                } else {
                    declare(1 , 1);
                }
            }
        } while (sym == COMMA);
        if (sym == LPARENT) {
            table_cnt --;
            //table_pointer[Level] --;
            table_hash.erase(make_pair(Domain , str));
            heap.pop_back();
            flag1 = intorchar;
            return;
        }
        if (sym != SEMICN)
            ERROR(1);
        getsym();
    }
}
const int max_quad_size = SIZE;
typedef pair<int , int> Pair;
bool cmp(const Pair& A , const Pair& B) {
    return make_pair(A.second , A.first) > make_pair(B.second , B.first);
}

int quad_cnt;
char _quadt[100];
void Pairprint(const Pair& R) {
    if (R.second == 1)
        printf("%-5d " , R.first);
    if (R.second == 2)
        printf("%-5s " , T[R.first].name.c_str());
    if (R.second == 3) {
        if (~R.first)
            sprintf(_quadt , "$t%d " , R.first);
        else
            sprintf(_quadt , "$v0 ");
        printf("%-5s " , _quadt);
    }
    if (R.second == 4) {
        sprintf(_quadt , "$L%d " , R.first);
        printf("%-5s " , _quadt);
    }
}
struct Quadruple {
    int type;
    Pair A , B , C;
    //first -> val (id for temp , table)
    //second -> type (meaningless 0) const->1 , table->2 , temp->3 , label -> 4
    Quadruple () {
        type = 0;
        A = B = C = make_pair(0 , 0);
    }
    Quadruple (int _type , Pair _A , Pair _B , Pair _C) {
        type = _type;
        A = _A , B = _B , C = _C;
    }
    void print() {
        if (type == 17) {
            printf("%-5s " , quad_type[type]);
            Pair R = A;
            if (~R.first)
                sprintf(_quadt , "$t%d " , R.first);
            else
                sprintf(_quadt , "$v0 ");
            printf("%-5s " , _quadt);
            puts("");
            return;
        }
        printf("%-5s " , quad_type[type]);
        Pairprint(A) , Pairprint(B) , Pairprint(C);
        puts("");
    }
}Quad[max_quad_size];
void newquad(int t , Pair A  = Pair(0 , 0) , Pair B = Pair(0 , 0) , Pair C = Pair(0 , 0)) {
    Quad[quad_cnt] = Quadruple(t , A , B , C);
    quad_cnt ++;
}
int label_cnt;
int temp_cnt;
int max_temp_cnt;

//四元式生成部分
/*
    0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15
    =   =:  +   -   *   /   <   <=  >   >=  ==  !=  lab jmp jz  :=
    16  17  18  19  20  21   22     23  24  25
    in  out put fun ret push pop    beg end =c=


    0. = A B
    A=B
    1. =: A B C
    A = B[C]
    2~11. A B C
    C = A # B
    12. lab label
    label:
    13. jmp label
    jmp label
    14. jz A label
    if (A == 0)
        jmp label
    15. := A B C
    A[B] = C
    16 in A
    scanf(A)
    17 out A
    printf(A)
    18 put A
    printf(__strA);
    19 fun A B ..//function
    call A / jr A
    return value will saved in $v0?
    20 ret A
    save A to $v0 and jump to end of function
    21 push A
    push A in stack
    22 pop A
    A = stack.top stack.pop
    23 begin A
    begin of function
    - save regs/ra
    - declare memory
    24 end A (contain jump)
    end of function
    - release memory
    - load regs/ra
    - jr $ra
    25. =c= A B
    A=B B is a char
*/
void expression();
void call(int id) {
    getsym();
    int cnt = 0;
    while (sym != RPARENT) {
        ++ cnt;
        //temp_init;
        expression();
        newquad(21 , Pair(temp_cnt - 1 , 3));
        //PUSH temp_cnt - 1 into stack
        if (sym == COMMA)
            getsym();
        else if (sym != RPARENT)
            ERROR(11);
    }
    getsym();
    if (cnt != T[id].val)
        ERROR(12);
    newquad(19 , Pair(id , 2));
}
void factor() { // 因子
    if (sym == IDEN) {
        // str
        int id = getpos();
        if (!~id)
            ERROR(3);
        getsym();
        // check id valid/invalid
        if (sym == LPARENT) {
            if (T[id].object != 3 || T[id].type == 0)
                ERROR(10);
            call(id);
            newquad(0 , Pair(temp_cnt , 3) , Pair(-1 , 3));
            temp_cnt ++;
            // remained to be done
        } else if (sym == LBRACK) {
            if (T[id].object != 2)
                ERROR(10);
            getsym();
            //as/sert(sym == INTCON);
            //int idx = num;
            //getsym();
            expression();
            if (sym != RBRACK)
                ERROR(13);
            getsym();
            newquad(1 , Pair(temp_cnt , 3) , Pair(id , 2) , Pair(temp_cnt - 1 , 3));
            //printf("=: t%d %s t%d\n" , temp_cnt , str , temp_cnt - 1);
            temp_cnt ++;
        } else {
            if (T[id].object >= 2)
                ERROR(10);
            if (T[id].object == 1) {
                //printf("= t%d %s\n" , temp_cnt , str);
                newquad(0 , Pair(temp_cnt , 3) , Pair(id , 2));
            } else if (T[id].object == 0) {
                //printf("= t%d %d\n" , temp_cnt , T[id].val);
                if (T[id].type == 2)
                    newquad(0 , Pair(temp_cnt , 3) , Pair(T[id].val , 1));
                else
                    newquad(25 , Pair(temp_cnt , 3) , Pair(T[id].val , 1));
            } else {
                ERROR(10);
            }
            temp_cnt ++;
        }
    } else {
        if (sym == PLUS || sym == MINU)
            getsym();
        if (sym == INTCON || sym == CHARCON) {
            if (sym == INTCON)
                newquad(0 , Pair(temp_cnt , 3) , Pair(num , 1));
                //printf("= t%d %d\n" , temp_cnt , num);
            else
                newquad(25 , Pair(temp_cnt , 3) , Pair(ch , 1));
                //printf("= t%d %d\n" , temp_cnt , ch);
            temp_cnt ++;
            //temp_pointer[Level] ++;
        } else if (sym == LPARENT) {
            getsym();
            expression();
//            printf("= t%d t%d\n" , temp_cnt , temp_cnt - 1);
//            temp_cnt ++;
//            //temp_pointer[Level] ++;
            if (sym != RPARENT)
                ERROR(11);
        } else {
            ERROR(10);
        }
        getsym();
    }

}
void term() { //项
    factor();
    while (sym == MULT || sym == DIV) {
        int tmp = sym;
        int pre = temp_cnt - 1;
        getsym();
        factor();
        if (tmp == MULT)
            newquad(4 , Pair(temp_cnt , 3) , Pair(pre , 3) , Pair(temp_cnt - 1 , 3));
        //    printf("* t%d t%d t%d\n" , temp_cnt , pre , temp_cnt - 1);
        else
            newquad(5 , Pair(temp_cnt , 3) , Pair(pre , 3) , Pair(temp_cnt - 1 , 3));
        //    printf("/ t%d t%d t%d\n" , temp_cnt , pre , temp_cnt - 1);
        temp_cnt ++;
        //temp_pointer[Level] ++;
    }
}
void expression() { //表达式
    int Negate = sym;
    if (sym == PLUS || sym == MINU)
        getsym();
    term();
    if (Negate == MINU) {
        newquad(3 , Pair(temp_cnt - 1 , 3) , Pair(0 , 1) , Pair(temp_cnt - 1 , 3));
        //printf("- t%d 0 t%d\n" , temp_cnt - 1 , temp_cnt - 1);
    }
    while (sym == PLUS || sym == MINU) {
        int tmp = sym;
        int pre = temp_cnt - 1;
        sign = 0;
        getsym();
        term();
        if (tmp == PLUS)
            newquad(2 , Pair(temp_cnt , 3) , Pair(pre , 3) , Pair(temp_cnt - 1 , 3));
        //    printf("+ t%d t%d t%d\n" , temp_cnt , pre , temp_cnt - 1);
        else
            newquad(3 , Pair(temp_cnt , 3) , Pair(pre , 3) , Pair(temp_cnt - 1 , 3));
        //    printf("- t%d t%d t%d\n" , temp_cnt , pre , temp_cnt - 1);
        temp_cnt ++;
        //temp_pointer[Level] ++;
    }
}
void condition() {
    expression();
    int tmp = temp_cnt - 1;
    //<,<=,>,>=,==,!=
    if (LSS <= sym && sym <= NEQ) { //<
        int oper = sym;
        getsym();
        expression();
        if (oper == LSS) newquad(6  , Pair(temp_cnt , 3) , Pair(tmp , 3) , Pair(temp_cnt - 1 , 3));//printf("< t%d t%d t%d\n"  , temp_cnt , tmp , temp_cnt - 1);
        if (oper == LEQ) newquad(7  , Pair(temp_cnt , 3) , Pair(tmp , 3) , Pair(temp_cnt - 1 , 3));//printf("<= t%d t%d t%d\n" , temp_cnt , tmp , temp_cnt - 1);
        if (oper == GRE) newquad(8  , Pair(temp_cnt , 3) , Pair(tmp , 3) , Pair(temp_cnt - 1 , 3));//printf("> t%d t%d t%d\n"  , temp_cnt , tmp , temp_cnt - 1);
        if (oper == GEQ) newquad(9  , Pair(temp_cnt , 3) , Pair(tmp , 3) , Pair(temp_cnt - 1 , 3));//printf(">= t%d t%d t%d\n" , temp_cnt , tmp , temp_cnt - 1);
        if (oper == EQL) newquad(10 , Pair(temp_cnt , 3) , Pair(tmp , 3) , Pair(temp_cnt - 1 , 3));//printf("== t%d t%d t%d\n" , temp_cnt , tmp , temp_cnt - 1);
        if (oper == NEQ) newquad(11 , Pair(temp_cnt , 3) , Pair(tmp , 3) , Pair(temp_cnt - 1 , 3));//printf("!= t%d t%d t%d\n" , temp_cnt , tmp , temp_cnt - 1);
        temp_cnt ++;
    }
}
/*
＜语句＞    ::= ＜条件语句＞｜＜循环语句＞｜‘{’＜语句列＞‘}’｜＜有返回值函数调用语句＞;
                            |＜无返回值函数调用语句＞;｜＜赋值语句＞;｜＜读语句＞;｜＜写语句＞;｜＜空＞;｜＜返回语句＞;
*/
void statements();
void statement() { // 语句
    if (sym == IFTK) {
        getsym();
        if (sym != LPARENT)
            ERROR(14);
        getsym();
        condition();
        newquad(14 , Pair(temp_cnt - 1 , 3) , Pair(label_cnt , 4));
        //printf("jz t%d label%d\n" , temp_cnt - 1 , label_cnt);
        int tmp = label_cnt ++;
        if (sym != RPARENT)
            ERROR(11);
        getsym();
        statement();
        if (sym == ELSETK) {
            newquad(13 , Pair(label_cnt , 4));
            //printf("jmp label%d\n" , label_cnt);
            newquad(12 , Pair(tmp , 4));
            //printf("lab label%d\n" , tmp);
            tmp = label_cnt ++;
            getsym();
            statement();
            newquad(12 , Pair(tmp , 4));
            //printf("lab label%d\n" , tmp);
        } else {
            newquad(12 , Pair(tmp , 4));
            //printf("lab label%d\n" , tmp);
        }
    } else if (sym == WHILETK) {
        //while ‘(’＜条件＞‘)’＜语句＞
        getsym();
        if (sym != LPARENT)
            ERROR(14);
        newquad(12 , Pair(label_cnt , 4));
        //printf("lab label%d\n" , label_cnt);
        int tmp1 = label_cnt ++;
        getsym();
        condition();
        int tmp2 = label_cnt ++;
        newquad(14 , Pair(temp_cnt - 1 , 3) , Pair(tmp2 , 4));
        //printf("jz t%d label%d\n" , temp_cnt - 1 , tmp2);
        if (sym != RPARENT)
            ERROR(11);
        getsym();
        statement();
        newquad(13 , Pair(tmp1 , 4));
        //printf("jmp label%d\n" , tmp1);
        newquad(12 , Pair(tmp2 , 4));
        //printf("lab label%d\n" , tmp2);
    } else if (sym == FORTK) {
        //for‘(’＜标识符＞＝＜表达式＞;＜条件＞;＜标识符＞＝＜标识符＞(+|-)＜步长＞‘)’＜语句＞
        getsym();
        if (sym != LPARENT)
            ERROR(14);
        getsym();
        if (sym != IDEN)
            ERROR(9);
        int id = getpos();
        if (!~id)
            ERROR(3);
        if (T[id].object != 1)
            ERROR(10);
        getsym();
        if (sym != ASSIGN)
            ERROR(2);
        getsym();
        expression();
        newquad(0 , Pair(id , 2) , Pair(temp_cnt - 1 , 3));
        //printf("= %s t%d\n" , T[id].name.c_str() , temp_cnt - 1);
        if (sym != SEMICN)
            ERROR(1);
        getsym();
        newquad(12 , Pair(label_cnt , 4));
        //printf("lab label%d\n" , label_cnt);
        int tmp1 = label_cnt ++;
        condition();
        int tmp2 = label_cnt ++;
        newquad(14 , Pair(temp_cnt - 1 , 3) , Pair(tmp2 , 4));
        //printf("jz t%d label%d\n" , temp_cnt - 1 , tmp2);
        if (sym != SEMICN)
            ERROR(1);
        getsym();
        if (sym != IDEN)
            ERROR(9);
        int id1 = getpos();
        if (!~id1)
            ERROR(3);
        getsym();
        if (sym != ASSIGN)
            ERROR(2);
        getsym();
        if (sym != IDEN)
            ERROR(9);
        int id2 = getpos();
        if (!~id2)
            ERROR(3);
        getsym();
        if (sym == MINU || sym == PLUS); else
            ERROR(10);
        getsym();
        if (sym != INTCON)
            ERROR(10);
        //a/ssert(num != 0);
        int step = num;
        getsym();
        if (sym != RPARENT)
            ERROR(11);
        getsym();
        statement();
        newquad(2 , Pair(id1 , 2) , Pair(id2 , 2) , Pair(step , 1));
        //printf("+ %s %d %s\n" , T[id].name.c_str() , step , T[id].name.c_str());
        newquad(13 , Pair(tmp1 , 4));
        //printf("jmp label%d\n" , tmp1);
        newquad(12 , Pair(tmp2 , 4));
        //printf("lab label%d\n" , tmp2);
    } else if (sym == SCANFTK) {
        getsym();
        if (sym != LPARENT)
            ERROR(14);
        getsym();
        if (sym != IDEN)
            ERROR(9);
        int id = getpos();
        if (!~id)
            ERROR(3);
        if (T[id].object != 1)
            ERROR(10);
        newquad(16 , Pair(id , 2));
        getsym();
        while (sym == COMMA) {
            getsym();
            if (sym != IDEN)
                ERROR(9);
            int id = getpos();
            if(!~id)
                ERROR(3);
            if (T[id].object != 1)
                ERROR(10);
            newquad(16 , Pair(id , 2));
            getsym();
        }
        if (sym != RPARENT)
            ERROR(11);
        getsym();
        if (sym != SEMICN)
            ERROR(1);
        getsym();
    } else if (sym == PRINTFTK) {
        getsym();
        if (sym != LPARENT)
            ERROR(14);
        getsym();

        if (sym == STRCON) {
            getsym();
            newquad(18 , Pair(const_string.size() , 1));
            const_string.push_back(str);
            if (sym == COMMA) {
                getsym();
                expression();
                newquad(17 , Pair(temp_cnt - 1 , 3));
            }
        } else { // should have check if is expression
            expression();// but if it isn't, will assert fail.
            Quadruple& last = Quad[quad_cnt - 1];
            bool intorchar = 0; // int
            if (last.A == Pair(temp_cnt - 1 , 3)) {
                if (last.type == 25)
                    intorchar = 1;
                if (last.B.second == 2 && T[last.B.first].type == 1)
                    intorchar = 1;
                if (last.B == Pair(-1 , 3)) {
                    Quadruple& fun = Quad[quad_cnt - 2];
                    if (fun.type == 19 && T[fun.A.first].type == 1)
                        intorchar = 1;
                }
            }
            if (intorchar)
                newquad(17 , Pair(temp_cnt - 1 , 1));
            else
                newquad(17 , Pair(temp_cnt - 1 , 3));
        }
        if (sym != RPARENT)
            ERROR(11);
        getsym();
        if (sym != SEMICN)
            ERROR(1);
        getsym();

    } else if (sym == LBRACE) {
        getsym();
        statements();
        if (sym != RBRACE)
            ERROR(15);
        getsym();
    } else if (sym == RETURNTK) {
        getsym();
        if (sym == LPARENT) {
            if (T[Domain].type == 0)
                ERROR(18);
            getsym();
            expression();
            if (sym != RPARENT)
                ERROR(11);
            getsym();
            newquad(20 , Pair(temp_cnt - 1 , 3));
        } else {
            if (T[Domain].type != 0)
                ERROR(18);
            newquad(20 , Pair(0 , 1));
        }
        if (sym != SEMICN)
            ERROR(1);
        getsym();
    } else if (sym == IDEN) {
        /*
        ＜赋值语句＞;
        ＜有返回值函数调用语句＞;
        ＜无返回值函数调用语句＞;
        */
        //find the type of 'str'
        int id = getpos();
        if (!~id)
            ERROR(3);
        getsym();
        if (T[id].object == 1) { // int or char , variable
            if (sym != ASSIGN)
                ERROR(2);
            getsym();
            expression();
            newquad(0 , Pair(id , 2) , Pair(temp_cnt - 1 , 3));
            //printf("= %s t%d\n" , T[id].name.c_str() , temp_cnt - 1);
        } else if (T[id].object == 2) { // array
            if (sym != LBRACK)
                ERROR(16);
            getsym();
            expression();
            if (sym != RBRACK)
                ERROR(13);
            getsym();
            int tmp = temp_cnt - 1;
            if (sym != ASSIGN)
                ERROR(2);
            getsym();
            expression();
            newquad(15 , Pair(id , 2) , Pair(tmp , 3) , Pair(temp_cnt - 1 , 3));
            //printf(":= %s t%d t%d\n" , T[id].name.c_str() , tmp , temp_cnt - 1);
        } else if (T[id].object == 3) { // function
            if (sym != LPARENT)
                ERROR(14);
            call(id);
        } else {
            ERROR(20);
        }
        if (sym != SEMICN)
            ERROR(1);
        getsym();
    } else if (sym == SEMICN) {
        getsym();
    } else {
        ERROR(10);
    }

}
void statements() { //语句列
    while (sym == IFTK || sym == WHILETK || sym == SEMICN ||
           sym == FORTK || sym == SCANFTK || sym == PRINTFTK ||
           sym == LBRACE || sym == RETURNTK || sym == IDEN)
        statement();
}
void compound_statement() { // 复合语句
    //栈指针、语句指针保存
    if (sym == CONSTTK) { //常量
        constdeclaration();
    }
    if (sym == INTTK || sym == CHARTK) { //变量
        vardeclaration();
    }
    //语句列
    statements();
}
void _main_ () { //程序
    Domain = -1;
    c = getchar();
    getsym();
    fprintf(mips , ".data\n");
    if (sym == CONSTTK) {
        constdeclaration();
    }
    if (sym == INTTK || sym == CHARTK) {
        vardeclaration();
    }
    string name;
    while (sym == INTTK || sym == CHARTK || sym == VOIDTK || flag1) {
        int _type = flag1 ? flag1 : sym;
        if (!flag1) {
            getsym();
            if (sym == IDEN || sym == MAINTK); else
                ERROR(9);
            getsym();
        } else {
            flag1 = 0;
        }
        name = str;
        if (name == "main" && _type != VOIDTK)
            ERROR(10);
        if (_type == INTTK)
            declare(3 , 2);
        if (_type == CHARTK)
            declare(3 , 1);
        if (_type == VOIDTK)
            declare(3 , 0);
        int id = table_cnt - 1;
        newquad(12 , Pair(id , 2));
        Domain = id;
        if (sym != LPARENT)
            ERROR(14);
        getsym();
        vector<int> args;
        while (sym == CHARTK || sym == INTTK) {
            //arguments
            int type = sym;
            getsym();
            if (sym != IDEN)
                ERROR(9);
            if (type == CHARTK)
                declare(1 , 1);
            else
                declare(1 , 2);
            args.push_back(table_cnt - 1);
            getsym();
            ++ T[id].val;
            if (sym == COMMA)
                getsym();
            else if (sym == RPARENT)
                break;
            else
                ERROR(10);
        }

        newquad(23 , Pair(Domain , 2));
        reverse(args.begin() , args.end());
        if (sym != RPARENT)
            ERROR(11);
        getsym();
        if (sym != LBRACE)
            ERROR(17);

        getsym();
        compound_statement();
        if (sym != RBRACE)
            ERROR(15);

        getsym();

        newquad(24 , Pair(Domain , 2));// end of function

        if (name == "main") {
            if (T[id].val != 0 || sym != -1)
                ERROR(10);
            break;
        }
    }
    if (sym != -1 || name != "main")
        ERROR(19);
    for (int i = 0 ; i < (int)heap.size() ; ++ i) {
        if (heap[i].second == 1) {
            fprintf(mips , "\t%s:\t\t.space\t\t4\n" , heap[i].first.c_str());
        } else {
            fprintf(mips , "\t%s:\t\t.space\t\t%d\n" , heap[i].first.c_str() , heap[i].second);
        }
    }
    for (int i = 0 ; i < (int)const_string.size() ; ++ i) {
        fprintf(mips , "\t_S%d:\t.asciiz\"%s\"\n" , i , const_string[i].c_str());
    }

    fprintf(mips , ".text\n");
    fprintf(mips , "\tb\tmain\n");
}
void printtemp(const Pair& A) {
    if (A.first == -1)
        fprintf(mips , "$v0\t");
    else
        fprintf(mips , "$%d\t" , 5 + A.first);
}
void Push(int id) {
    fprintf(mips , "\tsw\t$%d\t0($sp)\n" , id);
    fprintf(mips , "\tsubi\t$sp\t$sp\t4\n");
}
void Pop(int id) {
    fprintf(mips , "\taddi\t$sp\t$sp\t4\n");
    fprintf(mips , "\tlw\t$%d\t0($sp)\n" , id);
}
void mips_gen(Quadruple *Q , int qcnt) {
    Domain = -1;
    for (int i = 0 ; i < qcnt ; ++ i) {
        int com = Q[i].type;
        Pair &A = Q[i].A;
        Pair &B = Q[i].B;
        Pair &C = Q[i].C;
        if (com == 0) { // =
            if (A.second == 3 && B.second == 3) {
                fprintf(mips , "\tmove\t");
                printtemp(A);
                printtemp(B);
            } else if (A.second == 3 && B.second == 2) {
                fprintf(mips , "\tlw\t");
                printtemp(A);
                int id = B.first;
                if (~T[id].domain)
                    fprintf(mips , "%d($fp)\t" , T[id].adr);
                else
                    fprintf(mips , "%s" , T[id].name.c_str());
            } else if (A.second == 3 && B.second == 1) {
                fprintf(mips , "\tli\t");
                printtemp(A);
                fprintf(mips , "%d\t" , B.first);
            } else if (A.second == 2 && B.second == 3) {
                fprintf(mips , "\tsw\t");
                printtemp(B);
                int id = A.first;
                if (~T[id].domain)
                    fprintf(mips , "%d($fp)\t" , T[id].adr);
                else
                    fprintf(mips , "%s" , T[id].name.c_str());
            } else {
                //a/ssert(0);
            }
            fprintf(mips , "\n");
        }
        if (com == 1) { // A = B[C] 323 ****可能有问题
            int id = B.first;
            fprintf(mips , "\tsll\t");
            printtemp(Pair(-1 , 3));
            printtemp(C);
            fprintf(mips , "2\n");
            if (~T[id].domain) {
                fprintf(mips , "\tsub\t$v1\t$fp\t");
                printtemp(Pair(-1 , 3));
                fprintf(mips , "\n");
                fprintf(mips , "\tlw\t");
                printtemp(A);
                fprintf(mips , "%d($v1)\n" , T[id].adr);
            } else {
                fprintf(mips , "\tlw\t");
                printtemp(A);
                fprintf(mips , "%s($v0)\n" , T[id].name.c_str());
            }
        }
        if (com == 2) { //+
            if (A.second == 3) {
                fprintf(mips , "\tadd\t");
                printtemp(A);
                printtemp(B);
                printtemp(C);
                fprintf(mips , "\n");
            } else {
                int x = B.first;
                fprintf(mips , "\tlw\t$v1\t%d($fp)\n" , T[x].adr);
                fprintf(mips , "\taddi\t$v1\t$v1\t%d\n" , C.first);
                fprintf(mips , "\tsw\t$v1\t%d($fp)\n" , T[x].adr);
            }
        }
        if (com == 3) { //-
            if (B.second == 3) {
                fprintf(mips , "\tsub\t");
                printtemp(A);
                printtemp(B);
                printtemp(C);
                fprintf(mips , "\n");
            } else {
                fprintf(mips , "\tsub\t");
                printtemp(A);
                fprintf(mips , "$0\t");
                printtemp(A);
                fprintf(mips , "\n");
            }
        }
        if (4 <= com && com <= 11) { //  *   /   <   <=  >   >=  ==  !=
            if (com == 4)  fprintf(mips , "\tmul\t");
            if (com == 5)  fprintf(mips , "\tdiv\t");
            if (com == 6)  fprintf(mips , "\tslt\t");
            if (com == 7)  fprintf(mips , "\tsle\t");
            if (com == 8)  fprintf(mips , "\tsgt\t");
            if (com == 9)  fprintf(mips , "\tsge\t");
            if (com == 10) fprintf(mips , "\tseq\t");
            if (com == 11) fprintf(mips , "\tsne\t");
            printtemp(A);
            printtemp(B);
            printtemp(C);
            fprintf(mips , "\n");
        }
        if (com == 12) { //label
            if (A.second == 4) {
                fprintf(mips , "$L%d:\n" , A.first);
            } else {
                fprintf(mips , "%s:\n" , T[A.first].name.c_str());
                Domain = A.first;
            }
        }
        if (com == 13) { //jmp
            fprintf(mips , "\tb\t$L%d\n" , A.first);
        }
        if (com == 14) { //jz
            fprintf(mips , "\tbeqz\t");
            printtemp(A);
            fprintf(mips , "$L%d\n" , B.first);
        }
        if (com == 15) { // A[B] = C 233 ****可能有问题
            int id = A.first;
            fprintf(mips , "\tsll\t");
            printtemp(Pair(-1 , 3));
            printtemp(B);
            fprintf(mips , "2\n");
            if (~T[id].domain) {
                fprintf(mips , "\tsub\t$v1\t$fp\t");
                printtemp(Pair(-1 , 3));
                fprintf(mips , "\n");
                fprintf(mips , "\tsw\t");
                printtemp(C);
                fprintf(mips , "%d($v1)\n" , T[id].adr);
            } else {
                fprintf(mips , "\tsw\t");
                printtemp(C);
                fprintf(mips , "%s($v0)\n" , T[id].name.c_str());
            }
        }
        if (com == 16) { // scanf
            fprintf(mips , "\tli\t$v0\t");
            int x = A.first;
            if (T[x].type == 1)
                fprintf(mips , "12\n");
            else
                fprintf(mips , "5\n");
            fprintf(mips , "\tsyscall\n");
            fprintf(mips , "\tsw\t$v0\t");
            if (~T[x].domain)
                fprintf(mips , "%d($fp)\n" , T[x].adr);
            else
                fprintf(mips , "%s\n" , T[x].name.c_str());
        }
        if (com == 17) { // printf
            if (A.second == 3)  {
                fprintf(mips , "\tli\t$v0\t1\n");
            } else {
                fprintf(mips , "\tli\t$v0\t11\n");
            }
            fprintf(mips , "\tmove\t$a0\t$%d\n" , A.first + 5);
            fprintf(mips , "\tsyscall\n");
        }
        if (com == 18) { // put
            fprintf(mips , "\tla\t$a0\t_S%d\n" , A.first);
            fprintf(mips , "\tli\t$v0\t4\n");
            fprintf(mips , "\tsyscall\n");
        }
        if (com == 19) { // call
            int x = A.first;
            fprintf(mips , "\tjal\t%s\n" , T[x].name.c_str());
        }
        if (com == 20) { // return
            if (A.second == 3) {
                fprintf(mips , "\tmove\t$v0\t");
                printtemp(A);
                fprintf(mips , "\n");
            }
            fprintf(mips , "\tb\t%s_end\n" , T[Domain].name.c_str());
        }
        if (com == 21) { //push
            fprintf(mips , "\tsw\t");
            printtemp(A);
            fprintf(mips , "0($sp)\n");
            fprintf(mips , "\tsubi\t$sp\t$sp\t4\n");
        }
        if (com == 22) { //pop
            fprintf(mips , "\taddi\t$sp\t$sp\t4\n");
            fprintf(mips , "\tlw\t$v1\t0($sp)\n");
            fprintf(mips , "\tsw\t$v1\t%d($fp)\n" , T[A.first].adr);
        }
        if (com == 23) { //begin
            fprintf(mips , "\tmove\t$v1\t$fp\n");
            if (!T[Domain].val && !T[Domain].size) {
                fprintf(mips , "\tmove\t$fp\t$sp\n");
            } else {
                fprintf(mips , "\taddi\t$fp\t$sp\t%d\n" , T[Domain].val << 2);
                fprintf(mips , "\tsubi\t$sp\t$fp\t%d\n" , T[Domain].size);
            }
            if (T[Domain].name != "main") {
                Push(3) , Push(31);
                for (int i = 0 ; i < max_temp_cnt ; ++ i)
                    Push(5 + i);
            }
        }
        if (com == 24) { //end
            fprintf(mips , "%s_end:\n" , T[Domain].name.c_str());
            if (T[Domain].name != "main") {
                for (int i = max_temp_cnt - 1 ; i >= 0 ; -- i)
                    Pop(5 + i);
                Pop(31) , Pop(30);
                fprintf(mips , "\taddi\t$sp\t$sp\t%d\n" , T[Domain].size);
                fprintf(mips , "\tjr\t$ra\n");
                fprintf(mips , "\n");
            }
        }
        if (com == 25) { // =c=
            fprintf(mips , "\tli\t");
            printtemp(A);
            fprintf(mips , "%d\n" , B.first);
        }

    }
}

Quadruple OPT[SIZE];
int opt_cnt;
inline void newopt(int t , Pair A  = Pair(0 , 0) , Pair B = Pair(0 , 0) , Pair C = Pair(0 , 0)) {
    OPT[opt_cnt ++] = Quadruple(t , A , B , C);
}
int ncnt , deg[SIZE];
map< Pair , int > Version;
vector< Pair > Vec[SIZE];
struct Node {
    Pair val;
    vector<int> Fa;
    int type; // same as quad
    int L , R;
}D[SIZE];

inline int find_Node(Pair A) {
    if (!Version.count(A)) {
        Version[A] = ++ ncnt;
        Vec[ncnt].clear();
        D[ncnt].val = A;
        D[ncnt].type = -1;
        D[ncnt].Fa.clear();
        D[ncnt].L = D[ncnt].R = 0;
    }
    return Version[A];
}

bool newblock[SIZE];
void DAG(int L , int R) {
    //int tmp = opt_cnt;
    newblock[opt_cnt] = 1;
    ncnt = 0;
    Version.clear();
    memset(deg , 0 , sizeof(deg));
    bool jump = (Quad[R - 1].type != 15 && Quad[R - 1].type != 25 && Quad[R - 1].type > 11);
    if (jump)
        -- R;
    for (int i = L ; i < R ; ++ i) {
        int com = Quad[i].type;
        if (com != 0 && com != 25) {
            // A = B op C
            Pair A = Quad[i].A;
            int B = find_Node(Quad[i].B);
            int C = find_Node(Quad[i].C);
            int j;
            for (j = 1 ; j <= ncnt ; ++ j)
                if (com != 15 && D[j].type == com && D[j].L == B && D[j].R == C)
                    break;
            if (j > ncnt) {
                Version[A] = ++ ncnt;
                Vec[ncnt].clear();
                Vec[ncnt].push_back(A);
                D[ncnt].Fa.clear();
                D[ncnt].L = B , D[ncnt].R = C;
                D[ncnt].type = com;
                deg[ncnt] += 2;
                D[B].Fa.push_back(ncnt);
                D[C].Fa.push_back(ncnt);
            } else {
                Version[A] = j;
                Vec[j].push_back(A);
            }
        } else if (com == 0 || com == 25) {
            Pair A = Quad[i].A;
            int B = find_Node(Quad[i].B);
            Vec[B].push_back(A);
            Version[A] = B;
        } else {
            assert(0);
        }
    }
    //printf("$%d\n" , Version.size());
    //for (int i = 1 ; i <= ncnt ; ++ i)
    //    printf("%d\n" , Vec[i].size());
    //cout << ncnt << endl;
    for (int i = 1 ; i <= ncnt ; ++ i) {
        sort(Vec[i].begin() , Vec[i].end() , cmp);
        Vec[i].erase(unique(Vec[i].begin() , Vec[i].end()) , Vec[i].end());

//        printf("%2d|%2d : " , i , D[i].type);
//        for (int j = 0 ; j < (int) Vec[i].size() ; ++ j)
//            printf("%d %d " , Vec[i][j].first , Vec[i][j].second);
//        puts("");

        if (!~D[i].type) {
            //cout << "*" << i << ' ' << Vec[i].size() << endl;
            if (Vec[i].empty())
                continue;
            assert(Vec[i][0].second == 3);
            newopt(0 , Vec[i][0] , D[i].val);
            for (int j = 1 ; j < (int)Vec[i].size() ; ++ j)
                if (Version[Vec[i][j]] == i)
                    newopt(0 , Vec[i][j] , Vec[i][0]);
        } else {
            int L = D[i].L , R = D[i].R;
            if (D[i].type == 1) {
                assert(Vec[i][0].second == 3);
                assert(Vec[R][0].second == 3);
                if (Vec[L].empty())
                    newopt(D[i].type , Vec[i][0] , D[L].val , Vec[R][0]);
                else {
                    assert(Vec[L][0].second == 2);
                    newopt(D[i].type , Vec[i][0] , Vec[L][0] , Vec[R][0]);
                }
                for (int j = 1 ; j < (int)Vec[i].size() ; ++ j)
                    if (Version[Vec[i][j]] == i)
                        newopt(0 , Vec[i][j] , Vec[i][0]);
            } else if (D[i].type == 15) {
                assert(Vec[i].size() == 1);
                assert(Vec[L][0].second == 3);
                assert(Vec[R][0].second == 3);
                newopt(D[i].type , Vec[i][0] , Vec[L][0] , Vec[R][0]);
            } else {
                //assert(Vec[i][0].second == 3);
                //printf("* %d %d\n" , L , R);
                Pair AA , BB;
                AA = Vec[L].empty() ? D[L].val : Vec[L][0];
                BB = Vec[R].empty() ? D[R].val : Vec[R][0];

                newopt(D[i].type , Vec[i][0] , AA , BB);
                for (int j = 1 ; j < (int)Vec[i].size() ; ++ j)
                    if (Version[Vec[i][j]] == i)
                        newopt(0 , Vec[i][j] , Vec[i][0]);
            }
        }
    }
    if (jump)
        OPT[opt_cnt ++] = Quad[R];
}

bool is_block[SIZE];
void block_divided() {
    is_block[0] = 1;
    for (int i = 0 ; i < quad_cnt ; ++ i) {
        if (19 <= Quad[i].type && Quad[i].type <= 24 && Quad[i].type != 21)
            is_block[i] = is_block[i + 1] = 1;
        if (12 <= Quad[i].type && Quad[i].type <= 13)
            is_block[i] = is_block[i + 1] = 1;
        if (16 == Quad[i].type || Quad[i].type == 18)
            is_block[i] = is_block[i + 1] = 1;
        if (Quad[i].type == 14 || Quad[i].type == 17 || Quad[i].type == 21)
            is_block[i + 1] = 1;
        if (2 == Quad[i].type && Quad[i].A.second != 3)
            is_block[i] = is_block[i + 1] = 1;
    }
}
void local_common_expression() {
    int bcnt = 0;
    for (int i = 0 ; i < quad_cnt ; ++ i)
        if (is_block[i]) {
            ++ bcnt;
            int j = i + 1;
            while (j < quad_cnt && !is_block[j])
                ++ j;
            DAG(i , j);
            i = j - 1;
        }
}

vector<int> release[SIZE];
map<int , int> allo;
bool used[21];
int allocate() {
    for (int i = 0 ; i < 21 ; ++ i)
        if (!used[i]) {
            used[i] = 1;
            max_temp_cnt = max(max_temp_cnt , i + 1);
            return i;
        }
    assert(0);
    return -1;
}
void register_allocation(Quadruple *Q , int qcnt) {
    map<int , int> End;
    for (int i = 0 ; i < qcnt ; ++ i) {
        if (Q[i].type == 17 || (Q[i].A.second == 3 && ~Q[i].A.first))
            End[Q[i].A.first] = i;
        if (Q[i].B.second == 3 && ~Q[i].B.first)
            End[Q[i].B.first] = i;
        if (Q[i].C.second == 3 && ~Q[i].C.first)
            End[Q[i].C.first] = i;
    }
    for (map<int , int>::iterator it = End.begin() ; it != End.end() ; ++ it)
        release[it -> second].push_back(it -> first);

    for (int i = 0 ; i < qcnt ; ++ i) {
        if (Q[i].type == 17 || (Q[i].A.second == 3 && ~Q[i].A.first)) {
            if (!allo.count(Q[i].A.first))
                allo[Q[i].A.first] = allocate();
            Q[i].A.first = allo[Q[i].A.first];
        }
        if (Q[i].B.second == 3 && ~Q[i].B.first) {
            if (!allo.count(Q[i].B.first))
                allo[Q[i].B.first] = allocate();
            Q[i].B.first = allo[Q[i].B.first];
        }
        if (Q[i].C.second == 3 && ~Q[i].C.first) {
            if (!allo.count(Q[i].C.first))
                allo[Q[i].C.first] = allocate();
            Q[i].C.first = allo[Q[i].C.first];
        }
        for (int j = 0 ; j < (int)release[i].size() ; ++ j)
            used[allo[release[i][j]]] = 0;
    }
}
char file_name[SIZE] = "1.txt";

int main()
{
    freopen(file_name , "r" , stdin);
    mips = fopen("test.asm" , "w");
    init();
    //work();
    _main_();

    if (!CE) {
        block_divided();
        if (OPTIMIZATION) {
            local_common_expression();
            register_allocation(OPT , opt_cnt);
            if (QUAD_OUTPUT) {
                for (int i = 0 ; i < opt_cnt ; ++ i) {
                    if (newblock[i])
                        puts("*****************************");
                    printf("%3d: " , i);
                    OPT[i].print();
                }
                puts("*****************************");
            }
            mips_gen(OPT , opt_cnt);
        } else {
            register_allocation(Quad , quad_cnt);
            if (QUAD_OUTPUT) {
                for (int i = 0 ; i < quad_cnt ; ++ i) {
                    if (is_block[i])
                        puts("*****************************");
                    printf("%3d: " , i);
                    Quad[i].print();
                }
                puts("*****************************");
            }
            mips_gen(Quad , quad_cnt);
        }
        cerr << "Compile successful!" << endl;
        cerr << "Max temporary registers: " << max_temp_cnt << endl;
    }
    fclose(mips);
    return 0;
}
