#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define KEYNUM 14

const char *keyword[]={"const","int","char","void","main","if","else","while","switch","case",
                       "default","scanf","printf","return"};
const char *mnemonic[]={"CONST","INT","CHAR","VOID","MAIN","IF","ELSE","WHILE","SWITCH","CASE",

                        "DEFAULT","SCANF","PRINTF","RETURN"};

const enum symbol {constsym, intsym, charsym, voidsym, mainsym, ifsym, elsesym, whilesym,//0-7
                   switchsym, casesym, defaultsym, scanfsym, printfsym, returnsym,       //8-13
                   add, sub, mult, divi, les, loe, mor, moe, noteq, equal,               //14-23
                   comma, colon, semicolon, sinquo, douquo, equmark,                     //24-29
                   lparent, rparent, lbracket, rbracket, lbrace, rbrace,                 //30-35
                   identsym, inttype, chartype, strtype, blank};                         //36-40

int No=1;
int sym;
int blankflag=0; //0-跳过空格 1-不跳过空格
int vardecbegflag=0;//1-变量声明开始
int vardecendflag=0;//1-变量声明结束
int braceflag=0;
char ch; //字符
char token[100]; //字符串

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

/*＜常量说明＞ ::=  const＜常量定义＞;{ const＜常量定义＞;}*/
int constdec(FILE *IN)
{
    printf("Constdec begin:\n");
    while(1)
    {
        constdef(IN);
        //sym=nextsym(IN);
        //printf("1%d\n",sym);
        if(sym==semicolon)//分号
        {
            sym=nextsym(IN);
            //printf("2%d\n",sym);
            if(sym==constsym)//下一个常量说明
                continue;
            else
            {
                printf("Constdec end\n");
                return;
            }
        }
        else return;
    }
}

/*＜常量定义＞   ::=   int＜标识符＞＝＜整数＞{,＜标识符＞＝＜整数＞}
                    | char＜标识符＞＝＜字符＞{,＜标识符＞＝＜字符＞}*/
int constdef(FILE *IN)
{
    printf("\tConstdef begin:\n");
    sym=nextsym(IN);
    if(sym==intsym){
        while(1)
        {
            sym=nextsym(IN);
            if(sym==identsym||sym==chartype)//标识符
            {
                //登录符号表
                sym=nextsym(IN);
                if(sym==equmark)//等号
                {
                    sym=nextsym(IN);
                    if(sym==inttype)//整数
                    {
                        sym=nextsym(IN);
                        if(sym==comma)//逗号
                            continue;
                        else//分号
                        {
                           // printf("\tConstdef end\n");
                            return;
                        }
                    }
                }
            }
        }
    }
    else if(sym==charsym){
        while(1)
        {
            sym=nextsym(IN);
            if(sym==identsym||sym==chartype)//标识符
            {
                //登录符号表
                sym=nextsym(IN);
                if(sym==equmark)//等号
                {
                    sym=nextsym(IN);
                    if(sym==sinquo)//单引号
                    {
                        sym=nextsym(IN);
                        if(sym==chartype)//字符
                        {
                            sym=nextsym(IN);
                            if(sym==sinquo)//单引号
                            {
                                sym=nextsym(IN);
                                if(sym==comma)//逗号
                                    continue;
                                else//分号
                                {
                                    //printf("\tConstdef end\n");
                                    return;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

/*＜变量说明＞  ::= ＜变量定义＞;{＜变量定义＞;}*/
int vardec(FILE *IN)
{
    if(vardecbegflag==0){
        printf("Vardec begin:\n");
        vardecbegflag=1;
    }
    while(1)
    {
        //sym是左方括号或者逗号或者分号
        vardef(IN);
        if(sym==semicolon)//分号
        {
            sym=nextsym(IN);
            if(sym==intsym||sym==charsym)//下一个变量说明或者函数定义
                return;
            else
            {
                if(vardecendflag==0){
                    printf("Vardec end\n");
                    vardecendflag=1;
                }
                //printf("1 %d\n",sym);
                return;
            }
        }
        else return;
    }
}

/*＜变量定义＞  ::= ＜类型标识符＞(＜标识符＞|＜标识符＞‘[’＜无符号整数＞‘]’){,(＜标识符＞|＜标识符＞‘[’＜无符号整数＞‘]’ )}  */
int vardef(FILE *IN)
{
    printf("\tVardef begin:\n");
    while(1)
    {
        //sym是左方括号或者逗号或者分号
        if(sym==semicolon)
        {
            //printf("\tVardef end\n");
            return;
        }
        sym=nextsym(IN);
        if(sym==identsym||sym==chartype)//标识符
        {
            sym=nextsym(IN);
            if(sym==lbracket)//左方括号，是数组
            {
                sym=nextsym(IN);
                if(sym==inttype)//无符号整数
                {
                    sym=nextsym(IN);
                    if(sym==rbracket)//右方括号
                    {
                        sym=nextsym(IN);
                        if(sym==comma)//逗号
                            continue;
                        else//分号
                        {
                            //printf("\tVardef end\n");
                            return;
                        }
                    }
                }
            }
            else if(sym==comma)//逗号
                continue;
            else
            {
               // printf("\tVardef end\n");
                return;
            }
        }
        else if(sym==inttype)//无符号整数
        {
            sym=nextsym(IN);
            if(sym==rbracket)//右方括号
            {
                sym=nextsym(IN);
                if(sym==comma)//逗号
                    continue;
                else//分号
                {
                    //printf("\tVardef end\n");
                    return;
                }
            }
        }
    }
}

/*区分变量声明和有返回值函数定义*/
int differ(FILE *IN)
{
    //读到了类型标识符
    while(1){
        sym=nextsym(IN);
        if(sym==identsym||sym==chartype)//标识符
        {
            sym=nextsym(IN);
            if(sym==lbracket||sym==comma||sym==semicolon)//左方括号或逗号或分号
            {
                vardec(IN);
            }
            else{
                //printf("test\n");
                //printf("%d\n",sym);
                if(vardecendflag==0){
                    printf("Vardec end\n");
                    vardecendflag=1;
                }
                if (sym==lbrace||sym==lparent)//左花括号或左括号
                {
                    retfuncdef(IN);
                }
            }
        }
        else if(sym==intsym||sym==charsym)
            continue;
        if(sym==voidsym||sym==rbrace)
            return;
        if(sym==ifsym||sym==whilesym||sym==lbrace||sym==scanfsym
           ||sym==printfsym||sym==switchsym||sym==returnsym
           ||sym==identsym||sym==chartype)
            return;
    }
}

/*＜有返回值函数定义＞  ::=  ＜声明头部＞‘(’＜参数＞‘)’ ‘{’＜复合语句＞‘}’
                            |＜声明头部＞‘{’＜复合语句＞‘}’ */
int retfuncdef(FILE *IN)
{
    while(1){
        printf("Return funcdef begin:\n");
        if(sym==lbrace)//左花括号
        {
            sym=nextsym(IN);
            statements(IN);//复合语句
            if(sym==rbrace)//右花括号
            {
                printf("Return fundef end:\n");
                return;
            }
        }
        else if(sym==lparent)//左括号
        {
            parameters(IN);//参数
            sym=nextsym(IN);
            if(sym==rparent)//右括号
            {
                sym=nextsym(IN);
                if(sym==lbrace)//左花括号
                {
                    sym=nextsym(IN);
                    statements(IN);//复合语句
                    if(sym==rbrace)//右花括号
                    {
                        printf("Return fundef end:\n");
                        return;
                    }
                }
            }
        }
    }
}

/*＜无返回值函数定义＞  ::= void＜标识符＞(’＜参数＞‘)’‘{’＜复合语句＞‘}’
                          | void＜标识符＞{’＜复合语句＞‘}’*/
int voidfuncdef(FILE *IN)
{
    //sym是标识符
    printf("Void fundef begin:\n");
    while(1)
    {
        if(sym==identsym||sym==chartype)//标识符
        {
            sym=nextsym(IN);
            if(sym==lparent)//左括号
            {
                parameters(IN);//参数
                sym=nextsym(IN);
                if(sym==rparent)//右括号
                {
                    sym=nextsym(IN);
                    if(sym==lbrace)//左花括号
                    {
                        sym=nextsym(IN);
                        statements(IN);//语句
                        if(sym==rbrace)//右花括号
                        {
                            printf("Void fundef end:\n");
                            return;
                        }
                    }
                }
            }
            else if(sym==lbrace)//左花括号
            {
                sym=nextsym(IN);
                statements(IN);//复合语句
                if(sym==rbrace)//右花括号
                {
                    printf("Void fundef end:\n");
                    return;
                }
            }
        }
    }
}

/*＜主函数＞  ::= void main‘(’‘)’‘{’＜复合语句＞‘}’ */
int mainfunc(FILE *IN)
{
    //sym现在是main
    printf("Mainfunc begin:\n");
    sym=nextsym(IN);
    if(sym==lparent)//左括号
    {
        sym=nextsym(IN);
        if(sym==rparent)//右括号
        {
            sym=nextsym(IN);
            //printf("%d\n",sym);
            if(sym==lbrace)//左花括号
            {
                sym=nextsym(IN);
                statements(IN);//复合语句
                //printf("%d\n",sym);
                //sym=nextsym(IN);
                if(sym==rbrace)//右花括号
                {
                    printf("Mainfunc end\n");
                    return;
                }
            }
        }
    }
}

/*＜复合语句＞   ::=  ［＜常量说明＞］［＜变量说明＞］｛＜语句＞｝*/
int statements(FILE *IN)
{
    printf("Statements begin:\n");
    //while(sym!=rbrace)//不是右花括号
    //{
    vardecbegflag=0;
    vardecendflag=0;
    //常量声明
    if(sym==constsym){
        constdec(IN);
    }
    //变量声明
    //printf("%d\n",sym);
    if(sym==intsym||sym==charsym)//类型标识符
        differ(IN);
    //}
    //语句
    statement(IN);
    printf("Statements end\n");
    if(sym==rbrace) return;
    else printf("mainerror\n");
    return;
}

/*＜语句＞ ::= ＜条件语句＞｜＜循环语句＞| ‘{’｛＜语句＞｝‘}’
            ｜＜有返回值函数调用语句＞; |＜无返回值函数调用语句＞;
            ｜＜赋值语句＞;｜＜读语句＞;｜＜写语句＞;｜＜空＞;
            |＜情况语句＞｜＜返回语句＞*/
int statement(FILE *IN)
{
    //printf("%d\n",sym);
    while(!feof(IN)){
        switch(sym)
        {
            case rbrace:
                //sym=nextsym(IN);
                return;
            case semicolon:
                sym=nextsym(IN);
                if(sym==rbrace)//花括号
                    return;
                else continue;
            case ifsym:
                ifstatement(IN);
                continue;
            case elsesym:
                return;
            case whilesym:
                whilestatement(IN);
                continue;
            case switchsym:
                switchstatement(IN);
                continue;
            case scanfsym:
                readstatement(IN);
                continue;
            case printfsym:
                writestatement(IN);
                continue;
            case returnsym:
                returnstatement(IN);
                continue;
            case lbrace:
                braceflag++;
                sym=nextsym(IN);
                statement(IN);
                if(sym==rbrace){
                    if(braceflag>0){
                        printf("statement end\n");
                       braceflag--;
                    }
                    sym=nextsym(IN);
                    continue;
                }
                else printf("errorlbarce\n");break;
            case identsym:
            case chartype:
                sym=nextsym(IN);
                if(sym==lparent)//左括号，为函数调用语句
                {
                    voidfuncuse(IN);
                    continue;
                }
                else if(sym==equmark||sym==lbracket)//等号或左方括号，为赋值语句
                {
                    //printf("%d\n",sym);
                    assignstatement(IN);
                    continue;
                }
                else
                    printf("statementerror\n");
                    continue;
            default:
                //printf("default %d\n",sym);
                continue;
        }
    }

}

/*＜赋值语句＞ ::= ＜标识符＞＝＜表达式＞|＜标识符＞‘[’＜表达式＞‘]’=＜表达式＞*/
int assignstatement(FILE *IN) //赋值语句
{
    printf("Assign statement\n");
    //此时sym是等号或左方括号
    if(sym==lbracket)//左方括号，数组赋值
    {
        expression(IN);//表达式
        sym=nextsym(IN);
        {
            if(sym==rbracket)//右方括号
            {
                sym=nextsym(IN);
                if(sym==equmark)//等号
                {
                    expression(IN);//表达式
                    sym=nextsym(IN);
                    sym=nextsym(IN);//分号
                    //printf("%d\n",sym);
                    return;
                }
            }
        }
    }
    else if(sym==equmark)//等号
    {
        expression(IN);//表达式
        sym=nextsym(IN);
        sym=nextsym(IN);//分号
        //printf("%d\n",sym);
        return;
    }

    return;
}

/*＜条件语句＞::= if ‘(’＜条件＞‘)’＜语句＞else＜语句＞*/
int ifstatement(FILE *IN) //情况语句
{
    printf("If statement begin:\n");
    //此时sym是if
    sym=nextsym(IN);
    if(sym==lparent)//左括号
    {
        condition(IN);//条件
        sym=nextsym(IN);
        if(sym==rparent)//右括号
        {
            sym=nextsym(IN);
            statement(IN);//语句
            if(sym==rbrace)
                sym=nextsym(IN);
            if(sym==elsesym)//else
            {
                printf("Else statement:\n");
                sym=nextsym(IN);
                statement(IN);//语句
                if(sym==rbrace){
                    //printf("If statement end\n");
                    return;
                }
                else printf("iferror\n");
            }
        }
    }

}

/*＜条件＞ ::=  ＜表达式＞＜关系运算符＞＜表达式＞｜＜表达式＞*/
int condition(FILE *IN)
{
    printf("\tcondition\n");
    //此时sym是左括号
    sym=nextsym(IN);
    expression(IN);//表达式
    sym=nextsym(IN);
    if(sym>=les&&sym<=equal)//关系运算符
    {
        sym=nextsym(IN);
        expression(IN);
    }
    return;
}

/*＜循环语句＞   ::=  while ‘(’＜条件＞‘)’＜语句＞*/
int whilestatement(FILE *IN) //循环语句
{
    printf("While statement begin:\n");
    //此时sym是while
    sym=nextsym(IN);
    if(sym==lparent)//左括号
    {
        condition(IN);//条件
        sym=nextsym(IN);
        if(sym==rparent)//右括号
        {
            sym=nextsym(IN);
            statement(IN);//语句
            if(sym==rbrace){
                //printf("While statement end\n");
                return;
            }
            else printf("whileerror\n");
        }
    }
}
int switchstatement(FILE *IN) //情况语句
{
    printf("Switch statement\n");
}
int casestatement(FILE *IN) //情况子语句
{
    printf("Case statement\n");
}
int defaultstatement(FILE *IN) //缺省
{
    printf("Default statement\n");
}
int retfuncuse(FILE *IN) //有返回值函数调用语句
{
    printf("Return funcuse\n");
}
int voidfuncuse(FILE *IN) //无返回值函数调用语句
{
    printf("Void funcuse\n");
}

/*＜读语句＞ ::=  scanf ‘(’＜标识符＞{,＜标识符＞}‘)’*/
int readstatement(FILE *IN) //读语句
{
    printf("Read statement\n");
    //此时sym是scanf
    sym=nextsym(IN);
    if(sym==lparent)//左括号
    {
        while(1)
        {
            sym=nextsym(IN);
            if(sym==identsym||sym==chartype)//标识符
            {
                sym=nextsym(IN);
                if(sym==comma)//下一个标识符
                    continue;
                else{//右括号
                    sym=nextsym(IN);//分号
                    return;
                }

            }
        }
    }
}
int writestatement(FILE *IN) //写语句
{
    printf("Write statement\n");
}
int returnstatement(FILE *IN) //返回语句
{
    printf("Return statement\n");
}


int parameters(FILE *IN)
{
    printf("Parameters\n");
}

int expression(FILE *IN)//表达式
{
    printf("\tExpression\n");
}
int item(FILE *IN) //项
{
    printf("\tItem\n");
}
int factor(FILE *IN) //因子
{
    printf("\tFactor\n");
}


int nextsym(FILE *IN)
{
    int t=0, i;
    memset(token, 0, sizeof(token)); //清空数组！
    ch=fgetc(IN);
    if(feof(IN)) return -1;
    if(isdigit(ch))//如果是数字
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
        return inttype;
    }
    else if (isalpha(ch)) //如果是字母
    {
        int type=-1;
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
            }
        }
    }
    else //是其他字符
    {
        switch(ch)
        {
            //跳过空白字符
            case '\n':
                return nextsym(IN);
            case ' ':
            case '\t':
                if (blankflag==0) return nextsym(IN);
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
            case '#':
            case '$':
            case '.':
            case '?':
            case '@':
            case '\\':
            case '^':
            case '_':
            case '`':
            case '|':
            case '~':
                return strtype;
               // return blank;
        }
    }
}

int main()
{
	FILE *IN, *OUT;
    char file_addr[100];
    char buffer[100];
    char temp;
    IN = fopen("C:\\Users\\Administrator\\Desktop\\15061091.txt","r");
    OUT = fopen("C:\\Users\\Administrator\\Desktop\\15061091_result.txt","w");
    //printf("%d",intsym);
    if(IN == NULL){
        printf("NO SUCH FILE!\n");
    }
    else{
        printf("Program begin:\n");
        while(!feof(IN))
        {
            sym=nextsym(IN);
            //printf("%d\t%d\n",No++,sym);
            if(sym==constsym)
                constdec(IN);
            if(sym==intsym||sym==charsym)//类型标识符
                differ(IN);
            if(sym==voidsym)
            {
                sym=nextsym(IN);
                if(sym!=mainsym)//无返回值函数
                    voidfuncdef(IN);
                else //主函数
                    mainfunc(IN);
            }
        }
    }
    return 0;
}
