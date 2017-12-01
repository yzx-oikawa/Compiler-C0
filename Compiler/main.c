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
                   identsym, inttype, chartype, strtype, blank};                         //36-39

int No=1;
int sym;
int blankflag=0; //0-跳过空格 1-不跳过空格
int vardecbegflag=0;//1-变量声明开始
int vardecendflag=0;//1-变量声明结束
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
int ifstatement(); //情况语句
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
           ||sym==identsym)
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
            if(sym==lbrace)//左花括号
            {
                sym=nextsym(IN);
                statements(IN);//复合语句
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

    printf("Statements end:\n");
}

/*＜语句＞ ::= ＜条件语句＞｜＜循环语句＞| ‘{’｛＜语句＞｝‘}’
            ｜＜有返回值函数调用语句＞; |＜无返回值函数调用语句＞;
            ｜＜赋值语句＞;｜＜读语句＞;｜＜写语句＞;｜＜空＞;
            |＜情况语句＞｜＜返回语句＞*/
int statement(FILE *IN)
{
    switch(sym)
    {
        case ifsym:
            ifstatement(IN);
            return;
        case whilesym:
            whilestatement(IN);
            return;
        case switchsym:
            switchstatement(IN);
            return;
        case scanfsym:
            readstatement(IN);
            return;
        case printfsym:
            writestatement(IN);
            return;
        case returnsym:
            returnstatement(IN);
            return;
        case lbrace:
            return;
        case identsym:
            sym=nextsym(IN);
            if(sym==lparent)//左括号，为函数调用语句
            {
                voidfuncuse(IN);
                return;
            }
            else if(sym==equmark)//等号，为赋值语句
            {
                assignstatement(IN);
                return;
            }
            else
                printf("error\n");
        default:
            //printf("default %d\n",sym);
            return;
    }

}

int assignstatement(FILE *IN) //赋值语句
{
    printf("\tAssign statement\n");
    return;
}
int ifstatement(FILE *IN) //情况语句
{
    printf("\tIf statement\n");
}
int whilestatement(FILE *IN) //循环语句
{
    printf("\tWhile statement\n");
}
int switchstatement(FILE *IN) //情况语句
{
    printf("\tSwitch statement\n");
}
int casestatement(FILE *IN) //情况子语句
{
    printf("\tCase statement\n");
}
int defaultstatement(FILE *IN) //缺省
{
    printf("\tDefault statement\n");
}
int retfuncuse(FILE *IN) //有返回值函数调用语句
{
    printf("\tReturn funcuse\n");
}
int voidfuncuse(FILE *IN) //无返回值函数调用语句
{
    printf("\tVoid funcuse\n");
}
int readstatement(FILE *IN) //读语句
{
    printf("\tRead statement\n");
}
int writestatement(FILE *IN) //写语句
{
    printf("\tWrite statement\n");
}
int returnstatement(FILE *IN) //返回语句
{
    printf("\tReturn statement\n");
}


int parameters(FILE *IN)
{
    printf("Parameters\n");
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
        //printf("%c\n",ch);
        while(isdigit(ch))
        {
            token[++t]=ch;
            ch=fgetc(IN);
        }
        if(!feof(IN)) //否则文件最后一个字符为数字时会进入无限循环
            fseek(IN,-1L,SEEK_CUR);
        //fprintf(OUT,"%d\tINT\t",No++);
        //for(i=0;i<=t;i++)
         //   fprintf(OUT,"%c",token[i]);
        //fprintf(OUT,"INTTYPE\n");
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
        //for(i=0;i<=t;i++)
        //    fprintf(OUT,"%c",token[i]);
        //fprintf(OUT,"\n");
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
                    //fprintf(OUT,"%d\tEQUAL\t",No++); break;
                    return equal;
                case '<':
                    //fprintf(OUT,"%d\tLOE\t",No++); break;
                    return loe;
                case '>':
                    //fprintf(OUT,"%d\tMOE\t",No++); break;
                    return moe;
                case '!':
                    //fprintf(OUT,"%d\tNOTEQ\t",No++); break;
                    return noteq;
            }
        //    for(i=0;i<=t;i++)
        //        fprintf(OUT,"%c",token[i]);
        //    fprintf(OUT,"\n");
        }
        else
        {
            fseek(IN,-1L,SEEK_CUR); //退一个字符
            switch(token[0])
            {
                case '=':
                    //fprintf(OUT,"%d\tEQUMARK\t",No++); break;
                    return equmark;
                case '<':
                    //fprintf(OUT,"%d\tLES\t",No++); break;
                    return les;
                case '>':
                    //fprintf(OUT,"%d\tMOR\t",No++); break;
                    return mor;
            }
            //if(token[0]=='!');
            //else{
            //    for(i=0;i<=t;i++)
            //        fprintf(OUT,"%c",token[i]);
            //    fprintf(OUT,"\n");
            //}
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
                //fprintf(OUT,"%d\tADD\t%c\n",No++,ch); break;
                return add;
            case '-':
                //fprintf(OUT,"%d\tSUB\t%c\n",No++,ch); break;
                return sub;
            case '*':
                //fprintf(OUT,"%d\tMUL\t%c\n",No++,ch); break;
                return mult;
            case '/':
                //fprintf(OUT,"%d\tDIV\t%c\n",No++,ch); break;
                return divi;
            case ',':
                //fprintf(OUT,"%d\tCOMMA\t%c\n",No++,ch); break;
                return comma;
            case ':':
                //fprintf(OUT,"%d\tCOLON\t%c\n",No++,ch); break;
                return colon;
            case ';':
                //fprintf(OUT,"%d\tSEMICOL\t%c\n",No++,ch); break;
                return semicolon;
            case '\'':
                //printf("%d\tSINGUO\t%c\n",No++,ch); //break;
                return sinquo;
            case '\"':
                //fprintf(OUT,"%d\tDOUQUO\t%c\n",No++,ch); break;
                return douquo;
            case '(':
                //fprintf(OUT,"%d\tLPARENT\t%c\n",No++,ch); break;
                return lparent;
            case ')':
                //fprintf(OUT,"%d\tRPARENT\t%c\n",No++,ch); break;
                return rparent;
            case '[':
                //fprintf(OUT,"%d\tLBRACK\t%c\n",No++,ch); break;
                return lbracket;
            case ']':
                //fprintf(OUT,"%d\tRBRACK\t%c\n",No++,ch); break;
                return rbracket;
            case '{':
                //fprintf(OUT,"%d\tLBRACE\t%c\n",No++,ch); break;
                return lbrace;
            case '}':
                //fprintf(OUT,"RBRACE\n");
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
