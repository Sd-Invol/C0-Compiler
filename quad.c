lab   aa
begin aa
=c=   $t0   97
ret   $t0
end   aa
lab   main
begin main
=c=   $t0   97
=     aaa   $t0
=c=   $t0   97
=     aaaa  $t0

=c=   $t0   97
out   $t0

=     $t0   97
out   $t0

=     $t0   aaaa
out   $t0

=     $t0   aaa
out   $t0

fun   aa
=     $t0   $v0
out   $t0

end   main

/*
    0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15
    =   =:  +   -   *   /   <   <=  >   >=  ==  !=  lab jmp jz  :=
    16  17  18  19  20  21   (22)   23  24  25
    in  out put fun ret push pop    beg end =c=


    0. = A B
    A=B
    1. =: A B C
    A = B[C]
    2~11. A B C
    A = B # C
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

const int max_quad_size = 1000;
typedef pair<int , int> Pair;
char _quadt[100];
const char quad_type[][10] = {
"=","=:","+","-","*","/","<","<=",">",">=","==","!=",
"lab","jmp","jz",":=","in","out","put","fun","ret","push","pop","begin","end"
};

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
        printf("%-5s " , quad_type[type]);
        Pairprint(A) , Pairprint(B) , Pairprint(C);
        puts("");
    }
}Quad[max_quad_size];
int quad_cnt;

void newquad(int t , Pair A  = Pair(0 , 0) , Pair B = Pair(0 , 0) , Pair C = Pair(0 , 0)) {
    Quad[quad_cnt] = Quadruple(t , A , B , C);
    if (QUAD_OUTPUT)
        Quad[quad_cnt].print();
    quad_cnt ++;
}
