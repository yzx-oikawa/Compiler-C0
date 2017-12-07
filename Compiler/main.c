#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define KEYNUM 14
#define MAXIDENTLEN 20
#define MAXSYMTABLEN 500

const char *keyword[]={"const","int","char","void","main","if","else","while","switch","case",
                       "default","scanf","printf","return"};
const char *mnemonic[]={"CONST","INT","CHAR","VOID","MAIN","IF","ELSE","WHILE","SWITCH","CASE",

                        "DEFAULT","SCANF","PRINTF","RETURN"};

const enum symbol {constsym, intsym, charsym, voidsym, mainsym, ifsym, elsesym, whilesym,//0-7
                   switchsym, casesym, defaultsym, scanfsym, printfsym, returnsym,       //8-13
                   add, sub, mult, divi, les, loe, mor, moe, noteq, equal,               //14-23
                   comma, colon, semicolon, sinquo, douquo, equmark,                     //24-29
                   lparent, rparent, lbracket, rbracket, lbrace, rbrace,                 //30-35
                   identsym, inttype, chartype, numtype, strtype, blank, underline};     //36-42

/*符号表是一个结构
种类  	名字	类型			值			地址	 长度		所在函数开始的位置
常量	名字	1-int/0-char	常量的值	地址	 0
变量	名字	1-int/0-char	0					 0
数组	名字	1-int/0-char	0					 数组长度
函数	名字	1-int/0-char	1-有返回值 0-无		 参数个数
参数	名字	1-int/0-char	0					 0
*/
struct symbletable{
     char name[MAXIDENTLEN];  //名字
     int kind;       //种类 0-常量 1-变量 2-函数 3-参数
     int type;       //类型 1-int 0-char
     int value;      //值
     int address;    //在符号表中的偏移量
     int length;     //长度
     int location;   //所在函数开始的位置
};
struct symbletable symtab[MAXSYMTABLEN];
int tabpc=0;//符号表指针
int tabi;   //用来遍历符号表的参数

FILE *IN, *OUT, *INTER;
int No=1;

int blankflag=0; //0-跳过空格 1-不跳过空格
int vardecbegflag=0;//1-变量声明开始
int vardecendflag=1;//1-变量声明结束
int braceflag=0;
int checkiflogflag;

char ch; //字符
char token[100]; //当前字符串
int sym;     //当前标识符
int symnum;  //当前int标识符的数值
int symtype; //当前标识符类型
int symaddr; //当前标识符地址
int symloca; //当前标识符所在函数位置，遇到函数就加一
int sympara; //当前函数标识符参数个数

int nextsym();
int constdec();//常量说明
int constdef();//常量定义
int vardec();//常量说明
int vardef(); //变量定义
int differ();
int retfuncdef(); //有返回值函数定义
int voidfuncdef(); //无返回值函数定义
int parameters(); //参数
int mainfunc(); //主函数
int expression(); //表达式
int item(); //项
int factor(); //因子
int statements(); //复合语句
int statement();//语句
int assignstatement(); //赋值语句
int ifstatement(); //条件语句
int condition();//条件
int whilestatement(); //循环语句
int switchstatement(); //情况语句
int casestatement(); //情况子语句
int defaultstatement(); //缺省
int retfuncuse(); //有返回值函数调用语句
int voidfuncuse(); //无返回值函数调用语句
int valuepara(); //值参数表
int readstatement(); //读语句
int writestatement(); //写语句
int returnstatement(); //返回语句
int constant();//常量
int constforcase();
int strings();//字符串
int checkiflog();//检查是否登录符号表
int checkfunclog();//检查函数声明是否登录符号表

int checkiflog()//检查是否登录符号表
{
    //全局常量、全局变量、函数声明
    if(tabpc==0){
        for(tabi=tabpc;tabi>=0;tabi--){
            if (strcmp(token,symtab[tabi].name)==0)//在表中找到了这个标识符
                return 1;
        }
    }
    //main常量、main变量、函数常量、函数变量声明、函数参数
    else{
        for(tabi=tabpc;tabi>=0;tabi--){
            if(symtab[tabi].location==0||symtab[tabi].location==symloca){
                if (strcmp(token,symtab[tabi].name)==0)//在表中找到了这个标识符
                    return 1;
            }
        }
    }
    return 0;
}

/*＜常量说明＞ ::=  const＜常量定义＞;{ const＜常量定义＞;}*/
int constdec()
{
    printf("Constdec begin:\n");
    fprintf(OUT,"Constdec begin:\n");
    while(1)
    {
        constdef();
        if(sym==semicolon)//分号
        {
            sym=nextsym();
            if(sym==constsym)//下一个常量说明
                continue;
            else
            {
                printf("Constdec end\n");
                fprintf(OUT,"Constdec end\n");
                return;
            }
        }
        else return;
    }
}

/*＜常量定义＞   ::=   int＜标识符＞＝＜整数＞{,＜标识符＞＝＜整数＞}
                    | char＜标识符＞＝＜字符＞{,＜标识符＞＝＜字符＞}*/
int constdef()
{
    printf("\tConstdef\n");
    fprintf(OUT,"\tConstdef\n");
    sym=nextsym();
    if(sym==intsym){
        while(1)
        {
            sym=nextsym();
            if(sym==identsym||sym==chartype)//标识符
            {
                //登录符号表
                if(checkiflog()==0){
                    checkiflogflag=0;
                    strcpy(symtab[++tabpc].name,token);
                    symtab[tabpc].kind=0;//常量
                    symtab[tabpc].type=1;//int
                    symtab[tabpc].location=symloca;//函数位置
                }
                else{
                    checkiflogflag=1;
                    printf("%s DUPLICATE DEFINE\n",token);
                }
                sym=nextsym();
                if(sym==equmark)//等号
                {
                    sym=nextsym();
                    if(sym==add||sym==sub)
                        sym=nextsym();
                    if(sym==inttype||sym==numtype)//整数
                    {
                        if(checkiflogflag==0)
                            symtab[tabpc].value=symnum;//值
                        //printf("%s\n",token);
                        sym=nextsym();
                        if(sym==comma)//逗号
                            continue;
                        else//分号
                            return;
                    }
                }
            }
        }
    }
    else if(sym==charsym){
        while(1)
        {
            sym=nextsym();
            if(sym==identsym||sym==chartype)//标识符
            {
                //登录符号表
                if(checkiflog()==0){
                    checkiflogflag=0;
                    strcpy(symtab[++tabpc].name,token);
                    symtab[tabpc].kind=0;//常量
                    symtab[tabpc].type=0;//char
                    symtab[tabpc].location=symloca;//函数位置
                }
                else{
                    checkiflogflag=1;
                    printf("%s DUPLICATE DEFINE\n",token);
                }
                sym=nextsym();
                if(sym==equmark)//等号
                {
                    sym=nextsym();
                    if(sym==sinquo)//单引号
                    {
                        sym=nextsym();
                        if(sym==chartype||sym==numtype||
                           (sym>=add&&sym<=divi)||sym==underline)//字符
                        {
                            if(checkiflogflag==0)
                                symtab[tabpc].value=token[0];//值
                            sym=nextsym();
                            if(sym==sinquo)//单引号
                            {
                                sym=nextsym();
                                if(sym==comma)//逗号
                                    continue;
                                else//分号
                                    return;
                            }
                        }
                        else printf("constdeferror\n");
                    }
                }
            }
        }
    }
}

/*＜变量说明＞  ::= ＜变量定义＞;{＜变量定义＞;}*/
int vardec()
{
    if(vardecbegflag==0){
        printf("Vardec begin:\n");
        fprintf(OUT,"Vardec begin:\n");
        vardecbegflag=1;
        vardecendflag=0;
    }
    while(1)
    {
        //sym是左方括号或者逗号或者分号
        vardef();
        if(sym==semicolon)//分号
        {
            sym=nextsym();
            if(sym==intsym||sym==charsym){//下一个变量说明或者函数定义
                symtype=(sym==intsym?1:0);
                return;
            }
            else{
                if(vardecendflag==0){
                    printf("Vardec end\n");
                    fprintf(OUT,"Vardec end\n");
                    vardecendflag=1;
                }
                return;
            }
        }
        else return;
    }
}

/*＜变量定义＞  ::= ＜类型标识符＞(＜标识符＞|＜标识符＞‘[’＜无符号整数＞‘]’){,(＜标识符＞|＜标识符＞‘[’＜无符号整数＞‘]’ )}  */
int vardef()
{
    printf("\tVardef\n");
    fprintf(OUT,"\tVardef\n");
    while(1)
    {
        //sym是左方括号或者逗号或者分号
        if(sym==semicolon)
            return;
        sym=nextsym();
        if(sym==identsym||sym==chartype)//标识符
        {
            //登录符号表-名字、类型、种类
            if(checkiflog()==0){
                checkiflogflag=0;
                strcpy(symtab[++tabpc].name,token);
                symtab[tabpc].kind=1;//变量
                symtab[tabpc].type=symtype;
                symtab[tabpc].location=symloca;//函数位置
            }
            else{
                checkiflogflag=1;
                printf("%s DUPLICATE DEFINE\n",token);
            }
            sym=nextsym();
            if(sym==lbracket)//左方括号，是数组
            {
                sym=nextsym();
                if(sym==inttype||sym==numtype)//无符号整数
                {
                    //登录符号表-长度
                    if(checkiflogflag==0)
                        symtab[tabpc].length=symnum;
                    sym=nextsym();
                    if(sym==rbracket)//右方括号
                    {
                        sym=nextsym();
                        if(sym==comma)//逗号
                            continue;
                        else//分号
                            return;
                    }
                }
            }
            else if(sym==comma)//逗号
                continue;
            else
                return;
        }
        else if(sym==inttype||sym==numtype)//无符号整数
        {
            //登录符号表-长度
            if(checkiflogflag==0)
                symtab[tabpc].length=symnum;
            sym=nextsym();
            if(sym==rbracket)//右方括号
            {
                sym=nextsym();
                if(sym==comma)//逗号
                    continue;
                else//分号
                    return;
            }
        }
    }
}

/*区分变量声明和有返回值函数定义*/
int differ()
{
    //读到了类型标识符
    while(1){
        sym=nextsym();
        if(sym==identsym||sym==chartype)//标识符
        {
            if(checkiflog()==0){
                checkiflogflag=0;
                strcpy(symtab[++tabpc].name,token);
                symtab[tabpc].type=symtype;
                symtab[tabpc].location=symloca;//函数位置
            }
            else{
                checkiflogflag=1;
                printf("%s DUPLICATE DEFINE\n",token);
                }
            sym=nextsym();
            if(sym==lbracket||sym==comma||sym==semicolon)//左方括号或逗号或分号
            {
                if(checkiflogflag==0)
                    symtab[tabpc].kind=1;//变量
                vardec();
            }
            else{
                if(vardecendflag==0){
                    printf("Vardec end\n");
                    fprintf(OUT,"Vardec end\n");
                    vardecendflag=1;
                }
                if (sym==lbrace||sym==lparent){//左花括号或左括号
                    if(checkiflogflag==0){
                        symtab[tabpc].kind=2;//函数
                        symtab[tabpc].value=1;//有返回值
                        symloca=tabpc+1;
                        symtab[tabpc].location=symloca;
                    }
                    retfuncdef();
                }
            }
        }
        else if(sym==intsym||sym==charsym){
            symtype=(sym==intsym?1:0);
            continue;
        }
        if(sym==voidsym||sym==rbrace||sym==ifsym||sym==whilesym
           ||sym==lbrace||sym==scanfsym||sym==printfsym||sym==switchsym
           ||sym==returnsym||sym==identsym||sym==chartype)
            return;
    }
}

/*＜有返回值函数定义＞  ::=  ＜声明头部＞‘(’＜参数＞‘)’ ‘{’＜复合语句＞‘}’
                            |＜声明头部＞‘{’＜复合语句＞‘}’ */
int retfuncdef()
{
    while(1){
        printf("Retfuncdef begin:\n");
        fprintf(OUT,"Retfuncdef begin:\n");

        if(sym==lbrace)//左花括号
        {
            sym=nextsym();
            statements();//复合语句
            if(sym==rbrace)//右花括号
            {
                printf("Retfundef end:\n");
                fprintf(OUT,"Retfundef end:\n");
                return;
            }
        }
        else if(sym==lparent)//左括号
        {
            parameters();//参数
            if(checkiflogflag==0)
                symtab[tabpc-sympara-1].length=sympara+1;
            if(sym==rparent)//右括号
            {
                sym=nextsym();
                if(sym==lbrace)//左花括号
                {
                    sym=nextsym();
                    statements();//复合语句
                    if(sym==rbrace)//右花括号
                    {
                        printf("Retfuncdef end:\n");
                        fprintf(OUT,"Retfuncdef end:\n");
                        return;
                    }
                }
            }
        }
    }
}

/*＜无返回值函数定义＞  ::= void＜标识符＞(’＜参数＞‘)’‘{’＜复合语句＞‘}’
                          | void＜标识符＞{’＜复合语句＞‘}’*/
int voidfuncdef()
{
    //sym是标识符
    printf("Voidfuncdef begin:\n");
    fprintf(OUT,"Voidfuncdef begin:\n");
    symloca=tabpc+1;
    while(1)
    {
        if(sym==identsym||sym==chartype)//标识符
        {
            if(checkiflog()==0){
                checkiflogflag=0;
                strcpy(symtab[++tabpc].name,token);
                symtab[tabpc].kind=2;//函数
                symtab[tabpc].type=0;
                symtab[tabpc].value=0;//无返回值
                symtab[tabpc].location=symloca;//函数位置
            }
            else{
                checkiflogflag=1;
                printf("%s DUPLICATE DEFINE\n",token);
            }
            sym=nextsym();
            if(sym==lparent)//左括号
            {
                parameters();//参数
                if(checkiflogflag==0)
                    symtab[tabpc-sympara-1].length=sympara+1;
                if(sym==rparent)//右括号
                {
                    sym=nextsym();
                    if(sym==lbrace)//左花括号
                    {
                        sym=nextsym();
                        statements();//语句
                        if(sym==rbrace)//右花括号
                        {
                            printf("Voidfuncdef end\n");
                            fprintf(OUT,"Voidfuncdef end\n");
                            return;
                        }
                    }
                }
            }
            else if(sym==lbrace)//左花括号
            {
                sym=nextsym();
                statements();//复合语句
                if(sym==rbrace)//右花括号
                {
                    printf("Voidfuncdef end\n");
                    fprintf(OUT,"Voidfuncdef end\n");
                    return;
                }
            }
        }
    }
}

/*＜参数表＞  ::=  ＜类型标识符＞＜标识符＞{,＜类型标识符＞＜标识符＞}*/
int parameters()//参数表
{
    printf("\tParameters\n");
    fprintf(OUT,"\tParameters\n");
    sympara=0;
    //sym此时为左括号
    while(sym!=rparent)
    {
        sym=nextsym();
        if(sym==intsym||sym==charsym)//类型标识符
        {
            symtype=(sym==intsym?1:0);
            sym=nextsym();
            printf("%d\n",sym);
            if(sym==identsym||sym==chartype){//标识符
                if(checkiflog()==0){
                    checkiflogflag=0;
                    strcpy(symtab[++tabpc].name,token);
                    symtab[tabpc].kind=3;//参数
                    symtab[tabpc].type=symtype;
                    symtab[tabpc].location=symloca;//函数位置
                }
                else{
                    checkiflogflag=1;
                    printf("%s DUPLICATE DEFINE\n",token);
                }
                sym=nextsym();
                if(sym==comma)//逗号，下一个参数
                {
                    if(checkiflogflag==0)
                        sympara++;
                    continue;
                }
                else if(sym==rparent) return;
            }
        }
    }
    return;//返回时sym为右括号
}

/*＜主函数＞  ::= void main‘(’‘)’‘{’＜复合语句＞‘}’ */
int mainfunc()
{
    //sym此时为main
    printf("Mainfunc begin:\n");
    fprintf(OUT,"Mainfunc begin:\n");
    symloca=tabpc+1;
    sym=nextsym();
    if(sym==lparent)//左括号
    {
        sym=nextsym();
        if(sym==rparent)//右括号
        {
            sym=nextsym();
            if(sym==lbrace)//左花括号
            {
                sym=nextsym();
                statements();//复合语句
                if(sym==rbrace)//右花括号
                {
                    printf("Mainfunc end\n");
                    fprintf(OUT,"Mainfunc end\n");
                    return;
                }
            }
        }
    }
}

/*＜复合语句＞   ::=  ［＜常量说明＞］［＜变量说明＞］｛＜语句＞｝*/
int statements()
{
    printf("Statements begin:\n");
    fprintf(OUT,"Statements begin:\n");
    vardecbegflag=0;
    vardecendflag=1;
    //常量声明
    if(sym==constsym){
        constdec();
    }
    //变量声明
    if(sym==intsym||sym==charsym){//类型标识符
        symtype=(sym==intsym?1:0);
        differ();
    }
    //语句
    statement();
    printf("Statements end\n");
    fprintf(OUT,"Statements end\n");
    if(sym==rbrace) return;
    else printf("mainerror\n");
    return;
}

/*＜语句＞ ::= ＜条件语句＞｜＜循环语句＞| ‘{’｛＜语句＞｝‘}’
            ｜＜有返回值函数调用语句＞; |＜无返回值函数调用语句＞;
            ｜＜赋值语句＞;｜＜读语句＞;｜＜写语句＞;｜＜空＞;
            |＜情况语句＞｜＜返回语句＞*/
int statement()
{
    while(!feof(IN)){
        switch(sym)
        {
            case rbrace:;
                return;
            case semicolon:
                sym=nextsym();
                if(sym==rbrace)//右花括号
                    return;
                else continue;
            case ifsym:
                ifstatement();
                continue;
            case elsesym:
                return;
            case whilesym:
                whilestatement();
                continue;
            case switchsym:
                switchstatement();
                continue;
            case casesym:
            case defaultsym:
                return;
            case scanfsym:
                readstatement();
                continue;
            case printfsym:
                writestatement();
                continue;
            case returnsym:
                returnstatement();
                continue;
            case lbrace:
                braceflag++;
                sym=nextsym();
                statement();
                if(sym==rbrace){
                    if(braceflag>0){
                        printf("statement end\n");
                        fprintf(OUT,"statement end\n");
                        braceflag--;
                    }
                    sym=nextsym();
                    continue;
                }
                else printf("errorlbrace\n");break;
            case identsym:
            case chartype:
                if(checkiflog()==0)//不在符号表中
                    printf("%s UNDEFINED IDENT\n",token);
                sym=nextsym();
                if(sym==lparent||sym==semicolon)//左括号或分号，为函数调用语句
                {
                    voidfuncuse();//返回了右括号
                    sym=nextsym();
                    if(sym==semicolon)//括号
                        continue;
                }
                else if(sym==equmark||sym==lbracket)//等号或左方括号，为赋值语句
                {
                    assignstatement();
                    continue;
                }
                else
                    printf("statementerror\n");
                    continue;
            default:
                continue;
        }
    }

}

/*＜赋值语句＞ ::= ＜标识符＞＝＜表达式＞|＜标识符＞‘[’＜表达式＞‘]’=＜表达式＞*/
int assignstatement() //赋值语句
{
    printf("Assign statement\n");
    fprintf(OUT,"Assign statement\n");
    //sym此时为等号或左方括号
    if(sym==lbracket)//左方括号，数组赋值
    {
		sym=nextsym();
        expression();//表达式
        {
            if(sym==rbracket)//右方括号
            {
                sym=nextsym();
                if(sym==equmark)//等号
                {
					sym=nextsym();
                    expression();//表达式
                    return;
                }
            }
        }
    }
    else if(sym==equmark)//等号
    {
		sym=nextsym();
        expression();//表达式
        return;
    }
    return;
}

/*＜条件语句＞::= if ‘(’＜条件＞‘)’＜语句＞else＜语句＞*/
int ifstatement() //情况语句
{
    printf("If statement begin:\n");
    fprintf(OUT,"If statement begin:\n");
    //sym此时为if
    sym=nextsym();
    if(sym==lparent)//左括号
    {
        condition();//条件
        sym=nextsym();
        if(sym==rparent)//右括号
        {
            sym=nextsym();
            statement();//语句
            if(sym==rbrace)
                sym=nextsym();
            if(sym==elsesym)//else
            {
                printf("Else statement:\n");
                fprintf(OUT,"Else statement:\n");
                sym=nextsym();
                statement();//语句
                if(sym==rbrace)
                    return;
                else printf("iferror\n");
            }
        }
    }
}

/*＜条件＞ ::=  ＜表达式＞＜关系运算符＞＜表达式＞｜＜表达式＞*/
int condition()
{
    printf("\tcondition\n");
    fprintf(OUT,"\tcondition\n");
    //此时sym是左括号
    sym=nextsym();
    expression();//表达式
    //printf("%s\n",token);
    if(sym>=les&&sym<=equal)//关系运算符
    {
        sym=nextsym();
        expression();
    }
    return;
}

/*＜循环语句＞ ::= while ‘(’＜条件＞‘)’＜语句＞*/
int whilestatement() //循环语句
{
    printf("While statement begin:\n");
    fprintf(OUT,"While statement begin:\n");
    //sym此时为while
    sym=nextsym();
    if(sym==lparent)//左括号
    {
        condition();//条件
        sym=nextsym();
        if(sym==rparent)//右括号
        {
            sym=nextsym();
            statement();//语句
            if(sym==rbrace)
                return;
            else printf("whileerror\n");
        }
    }
}

/*＜情况语句＞ ::= switch ‘(’＜表达式＞‘)’ ‘{’＜情况表＞[＜缺省＞] ‘}’*/
int switchstatement() //情况语句
{
    printf("Switch statement:\n");
    fprintf(OUT,"Switch statement:\n");
    //sym此时为switch
    sym=nextsym();
    {
        if(sym==lparent)//左括号
        {
			sym=nextsym();
            expression();
            if(sym==rparent)//右括号
            {
                sym=nextsym();
                if(sym==lbrace)//左花括号
                {
                    sym=nextsym();
                    if(sym==casesym){//case
                        casestatement();
                        if(sym==rbrace)//右花括号
                        {
                            sym=nextsym();
                            return;
                        }
                        else
                            printf("switcherror\n");
                    }
                }
            }
        }
    }
}

/*＜情况子语句＞  ::=  case＜常量＞：＜语句＞*/
int casestatement() //情况子语句
{
    //sym此时为case
    while(1)
    {
        sym=nextsym();
        if(sym==sinquo||sym==inttype||sym==numtype)//单引号括起来的字母或数字，或者整数
        {
            if(constforcase()==1){//常量
                printf("Case statement\n");
                fprintf(OUT,"Case statement\n");
            }
            sym=nextsym();
            if(sym==colon)//冒号
            {
                sym=nextsym();
                statement();//语句 可能返回右花括号、case、default
                if(sym==casesym)//下一个case
                    continue;
                else if(sym==defaultsym){//default
                    defaultstatement();
                    return;
                }
                else if (sym==rbrace)//右花括号
                    return;
            }
        }
    }
}

/*＜缺省＞ ::=  default : ＜语句＞*/
int defaultstatement() //缺省
{
    printf("Default statement\n");
    fprintf(OUT,"Default statement\n");
    //sym此时为default
    sym=nextsym();
    {
        if(sym==colon)//冒号
        {
            sym=nextsym();
            statement();//语句
        }
    }
}

/*＜有返回值函数调用语句＞ ::= ＜标识符＞‘(’＜值参数表＞‘)’|<标识符>*/
int retfuncuse() //有返回值函数调用语句
{
    printf("Return funcuse\n");
    fprintf(OUT,"Return funcuse\n");
    //sym此时为左括号或分号
    if(sym==semicolon)
        return;
    else if(sym==lparent)
    {
        sym=nextsym();
        valuepara();//值参数表
        if(sym==rparent)//右括号
            return;
    }
    printf("retfuncuseerror\n");
}

/*＜无返回值函数调用语句＞ ::= ＜标识符＞‘(’＜值参数表＞‘)’|<标识符>*/
int voidfuncuse() //无返回值函数调用语句
{
    printf("Void funcuse\n");
    fprintf(OUT,"Void funcuse\n");
    if(sym==semicolon)
        return;
    else if(sym==lparent)
    {
        sym=nextsym();
        valuepara();//值参数表
        if(sym==rparent)//右括号
            return;
    }
    printf("voidfuncuseerror\n");
}

/*＜值参数表＞   ::= ＜表达式＞{,＜表达式＞}*/
int valuepara() //值参数表
{
    printf("\tValue Parameters\n");
    fprintf(OUT,"\tValue Parameters\n");
    while(sym!=rparent)
    {
        expression();
        if(sym==comma)
        {
             sym=nextsym();
             continue;
        }
    }
    return;//返回时sym为右括号
}

/*＜读语句＞ ::=  scanf ‘(’＜标识符＞{,＜标识符＞}‘)’*/
int readstatement() //读语句
{
    printf("Read statement\n");
    fprintf(OUT,"Read statement\n");
    //sym此时为scanf
    sym=nextsym();
    if(sym==lparent)//左括号
    {
        while(1)
        {
            sym=nextsym();
            if(sym==identsym||sym==chartype)//标识符
            {
                if(checkiflog()==0)//不在符号表中
                    printf("%s UNDEFINED IDENT\n",token);
                sym=nextsym();
                if(sym==comma)//下一个标识符
                    continue;
                else{//右括号
                    sym=nextsym();//分号
                    return;
                }
            }
        }
    }
}

/*＜写语句＞ ::= printf ‘(’ ＜字符串＞,＜表达式＞ ‘)’
               | printf ‘(’＜字符串＞ ‘)’| printf ‘(’＜表达式＞‘)’*/
int writestatement() //写语句
{
    printf("Write statement\n");
    fprintf(OUT,"Write statement\n");
    //sym此时为printf
    sym=nextsym();
    if(sym==lparent)//左括号
    {
        sym=nextsym();
        if(sym==douquo)//双引号，字符串
        {
            strings();
            sym=nextsym();
            if(sym==comma)//逗号，字符串后有表达式
            {
				sym=nextsym();
                expression();
                if(sym==rparent)//右括号
                {
                    sym=nextsym();//分号
                    return;
                }
            }
            else if(sym==rparent)//右括号
            {
                sym=nextsym();//分号
                return;
            }
        }
        else//表达式
        {
            expression();
            if(sym==rparent)//右括号
            {
                sym=nextsym();//分号
                return;
            }
        }
    }
}

/*＜返回语句＞ ::=  return[‘(’＜表达式＞‘)’] */
int returnstatement() //返回语句
{
    printf("Return statement\n");
    fprintf(OUT,"Return statement\n");
    sym=nextsym();
    //sym此时为return
    if(sym==lparent)//左括号
    {
		sym=nextsym();
        expression();
        {
            if(sym==rparent){//右括号
                sym=nextsym();
                return;
            }
        }
    }
    else//分号
        return;
}

/*＜常量＞ ::=  ＜整数＞|＜字符＞
  ＜字符＞ ::='＜加法运算符＞'｜'＜乘法运算符＞'｜'＜字母＞'｜'＜数字＞'*/
int constant()
{
    //sym此时为单引号或者整数
    if(sym==sinquo){//单引号
        sym=nextsym();
        if((sym>=add&&sym<=divi)||sym==chartype||sym==underline||sym==numtype)
        {
            sym=nextsym();
            if(sym==sinquo)//单引号
                return 1;
            else{
                printf("constanterror\n");
                return 0;
            }
        }
        else{
            sym=nextsym();//单引号
            return 0;
        }
    }
    else if(sym==inttype||sym==numtype)
        return 1;
    else if(sym==add||sym==sub)//加号或减号
    {
        sym=nextsym();
        if(sym==inttype||sym==numtype)
            return 1;
    }
    return 0;
}

/*情况语句中，switch后面的表达式和case后面的常量只允许出现int和char类型*/
int constforcase()
{
    //sym此时为单引号或者整数
    if(sym==sinquo){//单引号
        sym=nextsym();
        if(sym==chartype||sym==numtype)
        {
            sym=nextsym();
            if(sym==sinquo)//单引号
                return 1;
            else{
                printf("constanterror\n");
                return 0;
            }
        }
        else{
            sym=nextsym();//单引号
            return 0;
        }
    }
    else if(sym==inttype||sym==numtype)
        return 1;
    else if(sym==add||sym==sub)//加号或减号
    {
        sym=nextsym();
        if(sym==inttype||sym==numtype)
            return 1;
    }
    return 0;
}

/*＜字符串＞ ::=  "｛十进制编码为32,33,35-126的ASCII字符｝"*/
int strings()
{
    printf("\tString\n");
    fprintf(OUT,"\tString\n");
    //sym此时为双引号
    sym=nextsym();
    while(sym!=douquo)
    {
        if(sym>=constsym&&sym<=underline)
            sym=nextsym();
        else
            return;
    }
    return;//返回了双引号
}

/*＜表达式＞ ::= ［+｜-］＜项＞{＜加法运算符＞＜项＞}*/
int expression()//表达式
{
    printf("\tExpression begin\n");
    fprintf(OUT,"\tExpression begin\n");
    if(sym==add||sym==sub)
        sym=nextsym();
    while(1)
    {
        item();
        if(sym==add||sym==sub){
            sym=nextsym();
            continue;
        }
        else {
            printf("\tExpression end\n");
            fprintf(OUT,"\tExpression end\n");
            return;
        }
    }
}

/*＜项＞ ::= ＜因子＞{＜乘法运算符＞＜因子＞}*/
int item() //项
{
    printf("\tItem\n");
    fprintf(OUT,"\tItem\n");
    while(1)
    {
        factor();
        if(sym==mult||sym==divi){
            sym=nextsym();
            continue;
        }
        else return;
    }
}

/*＜因子＞::= ＜标识符＞｜＜标识符＞‘[’＜表达式＞‘]’|
‘(’＜表达式＞‘)’｜＜整数＞|＜字符＞｜＜有返回值函数调用语句＞ */
int factor() //因子
{
    printf("\tFactor\n");
    fprintf(OUT,"\tFactor\n");
    switch(sym)
    {
        printf("%s\n",token);
        case identsym:
        case chartype:
            if(checkiflog()==0)//不在符号表中
                printf("%s UNDEFINED IDENT\n",token);
            sym=nextsym();
            if(sym==lbracket){//左方括号，数组
                sym=nextsym();
                expression();
                if(sym==rbracket){//右方括号
                    sym=nextsym();
                    return;
                }
            }
            else if(sym==lparent)//左括号或分号，有返回值函数调用
            {
                retfuncuse();//返回右括号
                sym=nextsym();
                return;
            }
            else//标识符
                return;
            break;
        case lparent://表达式
            sym=nextsym();
            expression();
            if(sym==rparent){//右括号
                sym=nextsym();
                return;
            }
            break;
        case add:
        case sub://整数前正负号
            sym=nextsym();
            if(sym==inttype||sym==numtype){
                sym=nextsym();
                return;
            }
            break;
        case inttype:
        case numtype://整数
            sym=nextsym();
            return;
        case sinquo://字符
            sym=nextsym();
            if((sym>=add&&sym<=divi)||sym==chartype||sym==numtype){
                sym=nextsym();
                if(sym==sinquo){//单引号
                    sym=nextsym();
                    return;
                }
            }
            break;
    }
    printf("factor error\n");
}

//读下一个字符
int nextsym()
{
    int t=0, ti;
    memset(token, 0, sizeof(token)); //清空数组！
    ch=fgetc(IN);
    if(feof(IN)) return -1;
    if(ch=='0')//如果是零，直接返回
        return numtype;
    if(isdigit(ch))//如果是非零数字
    {
        token[t]=ch;
        ch=fgetc(IN);
        while(isdigit(ch))
        {
            token[++t]=ch;
            ch=fgetc(IN);
        }
        if(!feof(IN)) //否则文件最后一个字符为数字时会进入无限循环
            fseek(IN,-1L,SEEK_CUR);
        if(t==0){
            symnum=token[t]-'0';
            return numtype;}
        else{
            symnum=0;
            for(ti=0;ti<=t;ti++)
            {
                symnum=symnum*10+(token[ti]-'0');
            }
            return inttype;
        }
    }
    else if (isalpha(ch)) //如果是字母
    {
        token[t]=tolower(ch); //统一存成小写
        ch=fgetc(IN);
        while(isdigit(ch)|isalpha(ch))
        {
            token[++t]=tolower(ch);
            ch=fgetc(IN);
            continue;
        }
        if(!feof(IN)) //否则文件最后一个字符为字母时会进入无限循环
            fseek(IN,-1L,SEEK_CUR);
        //判断是否为保留字
        if (strcmp(token,keyword[0])==0)
            return constsym;
        else if (strcmp(token,keyword[1])==0)
            return intsym;
        else if (strcmp(token,keyword[2])==0)
            return charsym;
        else if (strcmp(token,keyword[3])==0)
            return voidsym;
        else if (strcmp(token,keyword[4])==0)
            return mainsym;
        else if (strcmp(token,keyword[5])==0)
            return ifsym;
        else if (strcmp(token,keyword[6])==0)
            return elsesym;
        else if (strcmp(token,keyword[7])==0)
            return whilesym;
        else if (strcmp(token,keyword[8])==0)
            return switchsym;
        else if (strcmp(token,keyword[9])==0)
            return casesym;
        else if (strcmp(token,keyword[10])==0)
            return defaultsym;
        else if (strcmp(token,keyword[11])==0)
            return scanfsym;
        else if (strcmp(token,keyword[12])==0)
            return printfsym;
        else if (strcmp(token,keyword[13])==0)
            return returnsym;
        else //是标识符
        {
            if(t==0) return chartype;
            else return identsym;
        }
    }
    else if ((ch=='=')|(ch=='<')|(ch=='>')|(ch=='!')) //如果是比较运算符
    {
        token[t]=ch;
        ch=fgetc(IN);
        if(ch=='=')
        {
            token[++t]=ch;
            switch(token[0])
            {
                case '=':
                    return equal;
                case '<':
                    return loe;
                case '>':
                    return moe;
                case '!':
                    return noteq;
            }
        }
        else
        {
            fseek(IN,-1L,SEEK_CUR); //退一个字符
            switch(token[0])
            {
                case '=':
                    return equmark;
                case '<':
                    return les;
                case '>':
                    return mor;
                case '!':
                    return strtype;
            }
        }
    }
    else //是其他字符
    {
        switch(ch)
        {
            //跳过空白字符
            case '\n':
                return nextsym();
            case ' ':
            case '\t':
                if (blankflag==0) return nextsym();
                else return blank;
            case '+':
                return add;
            case '-':
                return sub;
            case '*':
                return mult;
            case '/':
                return divi;
            case ',':
                return comma;
            case ':':
                return colon;
            case ';':
                return semicolon;
            case '\'':
                return sinquo;
            case '\"':
                return douquo;
            case '(':
                //fprintf(OUT,"%d\tLPARENT\t%c\n",No++,ch); break;
                return lparent;
            case ')':
                return rparent;
            case '[':
                return lbracket;
            case ']':
                return rbracket;
            case '{':
                return lbrace;
            case '}':
                return rbrace;
            case '_':
                return underline;
            case '#':
            case '$':
            case '.':
            case '?':
            case '@':
            case '\\':
            case '^':
            case '`':
            case '|':
            case '~':
                return strtype;
               // return blank;
        }
    }
    return -1;
}

int main()
{
    IN = fopen("15061091_test.txt","r");
    OUT = fopen("15061091_result.txt","w");
    INTER = fopen("C:\\Users\\Administrator\\Desktop\\inter.txt","w");
    /*C:\\Users\\Administrator\\Desktop\\*/
    if(IN == NULL){
        printf("NO SUCH FILE!\n");
    }
    else{
        printf("Program begin:\n");
        fprintf(OUT,"Program begin:\n");
        while(!feof(IN))
        {
            sym=nextsym();
            if(sym==constsym)
                constdec();
            if(sym==intsym||sym==charsym){//类型标识符
                symtype=(sym==intsym?1:0);
                differ();
            }
            if(sym==voidsym)
            {
                sym=nextsym();
                if(sym!=mainsym)//无返回值函数
                    voidfuncdef();
                else //主函数
                    mainfunc();
            }
        }
        printf("Program end\n");
        fprintf(OUT,"Program end\n");
        printf("\nname\tkind\ttype\tvalue\taddr\tlen\tloca\n");
        for(tabi=1;tabi<=tabpc;tabi++){
            printf("%s\t%d\t%d\t%d\t%d\t%d\t%d\t\n",
                   symtab[tabi].name,symtab[tabi].kind,symtab[tabi].type,
                   symtab[tabi].value,symtab[tabi].address,
                   symtab[tabi].length, symtab[tabi].location);
        }
    }
    return 0;
}
