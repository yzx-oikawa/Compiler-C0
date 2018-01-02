#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAXIDENTLEN 50   //标识符最大长度
#define MAXSYMTABLEN 150 //符号表最大长度
#define MAXINTERCODE 50  //四元式每一项最大长度
#define MAXINTERLEN 1000  //四元式序列最大长度
#define MAXASSEMLEN 50  //汇编代码每一项最大长度
#define MAXMIPSTRLEN 50  //汇编字符串序列最大长度和每一项最大长度
#define MAXERRORLEN 30  //错误信息数组长度

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

/*四元式是一个结构*/
struct intercodes{
    char inter0[MAXINTERCODE];
    char inter1[MAXINTERCODE];
    char inter2[MAXINTERCODE];
    char inter3[MAXINTERCODE];
};
struct intercodes intercode[MAXINTERLEN];
int interpc=0;//四元式序列指针
int interi;   //用来遍历四元式序列的参数


/*用于表达式转四元式的返回字符串*/
int exprt=1; //四元式中的临时变量tn计数
int labelt=1; //四元式中的标签计数
int endofcase=1; //单独给switch语句结束的标签计数
char exprtemp[MAXIDENTLEN]; //exprt的字符串形式
char labtemp[MAXIDENTLEN]; //labelt的字符串形式
char exprret[MAXIDENTLEN]; //表达式返回的字符串
char factret[MAXIDENTLEN]; //因子返回的字符串
char termret[MAXIDENTLEN]; //项返回的字符串
char strret[MAXIDENTLEN];  //写语句打印的字符串

/*四元式中的临时变量tn类型，第一个参数是函数的location，第二个参数是n*/
int tntype[MAXIDENTLEN][MAXIDENTLEN];
char tntoken[MAXIDENTLEN]; //tn类型的字符串形式
int tncount[MAXIDENTLEN]; //每个函数中tn的个数

int assems=0;
char assemtemp[MAXASSEMLEN]; //assems的字符串形式

/*用来存汇编字符串的结构*/
struct mipsstring{
    char name[MAXMIPSTRLEN];
    char str [MAXMIPSTRLEN];
};
struct mipsstring mipstring[MAXMIPSTRLEN];
int mipstrpc=0;//汇编字符串序列指针
int mipstri;   //用来遍历汇编字符串序列的参数
int stri=0;    //str序号
char strtemp[MAXMIPSTRLEN];  //stri的字符串形式

char parastack[MAXIDENTLEN][MAXIDENTLEN];//参数栈
int parapc;

char errtype[MAXERRORLEN][MAXERRORLEN];//错误信息
int linecount=1;//行数

FILE *IN, *OUT, *INTER, *ASSEM, *ERROR;

int blankflag=0; //0-跳过空格 1-不跳过空格
int upperflag=0; //0-大写转换为小写 1-输入为字符和常量时区分大小写
int vardecbegflag=0;//1-变量声明开始
int vardecendflag=1;//1-变量声明结束
int logflag;
int retmainflag=0; //main函数返回时跳转到Label_end
int gotomainflag=0;//编译时先跳转到main

char ch; //字符
char token[MAXIDENTLEN]; //当前字符串
char ident[MAXIDENTLEN]; //当前标识符、数字
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
int term(); //项
int factor(); //因子
int statements(); //复合语句
int statement();//语句
int onestatement();//单条语句
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
int strings();//字符串
int checkiflog();//检查是否登录符号表
int checkiftn();//检查是否为四元式的中间变量并返回序号
int checkiflab();//检查是否为四元式的label并返回序号
int tncheckiftn(); //这是什么我也不知道
int tncheckiflog();//回头争取重新写一下
void inter2assem();//四元式转汇编
void saveerror();//保存错误信息
void printerror();//打印错误信息

void saveerror()//保存错误信息
{
    strcpy(errtype[0],"FILE NOT EXIST");
    strcpy(errtype[1],"ILLEGAL SYM");
    strcpy(errtype[2],"UNDEFINED IDENT");
    strcpy(errtype[3],"DUPLICATE DEFINE");
    strcpy(errtype[4],"UNDEFINED FUNCTION");
    strcpy(errtype[5],"WRONG PARA NUMBER");
    strcpy(errtype[6],"WRONG PARA TYPE");
    strcpy(errtype[7],"LACK SINGLE QUOTES");
    strcpy(errtype[8],"LACK DOUBLE QUOTES");
    strcpy(errtype[9],"LACK LEFT PARENT");
    strcpy(errtype[10],"LACK RIGHT PARENT");
    strcpy(errtype[11],"LACK LEFT BRACKET");
    strcpy(errtype[12],"LACK RIGHT BRACKET");
    strcpy(errtype[13],"LACK LEFT BRACE");
    strcpy(errtype[14],"LACK RIGHT BRACE");
    strcpy(errtype[15],"LACK SEMICOLON");
    strcpy(errtype[16],"LACK EQUALMARK");
    strcpy(errtype[17],"LACK COMMA");
    strcpy(errtype[18],"LACK RETURN VALUE");
}

void printerror(int type)//打印错误信息
{
    if(linecount<10)
        fprintf(ERROR,"\nline %d:\t\t%s",linecount,errtype[type]);
    else
        fprintf(ERROR,"\nline %d:\t%s",linecount,errtype[type]);
}


/*语法分析*/
/*＜常量说明＞ ::=  const＜常量定义＞;{ const＜常量定义＞;}*/
int constdec()
{
    fprintf(OUT,"Constdec begin:\n");
    while(1)
    {
        constdef();
        if(sym==semicolon)//分号
            sym=nextsym();
        else
            printerror(15);
        if(sym==constsym)//下一个常量说明
            continue;
        else
        {
            fprintf(OUT,"Constdec end\n");
            return;
        }
    }
}

/*＜常量定义＞   ::=   int＜标识符＞＝＜整数＞{,＜标识符＞＝＜整数＞}
                    | char＜标识符＞＝＜字符＞{,＜标识符＞＝＜字符＞}*/
int constdef()
{
    int subflag=0;
    fprintf(OUT,"\tConstdef\n");
    sym=nextsym();
    if(sym==intsym){
        while(1)
        {
            sym=nextsym();
            if(sym==identsym||sym==chartype)//标识符
            {
                //登录符号表
				logflag=checkiflog();
                if(logflag==-1){
                    strcpy(symtab[++tabpc].name,token);
                    symtab[tabpc].kind=0;//常量
                    symtab[tabpc].type=1;//int
                    symtab[tabpc].address=symaddr;//地址
                    symaddr+=4;
                    symtab[tabpc].location=symloca;//函数位置
                    strcpy(intercode[++interpc].inter0,"const");
                    strcpy(intercode[interpc].inter1,"int");
                    strcpy(intercode[interpc].inter2,token);
                }
                else{
                    printerror(3);
                    fprintf(ERROR," %s",token);
                }
                sym=nextsym();
                if(sym==equmark)//等号
                    sym=nextsym();
                else
                    printerror(16);
                subflag=0;
                if(sym==add||sym==sub){
                    if(sym==sub) subflag=1;
                    sym=nextsym();
                }
                if(sym==inttype||sym==numtype)//整数
                {
                    if(logflag==-1){
                        if(subflag==1)
                            symtab[tabpc].value=-symnum;//值
                        else
                            symtab[tabpc].value=symnum;//值
                        strcpy(intercode[interpc].inter3,token);
                    }
                    sym=nextsym();
                    if(sym==comma)//逗号
                        continue;
                    else if(sym==semicolon)//分号
                        return;
                    else
                        printerror(15);
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
				logflag=checkiflog();
                if(logflag==-1){
                    strcpy(symtab[++tabpc].name,token);
                    symtab[tabpc].kind=0;//常量
                    symtab[tabpc].type=0;//char
                    symtab[tabpc].address=symaddr;//地址
                    symaddr+=4;
                    symtab[tabpc].location=symloca;//函数位置
                    strcpy(intercode[++interpc].inter0,"const");
                    strcpy(intercode[interpc].inter1,"char");
                    strcpy(intercode[interpc].inter2,token);
                }
                else{
                    printerror(3);
                    fprintf(ERROR," %s",token);
                }
                sym=nextsym();
                if(sym==equmark)//等号
                    sym=nextsym();
                else
                    printerror(16);
                if(sym==sinquo)//单引号
                {
                    upperflag=1;
                    sym=nextsym();
                    upperflag=0;
                }
                else
                    printerror(7);
                if(sym==chartype||sym==numtype||
                    (sym>=add&&sym<=divi)||sym==underline)//字符
                {
                    if(logflag==-1){
                        symtab[tabpc].value=token[0];//值
                        strcpy(intercode[interpc].inter3,token);
                    }
                    sym=nextsym();
                    if(sym==sinquo)//单引号
                        sym=nextsym();
                    else
                        printerror(7);
                    if(sym==comma)//逗号
                        continue;
                    else if(sym==semicolon)//分号
                        return;
                    else
                        printerror(15);
                }
            }
        }
    }
}

/*＜变量说明＞  ::= ＜变量定义＞;{＜变量定义＞;}*/
int vardec()
{
    if(vardecbegflag==0){
        fprintf(OUT,"Vardec begin:\n");
        vardecbegflag=1;
        vardecendflag=0;
    }
    while(1)
    {
        //sym是左方括号或者逗号或者分号
        vardef();
        if(sym==semicolon)//分号
            sym=nextsym();
        else
            printerror(15);
        if(sym==intsym||sym==charsym){//下一个变量说明或者函数定义
            symtype=(sym==intsym?1:0);
            return;
        }
        else{
            if(vardecendflag==0){
                fprintf(OUT,"Vardec end\n");
                vardecendflag=1;
            }
            return;
        }
    }
}

/*＜变量定义＞  ::= ＜类型标识符＞(＜标识符＞|＜标识符＞‘[’＜无符号整数＞‘]’){,(＜标识符＞|＜标识符＞‘[’＜无符号整数＞‘]’ )}  */
int vardef()
{
    fprintf(OUT,"\tVardef\n");
    while(1)
    {
        //sym是左方括号或者逗号或者分号或者右方括号
        if(sym==semicolon)
            return;
        if(sym==rbracket){//右方括号，缺少左方括号
            printerror(11);
            sym=nextsym();
            if(sym==comma) continue;
            if(sym==semicolon) return;
            else printerror(15);
        }
        sym=nextsym();
        if(sym==identsym||sym==chartype)//标识符
        {
            //登录符号表-名字、类型、种类
			logflag=checkiflog();
            if(logflag==-1){
                strcpy(symtab[++tabpc].name,token);
                symtab[tabpc].kind=1;//变量
                symtab[tabpc].type=symtype;
                symtab[tabpc].address=symaddr;//地址
                symaddr+=4;
                symtab[tabpc].location=symloca;//函数位置
                strcpy(intercode[++interpc].inter0,"var");
                strcpy(intercode[interpc].inter1,symtype==1?"int":"char");
                strcpy(intercode[interpc].inter2,token);
            }
            else{
                printerror(3);
                fprintf(ERROR," %s",token);
            }
            sym=nextsym();
            if(sym==lbracket)//左方括号，是数组
            {
                sym=nextsym();
                if(sym==inttype||sym==numtype)//无符号整数
                {
                    //登录符号表-长度
                    if(logflag==-1){
                        symtab[tabpc].length=symnum;
                        symaddr+=4*(symnum-1);
                        strcpy(intercode[interpc].inter3,token);
                    }

                    sym=nextsym();
                    if(sym==rbracket)//右方括号
                        sym=nextsym();
                    else
                        printerror(12);
                    if(sym==comma)//逗号
                        continue;
                    else if(sym==semicolon)//分号
                        return;
                    else
                        printerror(15);
                }
            }
            else if(sym==comma)//逗号
                continue;
            else if(sym==semicolon)//分号
                return;
            else if(sym==rbracket)//右方括号，缺少方左括号
            {
                printerror(11);
                sym=nextsym();
                if(sym==comma) continue;
                if(sym==semicolon) return;
                else printerror(15);
            }
            else
                printerror(15);
        }
        else if(sym==inttype||sym==numtype)//无符号整数
        {
            //登录符号表-长度
            if(logflag==-1){
                symtab[tabpc].length=symnum;
                symaddr+=4*(symnum-1);
                strcpy(intercode[interpc].inter3,token);
            }
            sym=nextsym();
            if(sym==rbracket)//右方括号
                sym=nextsym();
            else
                printerror(12);
            if(sym==comma)//逗号
                continue;
            else if(sym==semicolon)//分号
                return;
            else if(sym==rbracket)//右方括号，缺少方左括号
            {
                printerror(11);
                sym=nextsym();
                if(sym==comma) continue;
                if(sym==semicolon) return;
                else printerror(15);
            }
            else
                printerror(15);
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
			logflag=checkiflog();
            if(logflag==-1){
                strcpy(symtab[++tabpc].name,token);
                symtab[tabpc].type=symtype;
                symtab[tabpc].location=symloca;//函数位置
                strcpy(intercode[++interpc].inter1,symtype==1?"int":"char");
                strcpy(intercode[interpc].inter2,token);

            }
            else{
                printerror(3);
                fprintf(ERROR," %s",token);
            }
            sym=nextsym();
            if(sym==lbracket||sym==comma||sym==semicolon||sym==rbracket)//左方括号或逗号或分号或右方括号
            {
                if(logflag==-1){
                    symtab[tabpc].kind=1;//变量
                    symtab[tabpc].address=symaddr;
                    symaddr+=4;
                    strcpy(intercode[interpc].inter0,"var");
                }
                vardec();
            }
            else{
                if(vardecendflag==0){
                    fprintf(OUT,"Vardec end\n");
                    vardecendflag=1;
                }
                if (sym==lbrace||sym==lparent){//左花括号或左括号
                    if(logflag==-1){
                        symtab[tabpc].kind=2;//函数
                        symtab[tabpc].value=1;//有返回值
                        symloca++;
                        symtab[tabpc].location=symloca;
                        symaddr=0;
                        symtab[tabpc].address=symaddr;
                        symaddr+=4;
                        strcpy(intercode[interpc].inter0,"func");
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
    exprt=1;
    while(1){
        fprintf(OUT,"Retfuncdef begin:\n");

        if(sym==lbrace)//左花括号
        {
            sym=nextsym();
            statements();//复合语句
            if(sym==rbrace)//右花括号
            {
                fprintf(OUT,"Retfundef end:\n");
                return;
            }
        }
        else if(sym==lparent)//左括号
        {
            parameters();//参数
            if(logflag==-1)
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
                        fprintf(OUT,"Retfuncdef end:\n");
                        tncount[symloca]=exprt-1;
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
    fprintf(OUT,"Voidfuncdef begin:\n");
    symaddr=0;
    symloca++;
    exprt=1;
    while(1)
    {
        if(sym==identsym||sym==chartype)//标识符
        {
			logflag=checkiflog();
            if(logflag==-1){
                strcpy(symtab[++tabpc].name,token);
                symtab[tabpc].kind=2;//函数
                symtab[tabpc].type=0;
                symtab[tabpc].address=symaddr;
                symaddr+=4;
                symtab[tabpc].value=0;//无返回值
                symtab[tabpc].location=symloca;//函数位置
                strcpy(intercode[++interpc].inter0,"func");
                strcpy(intercode[interpc].inter1,"void");
                strcpy(intercode[interpc].inter2,token);
            }
            else{
                fprintf(OUT,"%s DUPLICATE DEFINE\n",token);
            }
            sym=nextsym();
            if(sym==lparent)//左括号
            {
                parameters();//参数
                if(logflag==-1)
                    symtab[tabpc-sympara-1].length=sympara+1;//全局常量、全局变量
                if(sym==rparent)//右括号
                {
                    sym=nextsym();
                    symtab[tabpc-sympara-1].length=sympara+1;//全局常量、全局变量
                    if(sym==lbrace)//左花括号
                    {
                        sym=nextsym();
                        statements();//语句
                        if(sym==rbrace)//右花括号
                        {
                            fprintf(OUT,"Voidfuncdef end\n");
                            strcpy(intercode[++interpc].inter0,"ret");
                            strcpy(intercode[interpc].inter1,"none");
                            tncount[symloca]=exprt-1;
                            return;
                        }
                    }
                }
            }
            else if(sym==lbrace)//左花括号
            {
                symtab[tabpc].length=0;
                sym=nextsym();
                statements();//复合语句
                if(sym==rbrace)//右花括号
                {
                    fprintf(OUT,"Voidfuncdef end\n");
                    strcpy(intercode[++interpc].inter0,"ret");
                    strcpy(intercode[interpc].inter1,"none");
                    tncount[symloca]=exprt-1;
                    return;
                }
            }
        }
    }
}

/*＜参数表＞  ::=  ＜类型标识符＞＜标识符＞{,＜类型标识符＞＜标识符＞}*/
int parameters()//参数表
{
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
			logflag=checkiflog();
            if(sym==identsym||sym==chartype){//标识符
                if(logflag==-1){
                    strcpy(symtab[++tabpc].name,token);
                    symtab[tabpc].kind=3;//参数
                    symtab[tabpc].type=symtype;
                    symtab[tabpc].address=symaddr;
                    symaddr+=4;
                    symtab[tabpc].location=symloca;//函数位置
                    strcpy(intercode[++interpc].inter0,"para");
                    strcpy(intercode[interpc].inter1,symtype==1?"int":"char");
                    strcpy(intercode[interpc].inter2,token);
                }
                else{
                    fprintf(OUT,"%s DUPLICATE DEFINE\n",token);
                }
                sym=nextsym();
                if(sym==comma)//逗号，下一个参数
                {
                    if(logflag==-1)
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
    fprintf(OUT,"Mainfunc begin:\n");
    symaddr=0;
    symloca++;
    exprt=1;
    retmainflag=1;
    strcpy(symtab[++tabpc].name,token);
    symtab[tabpc].kind=2;//函数
    symtab[tabpc].type=0;
    symtab[tabpc].address=symaddr;
    symaddr+=4;
    symtab[tabpc].value=0;//无返回值
    symtab[tabpc].location=symloca;//函数位置
    strcpy(intercode[++interpc].inter0,"func");
    strcpy(intercode[interpc].inter1,"void");
    strcpy(intercode[interpc].inter2,"main");
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
                    fprintf(OUT,"Mainfunc end\n");
                    tncount[symloca]=exprt-1;
                    return;
                }
            }
        }
    }
}

/*＜复合语句＞   ::=  ［＜常量说明＞］［＜变量说明＞］｛＜语句＞｝*/
int statements()
{
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
            case rbrace:
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
                sym=nextsym();
                if(sym==semicolon) continue;
                statement();
                if(sym==rbrace){
                    fprintf(OUT,"statement end\n");
                    return;
                }
                else printf("errorlbrace\n");break;
            case identsym:
            case chartype:
				logflag=checkiflog();
                if(logflag==-1){//不在符号表中{
                    printf("%s UNDEFINED IDENT\n",token);
                }
                sym=nextsym();
                if(sym==lparent||sym==semicolon)//左括号或分号，为函数调用语句
                {
                    if(logflag==2)
                        voidfuncuse();//返回了右括号
                    else if(logflag==3)
                        retfuncuse();//返回了右括号
                    sym=nextsym();
                    if(sym==semicolon)//分号
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

int onestatement()//返回分号
{
    switch(sym)
    {
        case rbrace:
            printf("oneerror\n");
            return;
        case semicolon:
            return;
        case scanfsym:
            readstatement();//返回分号
            return;
        case printfsym:
            writestatement();//返回分号
            return;
        case returnsym:
            returnstatement();//返回分号
            return;
        case identsym:
        case chartype:
            logflag=checkiflog();
            if(logflag==-1){//不在符号表中{
                printf("%s UNDEFINED IDENT\n",token);
            }
            sym=nextsym();
            if(sym==lparent||sym==semicolon)//左括号或分号，为函数调用语句
            {
                if(logflag==2)
                    voidfuncuse();//返回了右括号
                else if(logflag==3)
                    retfuncuse();//返回了右括号
                sym=nextsym();
                if(sym==semicolon)//分号
                    return;
            }
            else if(sym==equmark||sym==lbracket)//等号或左方括号，为赋值语句
            {
                assignstatement();
                if(sym!=semicolon) printf("one assign error\n");
                return;
            }
            else
                printf("onestatementerror\n");
                return;
        default:
            return;
    }
}

/*＜赋值语句＞ ::= ＜标识符＞＝＜表达式＞|＜标识符＞‘[’＜表达式＞‘]’=＜表达式＞*/
int assignstatement() //赋值语句
{
    char assignident[MAXIDENTLEN];//被赋值的变量
    char assarrnum[MAXIDENTLEN];//数组长度
    fprintf(OUT,"Assign statement\n");
    strcpy(assignident,ident);
    //sym此时为等号或左方括号
    if(sym==lbracket)//左方括号，数组赋值
    {
		sym=nextsym();
        expression();//表达式
        strcpy(assarrnum,exprret);
        {
            if(sym==rbracket)//右方括号
            {
                sym=nextsym();
                if(sym==equmark)//等号
                {
					sym=nextsym();
                    expression();//表达式
                    strcpy(intercode[++interpc].inter0,"arrass");
                    strcpy(intercode[interpc].inter1,exprret);
                    strcpy(intercode[interpc].inter2,assarrnum);
                    strcpy(intercode[interpc].inter3,assignident);
                    return;
                }
            }
        }
    }
    else if(sym==equmark)//等号
    {
		sym=nextsym();
        expression();//表达式
        strcpy(intercode[++interpc].inter0,"assign");
        strcpy(intercode[interpc].inter1,exprret);
        strcpy(intercode[interpc].inter3,assignident);
        return;
    }
    return;
}

/*＜条件语句＞::= if ‘(’＜条件＞‘)’＜语句＞else＜语句＞*/
int ifstatement() //情况语句
{
    char iflabel[MAXIDENTLEN];//goto的label
    char elselabel[MAXIDENTLEN];//bz的label
    fprintf(OUT,"If statement begin:\n");
    //sym此时为if
    sym=nextsym();
    if(sym==lparent)//左括号
    {
        itoa(condition(),elselabel,10);//条件
        if(sym==rparent)//右括号
        {
            sym=nextsym();
            if(sym==semicolon)
                sym=nextsym();//语句为空
            else if(sym==lbrace)
                statement();//语句
            else
                onestatement();
            strcpy(intercode[++interpc].inter0,"goto");
            itoa(labelt++,labtemp,10);
            strcpy(iflabel,labtemp);
            strcpy(intercode[interpc].inter1,"Label");
            strcat(intercode[interpc].inter1,labtemp);
            if(sym==rbrace||sym==semicolon)
                sym=nextsym();
            if(sym==elsesym)//else
            {
                fprintf(OUT,"Else statement:\n");
                sym=nextsym();
                strcpy(intercode[++interpc].inter0,"Label");
                strcat(intercode[interpc].inter0,elselabel);
                strcpy(intercode[interpc].inter1,":");
                if(sym==semicolon){//语句为空
                    sym=nextsym();
                    strcpy(intercode[++interpc].inter0,"Label");
                    strcat(intercode[interpc].inter0,iflabel);
                    strcpy(intercode[interpc].inter1,":");
                    return;//返回分号后的符号
                }
                else if(sym==lbrace){
                    statement();//语句
                    strcpy(intercode[++interpc].inter0,"Label");
                    strcat(intercode[interpc].inter0,iflabel);
                    strcpy(intercode[interpc].inter1,":");
                    if(sym==rbrace){
                        sym=nextsym();
                        return;
                    }
                    else printf("iferror\n");
                }
                else{
                    onestatement();
                    strcpy(intercode[++interpc].inter0,"Label");
                    strcat(intercode[interpc].inter0,iflabel);
                    strcpy(intercode[interpc].inter1,":");
                    if(sym==semicolon)
                        sym=nextsym();
                    return;
                }
            }
        }
    }
    printf("iferror\n");
}

/*＜条件＞ ::=  ＜表达式＞＜关系运算符＞＜表达式＞｜＜表达式＞*/
int condition()
{
    char conident[MAXIDENTLEN];
    char relation[MAXIDENTLEN];
    fprintf(OUT,"\tcondition\n");
    //此时sym是左括号
    sym=nextsym();
    expression();//表达式
    strcpy(conident,exprret);
    if(sym>=les&&sym<=equal)//关系运算符
    {
        strcpy(relation,token);
        sym=nextsym();
        expression();
        strcpy(intercode[++interpc].inter0,relation);
        strcpy(intercode[interpc].inter1,conident);
        strcpy(intercode[interpc].inter2,exprret);
        //跳转
        strcpy(intercode[++interpc].inter0,"bz");
        itoa(labelt++,labtemp,10);
        strcpy(intercode[interpc].inter1,"Label");
        strcat(intercode[interpc].inter1,labtemp);
    }
    else
    {
        strcpy(intercode[++interpc].inter0,">");
        strcpy(intercode[interpc].inter1,conident);
        strcpy(intercode[interpc].inter2,"0");
        //跳转
        strcpy(intercode[++interpc].inter0,"bz");
        itoa(labelt++,labtemp,10);
        strcpy(intercode[interpc].inter1,"Label");
        strcat(intercode[interpc].inter1,labtemp);
    }
    return labelt-1;
}

/*＜循环语句＞ ::= while ‘(’＜条件＞‘)’＜语句＞*/
int whilestatement() //循环语句
{
    char beginlabel[MAXIDENTLEN];//循环开始、goto的label
    char endlabel[MAXIDENTLEN];//循环结束、bz的label
    fprintf(OUT,"While statement begin:\n");
    //sym此时为while
    sym=nextsym();
    if(sym==lparent)//左括号
    {
        itoa(labelt++,beginlabel,10);
        strcpy(intercode[++interpc].inter0,"Label");
        strcat(intercode[interpc].inter0,beginlabel);
        strcpy(intercode[interpc].inter1,":");
        itoa(condition(),endlabel,10);//条件
        if(sym==rparent)//右括号
        {

            sym=nextsym();//分号或左花括号
            if(sym==semicolon){//语句为空
                //sym=nextsym();
                strcpy(intercode[++interpc].inter0,"goto");
                strcpy(intercode[interpc].inter1,"Label");
                strcat(intercode[interpc].inter1,beginlabel);
                //循环结束
                strcpy(intercode[++interpc].inter0,"Label");
                strcat(intercode[interpc].inter0,endlabel);
                strcpy(intercode[interpc].inter1,":");
                return;//返回分号后的符号
            }
            else if(sym==lbrace){
                statement();//语句,返回右花括号
                strcpy(intercode[++interpc].inter0,"goto");
                strcpy(intercode[interpc].inter1,"Label");
                strcat(intercode[interpc].inter1,beginlabel);
                //循环结束
                strcpy(intercode[++interpc].inter0,"Label");
                strcat(intercode[interpc].inter0,endlabel);
                strcpy(intercode[interpc].inter1,":");
                if(sym==rbrace){
                    sym=nextsym();
                    return;
                }
            }
            else{
                onestatement();
                strcpy(intercode[++interpc].inter0,"goto");
                strcpy(intercode[interpc].inter1,"Label");
                strcat(intercode[interpc].inter1,beginlabel);
                //循环结束
                strcpy(intercode[++interpc].inter0,"Label");
                strcat(intercode[interpc].inter0,endlabel);
                strcpy(intercode[interpc].inter1,":");
                if(sym==semicolon)
                    sym=nextsym();
                return;
            }
            printf("whileerror\n");
        }
    }
}

/*＜情况语句＞ ::= switch ‘(’＜表达式＞‘)’ ‘{’＜情况表＞[＜缺省＞] ‘}’*/
int switchstatement() //情况语句
{
    char switchlabel[MAXIDENTLEN];//每次用来比较的
    fprintf(OUT,"Switch statement:\n");
    //sym此时为switch
    sym=nextsym();
    {
        if(sym==lparent)//左括号
        {
			sym=nextsym();
            expression();
            strcpy(switchlabel,exprret);
            if(sym==rparent)//右括号
            {
                sym=nextsym();
                if(sym==lbrace)//左花括号
                {
                    sym=nextsym();
                    if(sym==casesym){//case
                        casestatement(switchlabel);
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
int casestatement(char *switchlabel) //情况子语句
{
    //sym此时为case
    int constflag;
    char caselabel[MAXIDENTLEN];
    char endlabel[MAXIDENTLEN];
    itoa(endofcase++,endlabel,10);
    while(1)
    {
        strcpy(intercode[++interpc].inter0,"==");
        strcpy(intercode[interpc].inter1,switchlabel);
        upperflag=1;
        sym=nextsym();
        upperflag=0;
        if(sym==sinquo||sym==inttype||sym==numtype||sym==add||sym==sub)//单引号括起来的字母或数字，或者整数
        {
            constflag=constant();
            if(constflag>0){//常量
                fprintf(OUT,"Case statement\n");
            }
            if(constflag==2){//char
                itoa((int)ident[0],ident,10);
            }
            if(constflag==3){//取相反数
                strcpy(intercode[interpc].inter2,"-");
                strcat(intercode[interpc].inter2,ident);
            }
            else
                strcpy(intercode[interpc].inter2,ident);
            //跳转
            strcpy(intercode[++interpc].inter0,"bz");
            itoa(labelt++,caselabel,10);
            strcpy(intercode[interpc].inter1,"Label");
            strcat(intercode[interpc].inter1,caselabel);
            sym=nextsym();
            if(sym==colon)//冒号
            {
                sym=nextsym();
                if(sym==semicolon){//语句为空
                    sym=nextsym();//可能返回右花括号、case、default
                }
                else if(sym==lbrace){
                    statement();//语句 可能返回右花括号、case、default
                }
                else{
                    onestatement();
                }
                //结束switch
                strcpy(intercode[++interpc].inter0,"goto");
                strcpy(intercode[interpc].inter1,"Labeleoc");
                strcat(intercode[interpc].inter1,endlabel);

                strcpy(intercode[++interpc].inter0,"Label");
                strcat(intercode[interpc].inter0,caselabel);
                strcpy(intercode[interpc].inter1,":");

                if(sym==semicolon||sym==rbrace)
                    sym=nextsym();
                if(sym==casesym){//下一个case
                    continue;
                }
                else if(sym==defaultsym){//default
                    defaultstatement();
                    strcpy(intercode[++interpc].inter0,"Labeleoc");
                    strcat(intercode[interpc].inter0,endlabel);
                    strcpy(intercode[interpc].inter1,":");
                    return;
                }
                else if (sym==rbrace){//右花括号
                    strcpy(intercode[++interpc].inter0,"Labeleoc");
                    strcat(intercode[interpc].inter0,endlabel);
                    strcpy(intercode[interpc].inter1,":");
                    return;
                }
            }
        }
    }
}

/*＜缺省＞ ::=  default : ＜语句＞*/
int defaultstatement() //缺省
{
    fprintf(OUT,"Default statement\n");
    //sym此时为default
    sym=nextsym();
    {
        if(sym==colon)//冒号
        {
            sym=nextsym();
            if(sym==semicolon)//语句为空
                sym=nextsym();//返回右花括号
            else if(sym==lbrace){
                statement();//语句
                sym=nextsym();//返回switch的右花括号
            }
            else{
                onestatement();
                sym=nextsym();//返回switch的右花括号
            }
        }
    }
    return;
}

/*＜有返回值函数调用语句＞ ::= ＜标识符＞‘(’＜值参数表＞‘)’|<标识符>*/
int retfuncuse() //有返回值函数调用语句
{
    char funcname[MAXIDENTLEN];
    fprintf(OUT,"Return funcuse\n");
    //sym此时为左括号或分号
    strcpy(funcname,ident);
    if(sym==semicolon){
        strcpy(intercode[++interpc].inter0,"call");
        strcpy(intercode[interpc].inter1,funcname);
        return;
    }
    else if(sym==lparent)
    {
        sym=nextsym();
        valuepara();//值参数表
        if(sym==rparent)//右括号
        {
            strcpy(intercode[++interpc].inter0,"call");
            strcpy(intercode[interpc].inter1,funcname);
            return;
        }
    }
    printf("retfuncuseerror\n");
}

/*＜无返回值函数调用语句＞ ::= ＜标识符＞‘(’＜值参数表＞‘)’|<标识符>*/
int voidfuncuse() //无返回值函数调用语句
{
    char funcname[MAXIDENTLEN];
    fprintf(OUT,"Void funcuse\n");
    strcpy(funcname,ident);
    if(sym==semicolon){
        strcpy(intercode[++interpc].inter0,"call");
        strcpy(intercode[interpc].inter1,funcname);
        return;
    }
    else if(sym==lparent)
    {
        sym=nextsym();
        valuepara();//值参数表
        if(sym==rparent)//右括号
        {
            strcpy(intercode[++interpc].inter0,"call");
            strcpy(intercode[interpc].inter1,funcname);
            return;
        }
    }
    printf("voidfuncuseerror\n");
}

/*＜值参数表＞   ::= ＜表达式＞{,＜表达式＞}*/
int valuepara() //值参数表
{
    fprintf(OUT,"\tValue Parameters\n");
    while(sym!=rparent)
    {
        expression();
        strcpy(intercode[++interpc].inter0,"push");
        strcpy(intercode[interpc].inter1,exprret);
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
				logflag=checkiflog();
                if(logflag==-1)//不在符号表中
                    printf("%s UNDEFINED IDENT\n",token);
                strcpy(intercode[++interpc].inter0,"scan");
                strcpy(intercode[interpc].inter1,ident);
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
    fprintf(OUT,"Write statement\n");
    //sym此时为printf
    sym=nextsym();
    if(sym==lparent)//左括号
    {
        sym=nextsym();
        if(sym==douquo)//双引号，字符串
        {
            upperflag=1;
            strings();
            upperflag=0;
            strcpy(intercode[++interpc].inter0,"print");
            strcpy(intercode[interpc].inter1,strret);
            sym=nextsym();
            if(sym==comma)//逗号，字符串后有表达式
            {
				sym=nextsym();
                expression();
                strcpy(intercode[++interpc].inter0,"print");
                strcpy(intercode[interpc].inter2,exprret);
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
            strcpy(intercode[++interpc].inter0,"print");
            strcpy(intercode[interpc].inter2,exprret);
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
    //sym此时为return
    fprintf(OUT,"Return statement\n");
    sym=nextsym();
    if(sym==lparent)//左括号
    {
		sym=nextsym();
        expression();
        strcpy(intercode[++interpc].inter0,"ret");
        strcpy(intercode[interpc].inter1,exprret);
        if(sym==rparent){//右括号
            sym=nextsym();//分号
        return;
        }
    }
    else{//分号
        //main函数的return
        if(retmainflag==1){
         strcpy(intercode[++interpc].inter0,"goto");
            strcpy(intercode[interpc].inter1,"Label_end");
        }
        else{
            strcpy(intercode[++interpc].inter0,"ret");
            strcpy(intercode[interpc].inter1,"none");
        }
        return;
    }
}

/*＜常量＞ ::=  ＜整数＞|＜字符＞
  ＜字符＞ ::='＜加法运算符＞'｜'＜乘法运算符＞'｜'＜字母＞'｜'＜数字＞'*/
/*情况语句中，switch后面的表达式和case后面的常量只允许出现int和char类型*/
int constant()
{
    //sym此时为单引号或者整数或正负号
    if(sym==sinquo){//单引号
        sym=nextsym();
        if(sym==chartype||sym==numtype||(sym>=add)&&(sym<=divi))
        {
            strcpy(ident,token);
            sym=nextsym();
            if(sym==sinquo)//单引号
                return 2;
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
    else if(sym==add)//加号
    {
        sym=nextsym();
        if(sym==inttype||sym==numtype)
            return 1;
    }
    else if(sym==sub)//减号
    {
        sym=nextsym();
        if(sym==inttype||sym==numtype)
            return 3;
    }
    return 0;
}

/*＜字符串＞ ::=  "｛十进制编码为32,33,35-126的ASCII字符｝"*/
int strings()
{
    memset(strret, 0, sizeof(strret));
    blankflag=1;
    fprintf(OUT,"\tString\n");
    //sym此时为双引号
    sym=nextsym();
    while(sym!=douquo)
    {
        if(sym>=constsym&&sym<=underline){
            strcat(strret,token);
            sym=nextsym();
        }
        else
            return;
    }
    blankflag=0;
    return;//返回了双引号
}

/*＜表达式＞ ::= ［+｜-］＜项＞{＜加法运算符＞＜项＞}*/
int expression()//表达式
{
    char expr[3][MAXIDENTLEN];
    int exprpc=1;
    int subtermflag=0;//第一个项取负
    fprintf(OUT,"\tExpression begin\n");
    if(sym==add||sym==sub){
        if(sym==sub)
            subtermflag=1;
        sym=nextsym();
    }
    while(1)
    {
        term();
        if(subtermflag==1)
        {
                strcpy(intercode[++interpc].inter0,"assign");
                strcpy(intercode[interpc].inter1,termret);
                strcpy(intercode[interpc].inter2,"-");
                strcpy(tntoken,termret);
                if(tncheckiflog()>=0)//是标识符
                    tntype[symloca][exprt]=symtab[tabi].type;
                else if(tncheckiftn()>0)//是tn
                    tntype[symloca][exprt]=tntype[symloca][tncheckiftn()];
                else //是立即数
                    tntype[symloca][exprt]=1;
                itoa(exprt++,exprtemp,10);
                strcpy(intercode[interpc].inter3,"t");
                strcat(intercode[interpc].inter3,exprtemp);
                strcpy(termret,intercode[interpc].inter3);
                subtermflag=0;
        }
        strcpy(expr[exprpc],termret);
        if(sym==add||sym==sub){
            if(exprpc==1) exprpc=2;
            else if(exprpc==2) exprpc=1;
            if(exprpc==1)
            {
                strcpy(intercode[++interpc].inter0,expr[0]);
                strcpy(intercode[interpc].inter1,expr[1]);
                strcpy(intercode[interpc].inter2,expr[2]);
                strcpy(tntoken,expr[1]);
                if(tncheckiflog()>=0)//是标识符
                    tntype[symloca][exprt]=symtab[tabi].type;
                else if(tncheckiftn()>0)//是tn
                    tntype[symloca][exprt]=tntype[symloca][tncheckiftn()];
                else //是立即数
                {
                    strcpy(tntoken,expr[2]);
                    if(tncheckiflog()>=0)//是标识符
                        tntype[symloca][exprt]=symtab[tabi].type;
                    else if(tncheckiftn()>0)//是tn
                        tntype[symloca][exprt]=tntype[symloca][tncheckiftn()];
                    else //是立即数
                        tntype[symloca][exprt]=1;
                }
                itoa(exprt++,exprtemp,10);
                strcpy(intercode[interpc].inter3,"t");
                strcat(intercode[interpc].inter3,exprtemp);
                strcpy(expr[1],intercode[interpc].inter3);
                if(exprpc==1) exprpc=2;
                else if(exprpc==2) exprpc=1;
            }
            strcpy(expr[0],token);
            sym=nextsym();
            continue;
        }
        else {
            if(exprpc==2)
            {
                strcpy(intercode[++interpc].inter0,expr[0]);
                strcpy(intercode[interpc].inter1,expr[1]);
                strcpy(intercode[interpc].inter2,expr[2]);
                strcpy(tntoken,expr[1]);
                if(tncheckiflog()>=0)//是标识符
                    tntype[symloca][exprt]=symtab[tabi].type;
                else if(tncheckiftn()>0)//是tn
                    tntype[symloca][exprt]=tntype[symloca][tncheckiftn()];
                else //是立即数
                {
                    strcpy(tntoken,expr[2]);
                    if(tncheckiflog()>=0)//是标识符
                        tntype[symloca][exprt]=symtab[tabi].type;
                    else if(tncheckiftn()>0)//是tn
                        tntype[symloca][exprt]=tntype[symloca][tncheckiftn()];
                    else //是立即数
                        tntype[symloca][exprt]=1;
                }
                itoa(exprt++,exprtemp,10);
                strcpy(intercode[interpc].inter3,"t");
                strcat(intercode[interpc].inter3,exprtemp);
                strcpy(expr[1],intercode[interpc].inter3);
            }
            strcpy(exprret,expr[1]);
            fprintf(OUT,"\tExpression end\n");
            return;
        }
    }
}

/*＜项＞ ::= ＜因子＞{＜乘法运算符＞＜因子＞}*/
int term() //项
{
    char ite[3][MAXIDENTLEN];
    int itepc=1;
    fprintf(OUT,"\tTerm\n");
    while(1)
    {
        factor();
        if(strcmp(factret,"ret")==0)//是函数返回值
        {
            strcpy(intercode[++interpc].inter0,"assign");
            strcpy(intercode[interpc].inter1,factret);
            strcpy(tntoken,factret);
            if(tncheckiflog()>=0)//是标识符
                tntype[symloca][exprt]=symtab[tabi].type;
            else if(tncheckiftn()>0)//是tn
                tntype[symloca][exprt]=tntype[symloca][tncheckiftn()];
            else //是立即数
                tntype[symloca][exprt]=1;
            itoa(exprt++,exprtemp,10);
            strcpy(intercode[interpc].inter3,"t");
            strcat(intercode[interpc].inter3,exprtemp);
            strcpy(ite[itepc],intercode[interpc].inter3);
        }
        else
            strcpy(ite[itepc],factret);
        if(sym==mult||sym==divi){
            if(itepc==1) itepc=2;
            else if(itepc==2) itepc=1;
            if(itepc==1)
            {
                strcpy(intercode[++interpc].inter0,ite[0]);
                strcpy(intercode[interpc].inter1,ite[1]);
                strcpy(intercode[interpc].inter2,ite[2]);
                strcpy(tntoken,ite[1]);
                if(tncheckiflog()>=0)//是标识符
                    tntype[symloca][exprt]=symtab[tabi].type;
                else if(tncheckiftn()>0)//是tn
                    tntype[symloca][exprt]=tntype[symloca][tncheckiftn()];
                else //是立即数
                {
                    strcpy(tntoken,ite[2]);
                    if(tncheckiflog()>=0)//是标识符
                        tntype[symloca][exprt]=symtab[tabi].type;
                    else if(tncheckiftn()>0)//是tn
                        tntype[symloca][exprt]=tntype[symloca][tncheckiftn()];
                    else //是立即数
                        tntype[symloca][exprt]=1;
                }
                itoa(exprt++,exprtemp,10);
                strcpy(intercode[interpc].inter3,"t");
                strcat(intercode[interpc].inter3,exprtemp);
                strcpy(ite[1],intercode[interpc].inter3);
                if(itepc==1) itepc=2;
                else if(itepc==2) itepc=1;
            }
            strcpy(ite[0],token);

            sym=nextsym();
            continue;
        }
        else{
            if(itepc==2)
            {
                strcpy(intercode[++interpc].inter0,ite[0]);
                strcpy(intercode[interpc].inter1,ite[1]);
                strcpy(intercode[interpc].inter2,ite[2]);
                strcpy(tntoken,ite[1]);
                if(tncheckiflog()>=0)//是标识符
                    tntype[symloca][exprt]=symtab[tabi].type;
                else if(tncheckiftn()>0)//是tn
                    tntype[symloca][exprt]=tntype[symloca][tncheckiftn()];
                else //是立即数
                {
                    strcpy(tntoken,ite[2]);
                    if(tncheckiflog()>=0)//是标识符
                        tntype[symloca][exprt]=symtab[tabi].type;
                    else if(tncheckiftn()>0)//是tn
                        tntype[symloca][exprt]=tntype[symloca][tncheckiftn()];
                    else //是立即数
                        tntype[symloca][exprt]=1;
                }
                itoa(exprt++,exprtemp,10);
                strcpy(intercode[interpc].inter3,"t");
                strcat(intercode[interpc].inter3,exprtemp);
                strcpy(ite[1],intercode[interpc].inter3);
            }
            strcpy(termret,ite[1]);
            return;
        }
    }
}

/*＜因子＞::= ＜标识符＞｜＜标识符＞‘[’＜表达式＞‘]’|
‘(’＜表达式＞‘)’｜＜整数＞|＜字符＞｜＜有返回值函数调用语句＞ */
int factor() //因子
{
    char factident[MAXIDENTLEN];
    char factarray[MAXIDENTLEN];
    fprintf(OUT,"\tFactor\n");
    switch(sym)
    {
        case identsym:
        case chartype:
            logflag=checkiflog();
            if(logflag==-1)//不在符号表中
                printf("%s UNDEFINED IDENT\n",token);
            strcpy(factident,ident);
            sym=nextsym();
            if(sym==lbracket){//左方括号，数组
                sym=nextsym();
                expression();
                strcpy(factarray,exprret);//数组系数
                if(sym==rbracket){//右方括号
                    sym=nextsym();
                    strcpy(intercode[++interpc].inter0,"arrget");
                    strcpy(intercode[interpc].inter1,factident);
                    strcpy(intercode[interpc].inter2,factarray);
                    strcpy(tntoken,factident);
                    if(tncheckiflog()>=0)//是标识符
                        tntype[symloca][exprt]=symtab[tabi].type;
                    else if(tncheckiftn()>0)//是tn
                        tntype[symloca][exprt]=tntype[symloca][tncheckiftn()];
                    else //是立即数
                        tntype[symloca][exprt]=1;
                    itoa(exprt++,exprtemp,10);
                    strcpy(intercode[interpc].inter3,"t");
                    strcat(intercode[interpc].inter3,exprtemp);
                    strcpy(factret,intercode[interpc].inter3);
                    return;
                }
            }
            else if(sym==lparent)//左括号，有返回值函数调用
            {
                retfuncuse();//返回右括号
                sym=nextsym();
                strcpy(factret,"ret");
                return;//
            }
            else{//标识符
                if(logflag==3)//有返回值函数调用
                {
                    retfuncuse();//返回分号
                    strcpy(factret,"ret");
                    return;
                }
                else
                    strcpy(factret,ident);
                return;
            }
            break;
        case lparent://表达式
            sym=nextsym();
            expression();
            strcpy(factret,exprret);
            if(sym==rparent){//右括号
                sym=nextsym();
                return;
            }
            break;

        case add://整数前正负号
            sym=nextsym();
            if(sym==inttype||sym==numtype){
                sym=nextsym();
                strcpy(factret,ident);
                return;
            }
            break;
        case sub:
            sym=nextsym();
            if(sym==inttype||sym==numtype){
                sym=nextsym();
                strcpy(factret,"-");
                strcat(factret,ident);
                return;
            }
            break;
        case inttype:
        case numtype://无符号整数
            sym=nextsym();
            strcpy(factret,ident);
            return;
        case sinquo://字符
            upperflag=1;
            sym=nextsym();
            upperflag=0;
            if((sym>=add&&sym<=divi)||sym==chartype||sym==numtype){
                itoa((int)token[0],ident,10);
                strcpy(factret,ident);
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
    if(ch=='0'){//如果是零，直接返回
        token[t]='0';
        strcpy(ident,token);
        return numtype;
    }
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
        strcpy(ident,token);
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
    else if (isalpha(ch)||ch=='_') //如果是字母
    {
        if(upperflag==0)
            token[t]=tolower(ch); //统一存成小写
        else
            token[t]=ch;
        ch=fgetc(IN);
        while(isdigit(ch)||isalpha(ch)||ch=='_')
        {
            if(upperflag==0)
                token[++t]=tolower(ch);
            else
                token[++t]=ch;
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
            strcpy(ident,token);
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
        token[t]=ch;
        switch(ch)
        {
            //跳过空白字符
            case '\n':
                linecount++;
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
            case '%':
            case '&':
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

void inter2assem()//四元式转汇编
{
    //每次操作使用的寄存器：$s0,$s1,$s2
    symloca=0;
    int op=0;//0-add 1-sub 2-mult 3-divi
    int con=0;//0== 1!= 2< 3<= 4> 5>=
    int numofpara=0, parai; //参数个数、参数栈指针
    int arrayaddr; //用于数组赋值的临时变量，保存数组在符号表中的位置
    int defloc,callloc;//函数定义和调用时所在的程序块
    char funcname[MAXIDENTLEN];
    fprintf(ASSEM,".kdata\n");
    fprintf(ASSEM,"\tenter: .ASCIIZ \"\\n\"\n");
    for(interi=1;interi<=interpc;interi++)
    {
        if(strcmp(intercode[interi].inter0,"print")==0){
            if(intercode[interi].inter1[0]!='\0')
            {
                if(mipstrpc==0)
                {
                    itoa(stri++,strtemp,10);
                    strcpy(mipstring[mipstrpc].name,"str");
                    strcat(mipstring[mipstrpc].name,strtemp);
                    strcpy(mipstring[mipstrpc++].str,intercode[interi].inter1);
                }
                for(mipstri=0;mipstri<mipstrpc;mipstri++)
                {
                    if(strcmp(intercode[interi].inter1,mipstring[mipstri].str)==0)
                        break;
                    if(mipstri==mipstrpc-1)//未保存的字符串
                    {
                        itoa(stri++,strtemp,10);
                        strcpy(mipstring[mipstrpc].name,"str");
                        strcat(mipstring[mipstrpc].name,strtemp);
                        strcpy(mipstring[mipstrpc++].str,intercode[interi].inter1);
                        break;
                    }
                }
            }
        }
    }
    for(mipstri=0;mipstri<mipstrpc;mipstri++)
    {
        fprintf(ASSEM,"\t%s: .ASCIIZ \"%s\"\n",mipstring[mipstri].name,mipstring[mipstri].str);
    }
    fprintf(ASSEM,".text\n");
    fprintf(ASSEM,"\tli $fp, 0x00000000\n");
    fprintf(ASSEM,"\tli $sp, 0x00000000\n");
    for(interi=1;interi<=interpc;interi++)
    {
        memset(token, 0, sizeof(token));
        //常量定义
        if(strcmp(intercode[interi].inter0,"const")==0)
        {
            strcpy(token,intercode[interi].inter2);
            //全局常量定义
            if(checkiflog()==0&&symtab[tabi].location==0)
            {
                fprintf(ASSEM,"\tli $s0,%d\n",symtab[tabi].value);
                fprintf(ASSEM,"\tsw $s0,0($fp)\n");
                //移动fp
                fprintf(ASSEM,"\taddi $fp,$fp,4\n");
                //移动sp
                fprintf(ASSEM,"\tor $sp,$0,$fp\n");
            }
            //局部常量定义
            if(checkiflog()==0&&symtab[tabi].location>0)
            {
                fprintf(ASSEM,"\tli $s0,%d\n",symtab[tabi].value);
                fprintf(ASSEM,"\tsw $s0,0($sp)\n");
                //移动sp
                fprintf(ASSEM,"\taddi $sp,$sp,4\n");
            }

        }
        //变量定义
        else if(strcmp(intercode[interi].inter0,"var")==0)
        {
            strcpy(token,intercode[interi].inter2);
            //全局变量定义
            if(checkiflog()==1&&symtab[tabi].location==0)
            {
                //移动fp
                fprintf(ASSEM,"\taddi $fp,$fp,%d\n",symtab[tabi].length==0?4:symtab[tabi].length*4);
                //移动sp
                fprintf(ASSEM,"\tor $sp,$0,$fp\n");
            }
            //局部变量定义
            if(checkiflog()==1&&symtab[tabi].location>0)
            {
                fprintf(ASSEM,"\tli $s0,0\n");
                fprintf(ASSEM,"\tsw $s0,0($sp)\n");
                //移动sp
                fprintf(ASSEM,"\taddi $sp,$sp,%d\n",symtab[tabi].length==0?4:symtab[tabi].length*4);
            }
        }
        //函数定义
        else if(strcmp(intercode[interi].inter0,"func")==0)
        {
            if(gotomainflag==0)
            {
                fprintf(ASSEM,"\taddi $sp,$fp,4\n");
                fprintf(ASSEM,"\tj Label_main\n");
                gotomainflag=1;
            }
            symloca++;
            strcpy(token,intercode[interi].inter2);
            //有返回值函数定义
            if(strcmp(intercode[interi].inter1,"int")==0
            ||strcmp(intercode[interi].inter1,"char")==0)
            {
                if(checkiflog()==3){
                    defloc=symtab[tabi].location;
                    fprintf(ASSEM,"Label_%s:\n",token);
                    fprintf(ASSEM,"\tsw $ra,4($sp)\n",token);
                    fprintf(ASSEM,"\taddi $fp,$sp,16\n");
                    fprintf(ASSEM,"\taddi $sp,$fp,4\n");
                    //留出tn的位置
                    fprintf(ASSEM,"\taddi $sp,$sp,%d\n",4*tncount[defloc]);
                }
            }
            //无返回值函数定义
            else if(strcmp(intercode[interi].inter1,"void")==0)
            {
                if(checkiflog()==2){
                    defloc=symtab[tabi].location;
                    fprintf(ASSEM,"Label_%s:\n",token);
                }
                if(strcmp(token,"main")!=0){
                    fprintf(ASSEM,"\tsw $ra,4($sp)\n",token);
                    fprintf(ASSEM,"\taddi $fp,$sp,16\n");
                    fprintf(ASSEM,"\taddi $sp,$fp,4\n");
                }
                //留出tn的位置
                fprintf(ASSEM,"\taddi $sp,$sp,%d\n",4*tncount[defloc]);
            }

        }
        //参数定义
        else if(strcmp(intercode[interi].inter0,"para")==0)
        {
            strcpy(token,intercode[interi].inter2);
            if(checkiflog()==4)
                //移动sp
                fprintf(ASSEM,"\taddi $sp,$sp,4\n");
        }
        //赋值语句
        else if(strcmp(intercode[interi].inter0,"assign")==0)
        {
            //被赋的值
            strcpy(token,intercode[interi].inter1);
            if(strcmp(token,"ret")==0)//返回值
            {
                fprintf(ASSEM,"\tlw $s0,8($sp)\n");
            }
            else if(checkiflog()==0||checkiflog()==1||checkiflog()==4)//常量或变量或参数
            {
                if(symtab[tabi].location==0)//全局常变量
                    fprintf(ASSEM,"\tlw $s0,%d($0)\n",symtab[tabi].address);
                else//局部常变量
                    fprintf(ASSEM,"\tlw $s0,%d($fp)\n",4*tncount[defloc]+symtab[tabi].address);
            }
            else if(checkiftn()>0)//tn
                fprintf(ASSEM,"\tlw $s0,%d($fp)\n",checkiftn()*4);
            else//立即数
                fprintf(ASSEM,"\tori $s0,$0,%s\n",token);

            //取相反数
            if(intercode[interi].inter2[0]=='-')
            {
                fprintf(ASSEM,"\tsub $s0,$0,$s0\n");
            }

            //赋给的值
            strcpy(token,intercode[interi].inter3);
            if(checkiflog()==1||checkiflog()==4)//变量或参数
            {
                if(symtab[tabi].location==0)//全局变量
                    fprintf(ASSEM,"\tsw $s0,%d($0)\n",symtab[tabi].address);
                else //局部变量
                    fprintf(ASSEM,"\tsw $s0,%d($fp)\n",4*tncount[defloc]+symtab[tabi].address);
            }
            else if(checkiftn()>0)//tn
                fprintf(ASSEM,"\tsw $s0,%d($fp)\n",checkiftn()*4);
        }
        //给数组赋值
        else if(strcmp(intercode[interi].inter0,"arrass")==0)
        {
            //被赋的值
            strcpy(token,intercode[interi].inter1);
            if(strcmp(token,"ret")==0)//返回值
            {
                fprintf(ASSEM,"\tlw $s0,8($sp)\n");
            }
            else if(checkiflog()==0||checkiflog()==1||checkiflog()==4)//常量或变量或参数
            {
                if(symtab[tabi].location==0)//全局常变量
                    fprintf(ASSEM,"\tlw $s0,%d($0)\n",symtab[tabi].address);
                else//局部常变量
                    fprintf(ASSEM,"\tlw $s0,%d($fp)\n",4*tncount[defloc]+symtab[tabi].address);
            }
            else if(checkiftn()>0)//tn
                fprintf(ASSEM,"\tlw $s0,%d($fp)\n",checkiftn()*4);
            else//立即数
                fprintf(ASSEM,"\tori $s0,$0,%s\n",token);

            //赋给的值
            strcpy(token,intercode[interi].inter3);
            if(checkiflog()==1)
            {
                //defloc=symtab[tabi].location;
                if(symtab[tabi].location==0)//全局变量
                {
                    arrayaddr=symtab[tabi].address;
                    strcpy(token,intercode[interi].inter2);
                    if(checkiflog()==0||checkiflog()==1||checkiflog()==4)//常量或变量或参数
                    {
                        if(symtab[tabi].location==0)//全局常变量
                            fprintf(ASSEM,"\tlw $s1,%d($0)\n",symtab[tabi].address);
                        else//局部常变量
                            fprintf(ASSEM,"\tlw $s1,%d($fp)\n",4*tncount[defloc]+symtab[tabi].address);
                    }
                    else if(checkiftn()>0)//tn
                        fprintf(ASSEM,"\tlw $s1,%d($fp)\n",checkiftn()*4);
                    else//立即数
                        fprintf(ASSEM,"\tori $s1,$0,%s\n",token);
                    fprintf(ASSEM,"\tmul $s1,$s1,4\n");
                    fprintf(ASSEM,"\taddi $s1,$s1,%d\n",arrayaddr);
                    fprintf(ASSEM,"\tsw $s0,0($s1)\n");
                }
                else//局部变量
                {
                    arrayaddr=symtab[tabi].address;
                    strcpy(token,intercode[interi].inter2);
                    if(checkiflog()==0||checkiflog()==1||checkiflog()==4)//常量或变量或参数
                    {
                        if(symtab[tabi].location==0)//全局常变量
                            fprintf(ASSEM,"\tlw $s1,%d($0)\n",symtab[tabi].address);
                        else//局部常变量
                            fprintf(ASSEM,"\tlw $s1,%d($fp)\n",4*tncount[defloc]+symtab[tabi].address);
                    }
                    else if(checkiftn()>0)//tn
                        fprintf(ASSEM,"\tlw $s1,%d($fp)\n",checkiftn()*4);
                    else//立即数
                        fprintf(ASSEM,"\tori $s1,$0,%s\n",token);
                    fprintf(ASSEM,"\tmul $s1,$s1,4\n");
                    fprintf(ASSEM,"\taddi $s1,$s1,%d\n",arrayaddr+4*tncount[defloc]);
                    fprintf(ASSEM,"\tadd $s1,$s1,$fp\n");
                    fprintf(ASSEM,"\tsw $s0,0($s1)\n");
                }
            }

        }
        //取数组的值
        else if(strcmp(intercode[interi].inter0,"arrget")==0)
        {
            //被赋的值
            strcpy(token,intercode[interi].inter1);
            if(checkiflog()==1)
            {
                if(symtab[tabi].location==0)//全局变量
                {
                    arrayaddr=symtab[tabi].address;
                    strcpy(token,intercode[interi].inter2);
                    if(checkiflog()==0||checkiflog()==1||checkiflog()==4)//常量或变量或参数
                    {
                        if(symtab[tabi].location==0)//全局常变量
                            fprintf(ASSEM,"\tlw $s1,%d($0)\n",symtab[tabi].address);
                        else//局部常变量
                            fprintf(ASSEM,"\tlw $s1,%d($fp)\n",4*tncount[defloc]+symtab[tabi].address);
                    }
                    else if(checkiftn()>0)//tn
                        fprintf(ASSEM,"\tlw $s1,%d($fp)\n",checkiftn()*4);
                    else//立即数
                        fprintf(ASSEM,"\tori $s1,$0,%s\n",token);
                    fprintf(ASSEM,"\tmul $s1,$s1,4\n");
                    fprintf(ASSEM,"\taddi $s1,$s1,%d\n",arrayaddr);
                    fprintf(ASSEM,"\tlw $s0,0($s1)\n");
                }
                else//局部变量
                {
                    arrayaddr=symtab[tabi].address;
                    strcpy(token,intercode[interi].inter2);
                    if(checkiflog()==0||checkiflog()==1||checkiflog()==4)//常量或变量或参数
                    {
                        if(symtab[tabi].location==0)//全局常变量
                            fprintf(ASSEM,"\tlw $s1,%d($0)\n",symtab[tabi].address);
                        else//局部常变量
                            fprintf(ASSEM,"\tlw $s1,%d($fp)\n",4*tncount[defloc]+symtab[tabi].address);
                    }
                    else if(checkiftn()>0)//tn
                        fprintf(ASSEM,"\tlw $s1,%d($fp)\n",checkiftn()*4);
                    else//立即数
                        fprintf(ASSEM,"\tori $s1,$0,%s\n",token);
                    fprintf(ASSEM,"\tmul $s1,$s1,4\n");
                    fprintf(ASSEM,"\taddi $s1,$s1,%d\n",arrayaddr+4*tncount[defloc]);
                    fprintf(ASSEM,"\tadd $s1,$s1,$fp\n");
                    fprintf(ASSEM,"\tlw $s0,0($s1)\n");
                }
            }

            //赋给的值
            strcpy(token,intercode[interi].inter3);
            if(checkiflog()==1||checkiflog()==4)//变量或参数
            {
                if(symtab[tabi].location==0)//全局变量
                    fprintf(ASSEM,"\tsw $s0,%d($0)\n",symtab[tabi].address);
                else//局部变量
                    fprintf(ASSEM,"\tsw $s0,%d($fp)\n",4*tncount[defloc]+symtab[tabi].address);
            }
            else if(checkiftn()>0)//tn
                fprintf(ASSEM,"\tsw $s0,%d($fp)\n",checkiftn()*4);


        }
        //加减乘除
        else if(strcmp(intercode[interi].inter0,"+")==0||strcmp(intercode[interi].inter0,"-")==0||
                strcmp(intercode[interi].inter0,"*")==0||strcmp(intercode[interi].inter0,"/")==0)
        {
            if(strcmp(intercode[interi].inter0,"+")==0) op=0;
            else if(strcmp(intercode[interi].inter0,"-")==0) op=1;
            else if(strcmp(intercode[interi].inter0,"*")==0) op=2;
            else if(strcmp(intercode[interi].inter0,"/")==0) op=3;
            //第一个操作数
            strcpy(token,intercode[interi].inter1);
            if(checkiflog()==0||checkiflog()==1||checkiflog()==4)//常量或变量或参数
            {
                if(symtab[tabi].location==0)//全局常变量
                    fprintf(ASSEM,"\tlw $s1,%d($0)\n",symtab[tabi].address);
                else//局部常变量
                    fprintf(ASSEM,"\tlw $s1,%d($fp)\n",4*tncount[defloc]+symtab[tabi].address);
            }
            else if(checkiftn()>0)//tn
                fprintf(ASSEM,"\tlw $s1,%d($fp)\n",checkiftn()*4);
            else//立即数
                fprintf(ASSEM,"\tori $s1,$0,%s\n",token);

            //第二个操作数
            strcpy(token,intercode[interi].inter2);
            if(checkiflog()==0||checkiflog()==1||checkiflog()==4)//常量或变量或参数
            {
                if(symtab[tabi].location==0)//全局常变量
                    fprintf(ASSEM,"\tlw $s2,%d($0)\n",symtab[tabi].address);
                else//局部常变量
                    fprintf(ASSEM,"\tlw $s2,%d($fp)\n",4*tncount[defloc]+symtab[tabi].address);
            }
            else if(checkiftn()>0)//tn
                fprintf(ASSEM,"\tlw $s2,%d($fp)\n",checkiftn()*4);
            else//立即数
                fprintf(ASSEM,"\tori $s2,$0,%s\n",token);

            //加减乘除操作
            //fprintf(ASSEM,"%d\n",op);
            if(op==0)//加法
                fprintf(ASSEM,"\tadd $s0,$s1,$s2\n");
            else if(op==1)//减法
                fprintf(ASSEM,"\tsub $s0,$s1,$s2\n");
            else if(op==2)//乘法
                fprintf(ASSEM,"\tmul $s0,$s1,$s2\n");
            else if(op==3)//除法
                fprintf(ASSEM,"\tdiv $s0,$s1,$s2\n");

            //运算结果
            strcpy(token,intercode[interi].inter3);
            if(checkiflog()==1||checkiflog()==4)//变量或参数
            {
                if(symtab[tabi].location==0)//全局变量
                    fprintf(ASSEM,"\tsw $s0,%d($0)\n",symtab[tabi].address);
                else //局部变量
                    fprintf(ASSEM,"\tsw $s0,%d($fp)\n",4*tncount[defloc]+symtab[tabi].address);
            }
            else if(checkiftn()>0){//tn
                fprintf(ASSEM,"\tsw $s0,%d($fp)\n",checkiftn()*4);
            }
        }
        //函数调用
        else if(strcmp(intercode[interi].inter0,"push")==0)
        {
            strcpy(parastack[parapc++],intercode[interi].inter1);
        }
        else if(strcmp(intercode[interi].inter0,"call")==0)
        {
            strcpy(token,intercode[interi].inter1);
            if(checkiflog()==2||checkiflog()==3)//是函数
            {
                callloc=symtab[tabi].location;
                numofpara=symtab[tabi].length;
                if(numofpara==0)//无参数
                {
                    //把fp存到sp+4
                    fprintf(ASSEM,"\tsw $fp,16($sp)\n");
                    //把sp存到sp+3
                    fprintf(ASSEM,"\tsw $sp,12($sp)\n");
                    //把返回值存到sp+2，把PC返回地址存到sp+1
                    fprintf(ASSEM,"\tjal Label_%s\n",token);
                }
                else
                {
                    strcpy(funcname,token);
                    //把fp存到sp+4
                    fprintf(ASSEM,"\tsw $fp,16($sp)\n");
                    //把sp存到sp+3
                    fprintf(ASSEM,"\tsw $sp,12($sp)\n");
                    for(parai=numofpara;parai>=1;parai--)
                    {
                        //对每一个形参
                        strcpy(token,parastack[parapc-parai]);
                        if(checkiflog()==0||checkiflog()==1||checkiflog()==4)//常量或变量或参数
                        {
                            if(symtab[tabi].location==0)//全局常变量
                                fprintf(ASSEM,"\tlw $s0,%d($0)\n",symtab[tabi].address);
                            else//局部常变量
                            {
                                fprintf(ASSEM,"\tlw $s0,%d($fp)\n",4*tncount[defloc]+symtab[tabi].address);
                            }
                        }
                        else if(checkiftn()>0)//tn
                            fprintf(ASSEM,"\tlw $s0,%d($fp)\n",checkiftn()*4);
                        else//立即数
                            fprintf(ASSEM,"\tori $s0,$0,%s\n",token);
                        fprintf(ASSEM,"\tsw $s0,%d($sp)\n",4*tncount[callloc]+16+(numofpara-parai+1)*4);
                    }
                    parapc=parapc-numofpara;
                    //把返回值存到sp+2，把PC返回地址存到sp+1
                    fprintf(ASSEM,"\tjal Label_%s\n",funcname);
                }
            }

        }
        //函数返回
        else if(strcmp(intercode[interi].inter0,"ret")==0)
        {
            //返回值
            strcpy(token,intercode[interi].inter1);
            //printf("%s\n",token);
            if(strcmp(token,"none")!=0){
                if(checkiflog()==0||checkiflog()==1||checkiflog()==4)//常量或变量或参数
                {
                    if(symtab[tabi].location==0)//全局常变量
                        fprintf(ASSEM,"\tlw $s0,%d($0)\n",symtab[tabi].address);
                    else//局部常变量
                        fprintf(ASSEM,"\tlw $s0,%d($fp)\n",4*tncount[defloc]+symtab[tabi].address);
                }
                else if(checkiftn()>0)//tn
                    fprintf(ASSEM,"\tlw $s0,%d($fp)\n",checkiftn()*4);
                else//立即数
                    fprintf(ASSEM,"\tori $s0,$0,%s\n",token);
                fprintf(ASSEM,"\tsw $s0,-8($fp)\n");
            }
            fprintf(ASSEM,"\tlw $ra,-12($fp)\n");
            fprintf(ASSEM,"\tlw $sp,-4($fp)\n");
            fprintf(ASSEM,"\tlw $fp,0($fp)\n");
            fprintf(ASSEM,"\tjr $ra\n");
        }
        //Label:
        else if(strcmp(intercode[interi].inter1,":")==0)
        {
            fprintf(ASSEM,"%s:\n",intercode[interi].inter0);
        }
        //goto Label
        else if(strcmp(intercode[interi].inter0,"goto")==0)
        {
            fprintf(ASSEM,"\tj %s\n",intercode[interi].inter1);
        }
        //条件判断
        else if(strcmp(intercode[interi].inter0,"==")==0||strcmp(intercode[interi].inter0,"!=")==0||
                strcmp(intercode[interi].inter0,"<")==0||strcmp(intercode[interi].inter0,"<=")==0||
                strcmp(intercode[interi].inter0,">")==0||strcmp(intercode[interi].inter0,">=")==0)
        {
            if(strcmp(intercode[interi].inter0,"==")==0) con=0;
            else if(strcmp(intercode[interi].inter0,"!=")==0) con=1;
            else if(strcmp(intercode[interi].inter0,"<")==0) con=2;
            else if(strcmp(intercode[interi].inter0,"<=")==0) con=3;
            else if(strcmp(intercode[interi].inter0,">")==0) con=4;
            else if(strcmp(intercode[interi].inter0,">=")==0) con=5;
            //第一个操作数
            strcpy(token,intercode[interi].inter1);
            if(checkiflog()==0||checkiflog()==1||checkiflog()==4)//常量或变量或参数
            {
                if(symtab[tabi].location==0)//全局常变量
                    fprintf(ASSEM,"\tlw $s1,%d($0)\n",symtab[tabi].address);
                else//局部常变量
                    fprintf(ASSEM,"\tlw $s1,%d($fp)\n",4*tncount[defloc]+symtab[tabi].address);
            }
            else if(checkiftn()>0)//tn
                fprintf(ASSEM,"\tlw $s1,%d($fp)\n",checkiftn()*4);
            else//立即数
                fprintf(ASSEM,"\tori $s1,$0,%s\n",token);

            //第二个操作数
            strcpy(token,intercode[interi].inter2);
            if(checkiflog()==0||checkiflog()==1||checkiflog()==4)//常量或变量或参数
            {
                if(symtab[tabi].location==0)//全局常变量
                    fprintf(ASSEM,"\tlw $s2,%d($0)\n",symtab[tabi].address);
                else//局部常变量
                    fprintf(ASSEM,"\tlw $s2,%d($fp)\n",4*tncount[defloc]+symtab[tabi].address);
            }
            else if(checkiftn()>0)//tn
                fprintf(ASSEM,"\tlw $s2,%d($fp)\n",checkiftn()*4);
            else//立即数
                fprintf(ASSEM,"\tori $s2,$0,%s\n",token);

            //比较并跳转操作
            //因为四元式中所有的条件下一句都是bz，故汇编所有操作都是反着的;
            fprintf(ASSEM,"\tsub $s0,$s1,$s2\n");
            if(con==0) // ==
                fprintf(ASSEM,"\tbne $s1,$s2,");
            else if(con==1)// !=
                fprintf(ASSEM,"\tbeq $s1,$s2,");
            else if(con==2)// <
                fprintf(ASSEM,"\tbgez $s0,");
            else if(con==3)// <=
                fprintf(ASSEM,"\tbgtz $s0,");
            else if(con==4)// >
                fprintf(ASSEM,"\tblez $s0,");
            else if(con==5)// >=
                fprintf(ASSEM,"\tbltz $s0,");

            //跳转到Label
            fprintf(ASSEM,"%s\n",intercode[++interi].inter1);
        }
        //scanf
        else if(strcmp(intercode[interi].inter0,"scan")==0)
        {
            //标识符
            strcpy(token,intercode[interi].inter1);
            if(checkiflog()==1||checkiflog()==4)//变量或参数
            {
                if(symtab[tabi].type==1)//读入int
                {
                     fprintf(ASSEM,"\tli $v0,5\n");
                     fprintf(ASSEM,"\tsyscall\n");
                }
                else if(symtab[tabi].type==0)//读入char
                {
                    fprintf(ASSEM,"\tli $v0,12\n");
                     fprintf(ASSEM,"\tsyscall\n");
                }
                if(symtab[tabi].location==0)//全局变量
                    fprintf(ASSEM,"\tsw $v0,%d($0)\n",symtab[tabi].address);
                else//局部变量
                {
                    fprintf(ASSEM,"\tsw $v0,%d($fp)\n",4*tncount[defloc]+symtab[tabi].address);
                }

            }
        }
        //printf
        else if(strcmp(intercode[interi].inter0,"print")==0)
        {
            if(intercode[interi].inter1[0]!='\0')//字符串
            {

                for(mipstri=0;mipstri<mipstrpc;mipstri++)
                {
                    if(strcmp(intercode[interi].inter1,mipstring[mipstri].str)==0)
                    {
                        fprintf(ASSEM,"\tla $a0,%s\n",mipstring[mipstri].name);
                        fprintf(ASSEM,"\tli $v0,4\n");
                        fprintf(ASSEM,"\tsyscall\n");
                        break;
                    }

                }
            }
            else//表达式
            {
                strcpy(token,intercode[interi].inter2);
                if(checkiflog()==0||checkiflog()==1||checkiflog()==4)//常量或变量或参数
                {
                    if(symtab[tabi].location==0)//全局常变量
                        fprintf(ASSEM,"\tlw $a0,%d($0)\n",symtab[tabi].address);
                    else//局部常变量
                        fprintf(ASSEM,"\tlw $a0,%d($fp)\n",4*tncount[defloc]+symtab[tabi].address);
                    if(symtab[tabi].type==1)//打印int
                    {
                         fprintf(ASSEM,"\tli $v0,1\n");
                         fprintf(ASSEM,"\tsyscall\n");
                    }
                    else if(symtab[tabi].type==0)//打印char
                    {
                         fprintf(ASSEM,"\tli $v0,11\n");
                         fprintf(ASSEM,"\tsyscall\n");
                    }
                }
                else if(checkiftn()>0)//tn
                {
                    if(tntype[symloca][checkiftn()]==1)
                    {
                        fprintf(ASSEM,"\tlw $a0,%d($fp)\n",checkiftn()*4);
                        fprintf(ASSEM,"\tli $v0,1\n");
                        fprintf(ASSEM,"\tsyscall\n");
                    }
                    else if(tntype[symloca][checkiftn()]==0)
                    {
                        fprintf(ASSEM,"\tlw $a0,%d($fp)\n",checkiftn()*4);
                        fprintf(ASSEM,"\tli $v0,11\n");
                        fprintf(ASSEM,"\tsyscall\n");
                    }
                }
                else//立即数
                {
                    fprintf(ASSEM,"\tori $a0,$0,%s\n",token);
                    fprintf(ASSEM,"\tli $v0,1\n");
                    fprintf(ASSEM,"\tsyscall\n");
                }
            }
            fprintf(ASSEM,"\tla $a0,enter\n",intercode[interi].inter1);
            fprintf(ASSEM,"\tli $v0,4\n");
            fprintf(ASSEM,"\tsyscall\n");
        }
        fprintf(ASSEM,"\n");
    }
    fprintf(ASSEM,"Label_end:\n");
}

int checkiflog()//检查是否登录符号表并返回类型
{
    //全局常量、全局变量、main常量、main变量、函数常量变量声明、函数参数、函数声明
    for(tabi=tabpc;tabi>=0;tabi--){
        if(symtab[tabi].location==0||symtab[tabi].location==symloca||symtab[tabi].kind==2){
            if (strcmp(token,symtab[tabi].name)==0){//在表中找到了这个标识符
                if(symtab[tabi].kind==0)//是常量
                    return 0;
                else if(symtab[tabi].kind==1)//是变量
                    return 1;
                else if(symtab[tabi].kind==2&&symtab[tabi].value==0)//无返函数
                    return 2;
                else if(symtab[tabi].kind==2&&symtab[tabi].value==1)//有返函数
                    return 3;
                else if(symtab[tabi].kind==3)//是参数
                    return 4;
            }
        }
    }
    return -1;//未登录
}

int checkiftn()//检查是否为四元式的中间变量并返回序号
{
    int i;
    int num;
    num=0;
    //printf("%s",token);
    if(token[0]=='t')
    {
        i=1;
        while(token[i]!='\0')
        {
            if(token[i]>='0'&&token[i]<='9')//是数字
                num=num*10+(token[i]-'0');
            i++;
        }
    }
    return num;
}

int checkiflab()//检查是否为四元式的label并返回序号
{
    int i;
    int num=0;
    if(token[0]=='L'&&token[0]=='a'&&token[0]=='b'
                    &&token[0]=='e'&&token[0]=='l')
        num=1;
    return num;
}

int tncheckiflog()
{
    //全局常量、全局变量、main常量、main变量、函数常量变量声明、函数参数、函数声明
    for(tabi=tabpc;tabi>=0;tabi--){
        if(symtab[tabi].location==0||symtab[tabi].location==symloca||symtab[tabi].kind==2){
            if (strcmp(tntoken,symtab[tabi].name)==0){//在表中找到了这个标识符
                if(symtab[tabi].kind==0)//是常量
                    return 0;
                else if(symtab[tabi].kind==1)//是变量
                    return 1;
                else if(symtab[tabi].kind==2&&symtab[tabi].value==0)//无返函数
                    return 2;
                else if(symtab[tabi].kind==2&&symtab[tabi].value==1)//有返函数
                    return 3;
                else if(symtab[tabi].kind==3)//是参数
                    return 4;
            }
        }
    }
    return -1;//未登录
}

int tncheckiftn()
{
    int i;
    int num;
    num=0;
    //printf("%s",token);
    if(tntoken[0]=='t')
    {
        i=1;
        while(tntoken[i]!='\0')
        {
            if(tntoken[i]>='0'&&tntoken[i]<='9')//是数字
                num=num*10+(tntoken[i]-'0');
            i++;
        }
    }
    return num;
}

int main()
{
    char strline[MAXIDENTLEN];
    IN = fopen("15061091_test.txt","r");
    OUT = fopen("15061091_result.txt","w");
    INTER = fopen("C:\\Users\\Administrator\\Desktop\\inter.txt","w");
    ASSEM = fopen("C:\\Users\\Administrator\\Desktop\\assem.txt","w");
    ERROR = fopen("C:\\Users\\Administrator\\Desktop\\error.txt","w");
    /*C:\\Users\\Administrator\\Desktop\\*/
//    printf("Please input the test file:\n");
//    while(IN == NULL){
//        gets(strline);
//        IN = fopen(strline,"r");
//        if(IN == NULL)
//            printf("NO SUCH FILE! %s\n", strline);
//
//    }
    if(IN == NULL){
        printf("NO SUCH FILE!\n");
    }
    else{
        fprintf(OUT,"Program begin:\n");
        saveerror();
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
        fprintf(OUT,"Program end\n");

        //打印符号表
        printf("name\tkind\ttype\tvalue\taddr\tlen\tloca\n");
        for(tabi=1;tabi<=tabpc;tabi++){
            printf("%s\t%d\t%d\t%d\t%d\t%d\t%d\t\n",
                   symtab[tabi].name,symtab[tabi].kind,symtab[tabi].type,
                   symtab[tabi].value,symtab[tabi].address,
                   symtab[tabi].length, symtab[tabi].location);
        }

        //打印四元式
        fprintf(INTER,"0\t1\t2\t3\n");
        for(interi=1;interi<=interpc;interi++)
        {
            fprintf(INTER,"%s\t%s\t%s\t%s\n",
                intercode[interi].inter0,intercode[interi].inter1,
                intercode[interi].inter2,intercode[interi].inter3);
        }
        inter2assem();
    }
    return 0;
}
