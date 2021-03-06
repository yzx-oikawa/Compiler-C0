#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAXIDENTLEN 20   //标识符最大长度
#define MAXSYMTABLEN 100 //符号表最大长度
#define MAXINTERCODE 10  //四元式每一项最大长度
#define MAXINTERLEN 500  //四元式序列最大长度
#define MAXASSEMLEN 10  //汇编代码每一项最大长度

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
int interaddr=0;//地址

/*用于表达式转四元式的返回字符串*/
int exprt=1; //四元式中的临时变量tn计数
int labelt=1; //四元式中的标签计数
int endofcase=1; //单独给switch语句结束的标签计数
char exprtemp[MAXIDENTLEN]; //exprt的字符串形式
char labtemp[MAXIDENTLEN]; //labelt的字符串形式
char exprret[MAXIDENTLEN]; //表达式返回的字符串
char factret[MAXIDENTLEN]; //因子返回的字符串
char itemret[MAXIDENTLEN]; //项返回的字符串
char strret[MAXIDENTLEN];  //写语句打印的字符串

int assems=0;
char assemtemp[MAXASSEMLEN]; //assems的字符串形式


FILE *IN, *OUT, *INTER, *ASSEM;
int No=1;

int blankflag=0; //0-跳过空格 1-不跳过空格
int vardecbegflag=0;//1-变量声明开始
int vardecendflag=1;//1-变量声明结束
int braceflag=0;
int logflag;

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
int checkiftn();//检查是否为四元式的中间变量并返回序号
int checkiflab();//检查是否为四元式的label并返回序号
void inter2assem();//四元式转汇编

void inter2assem()
{
    int tn;
    int immflag1, immflag2, immflag3;//立即数
    int addsub=0; //0-add 1-sub
    int muldiv=0; //0-mul 1-div
    char tabtemp[MAXASSEMLEN];
    char temp1[MAXASSEMLEN];
    char temp2[MAXASSEMLEN];
    char temp3[MAXASSEMLEN];
    //$a0为存标识符的开始地址
    //$a1为运算的开始地址
    //$a2存立即数
    //$t的寄存器存中间变量tn的值
    //$s的寄存器存标识符的值
    fprintf(ASSEM,".data\n.text\n");
    fprintf(ASSEM,"li $a0, 0x00000000\n");
    fprintf(ASSEM,"li $a1, 0x10000000\n");
    for(interi=1;interi<=interpc;interi++)
    {
        immflag1=0;
        immflag2=0;
        immflag3=0;
        memset(tabtemp, 0, sizeof(tabtemp));
        //常量定义
        if(strcmp(intercode[interi].inter0,"const")==0)
        {
            strcpy(token,intercode[interi].inter2);
            if(checkiflog()>=0)
            {
                fprintf(ASSEM,"li $s0,%d\n",symtab[tabi].value);
                fprintf(ASSEM,"sw ");
                fprintf(ASSEM,"$s0,%d($a0)\n",symtab[tabi].address);
            }

        }
        //变量定义 好像不用sw = =
        //变量赋值
        else if(strcmp(intercode[interi].inter0,"assign")==0)
        {
            //被赋的值
            strcpy(token,intercode[interi].inter1);
            if(checkiflog()>=0)//取标识符的值
            {
                fprintf(ASSEM,"lw ");
                //temp1保存被取到哪个寄存器
                strcpy(temp1,"$s");
                itoa(tabi,tabtemp,10);
                strcat(temp1,tabtemp);
                fprintf(ASSEM,"%s,%d($a0)\n",temp1,symtab[tabi].address);
            }
            else
            {
                tn=checkiftn();
                if(tn>0)//取中间变量的值
                {
                    fprintf(ASSEM,"lw ");
                    //temp1保存被取到哪个寄存器
                    strcpy(temp1,"$");
                    strcat(temp1,token);
                    fprintf(ASSEM,"%s,%d($a1)\n",temp1,4*tn);
                }
                else//被赋的值是立即数
                {
                    immflag1=1;
                    strcpy(temp1,token);
                }
            }
            //赋给的值
            strcpy(token,intercode[interi].inter3);
            if(checkiflog()>=0)//给标识符赋值
            {
                if(immflag1==1){
                    fprintf(ASSEM,"ori ");
                    immflag1=0;
                }
                else
                    fprintf(ASSEM,"or ");
                //temp2保存存到哪个寄存器
                strcpy(temp2,"$s");
                itoa(tabi,tabtemp,10);
                strcat(temp2,tabtemp);
                fprintf(ASSEM,"%s,$0,%s\n",temp2,temp1);
                //存到内存
                fprintf(ASSEM,"sw ");
                fprintf(ASSEM,"%s,%d($a0)\n",temp2,symtab[tabi].address);
            }
            else
            {
                tn=checkiftn();
                if(tn>0)//给中间变量赋值
                {
                    if(immflag1==1){
                        fprintf(ASSEM,"ori ");
                        immflag1=0;
                    }
                    else
                        fprintf(ASSEM,"or ");
                    //temp2保存存到哪个寄存器
                    strcpy(temp2,"$");
                    strcat(temp2,token);
                    fprintf(ASSEM,"%s,$0,%s\n",temp2,temp1);
                    //存到内存
                    fprintf(ASSEM,"sw ");
                    fprintf(ASSEM,"%s,%d($a1)\n",temp2,4*tn);
                }
            }
        }
        //加减法
        else if(strcmp(intercode[interi].inter0,"+")==0||strcmp(intercode[interi].inter0,"-")==0)
        {
            if(strcmp(intercode[interi].inter0,"-")==0) addsub=1;
            //第一个操作数
            strcpy(token,intercode[interi].inter1);
            if(checkiflog()>=0)//取标识符的值
            {
                fprintf(ASSEM,"lw ");
                //temp1保存被取到哪个寄存器
                strcpy(temp1,"$s");
                itoa(tabi,tabtemp,10);
                strcat(temp1,tabtemp);
                fprintf(ASSEM,"%s,%d($a0)\n",temp1,symtab[tabi].address);
            }
            else
            {
                tn=checkiftn();
                if(tn>0)//取中间变量的值
                {
                    fprintf(ASSEM,"lw ");
                    //temp1保存被取到哪个寄存器
                    strcpy(temp1,"$");
                    strcat(temp1,token);
                    fprintf(ASSEM,"%s,%d($a1)\n",temp1,4*tn);
                }
                else//被赋的值是立即数
                {
                    immflag1=1;
                    strcpy(temp1,token);
                }
            }
            //第二个操作数
            strcpy(token,intercode[interi].inter2);
            if(checkiflog()>=0)//取标识符的值
            {
                fprintf(ASSEM,"lw ");
                //temp2保存被取到哪个寄存器
                strcpy(temp2,"$s");
                itoa(tabi,tabtemp,10);
                strcat(temp2,tabtemp);
                fprintf(ASSEM,"%s,%d($a0)\n",temp2,symtab[tabi].address);
            }
            else
            {
                tn=checkiftn();
                if(tn>0)//取中间变量的值
                {
                    fprintf(ASSEM,"lw ");
                    //temp2保存被取到哪个寄存器
                    strcpy(temp2,"$");
                    strcat(temp2,token);
                    fprintf(ASSEM,"%s,%d($a1)\n",temp2,4*tn);
                }
                else//被赋的值是立即数
                {
                    immflag2=1;
                    strcpy(temp2,token);
                }
            }
            //结果
            strcpy(token,intercode[interi].inter3);
            if(checkiflog()>=0)//给标识符赋值
            {
                if(immflag1==1||immflag2==1){
                    if(addsub==0)
                        fprintf(ASSEM,"addi ");
                    else
                        fprintf(ASSEM,"subi ");
                    immflag1=0;
                    immflag2=0;
                }
                else{
                    if(addsub==0)
                        fprintf(ASSEM,"add ");
                    else
                        fprintf(ASSEM,"sub ");
                }
                //temp3保存存到哪个寄存器
                strcpy(temp3,"$s");
                itoa(tabi,tabtemp,10);
                strcat(temp3,tabtemp);
                fprintf(ASSEM,"%s,%s,%s\n",temp3,temp1,temp2);
                //存到内存
                fprintf(ASSEM,"sw ");
                fprintf(ASSEM,"%s,%d($a0)\n",temp3,symtab[tabi].address);
            }
            else
            {
                tn=checkiftn();
                if(tn>0)//给中间变量赋值
                {
                    if(immflag1==1||immflag2==1){
                        if(addsub==0)
                            fprintf(ASSEM,"addi ");
                        else
                            fprintf(ASSEM,"subi ");
                        immflag1=0;
                        immflag2=0;
                    }
                    else{
                        if(addsub==0)
                            fprintf(ASSEM,"add ");
                        else
                            fprintf(ASSEM,"sub ");
                    }
                    //temp2保存存到哪个寄存器
                    strcpy(temp3,"$");
                    strcat(temp3,token);
                    fprintf(ASSEM,"%s,%s,%s\n",temp3,temp1,temp2);
                    //存到内存
                    fprintf(ASSEM,"sw ");
                    fprintf(ASSEM,"%s,%d($a1)\n",temp3,4*tn);
                }
            }
        }
        //乘除法
        else if(strcmp(intercode[interi].inter0,"*")==0||strcmp(intercode[interi].inter0,"/")==0)
        {
            if(strcmp(intercode[interi].inter0,"/")==0) addsub=1;
            //第一个操作数
            strcpy(token,intercode[interi].inter1);
            if(checkiflog()>=0)//取标识符的值
            {
                fprintf(ASSEM,"lw ");
                //temp1保存被取到哪个寄存器
                strcpy(temp1,"$s");
                itoa(tabi,tabtemp,10);
                strcat(temp1,tabtemp);
                fprintf(ASSEM,"%s,%d($a0)\n",temp1,symtab[tabi].address);
            }
            else
            {
                tn=checkiftn();
                if(tn>0)//取中间变量的值
                {
                    fprintf(ASSEM,"lw ");
                    //temp1保存被取到哪个寄存器
                    strcpy(temp1,"$");
                    strcat(temp1,token);
                    fprintf(ASSEM,"%s,%d($a1)\n",temp1,4*tn);
                }
                else//被赋的值是立即数
                {
                    fprintf(ASSEM,"li ");
                    strcpy(temp1,"$a2");
                    fprintf(ASSEM,"%s,%s\n",temp1,token);
                }
            }
            //第二个操作数
            strcpy(token,intercode[interi].inter2);
            if(checkiflog()>=0)//取标识符的值
            {
                fprintf(ASSEM,"lw ");
                //temp2保存被取到哪个寄存器
                strcpy(temp2,"$s");
                itoa(tabi,tabtemp,10);
                strcat(temp2,tabtemp);
                fprintf(ASSEM,"%s,%d($a0)\n",temp2,symtab[tabi].address);
            }
            else
            {
                tn=checkiftn();
                if(tn>0)//取中间变量的值
                {
                    fprintf(ASSEM,"lw ");
                    //temp2保存被取到哪个寄存器
                    strcpy(temp2,"$");
                    strcat(temp2,token);
                    fprintf(ASSEM,"%s,%d($a1)\n",temp2,4*tn);
                }
                else//被赋的值是立即数
                {
                    fprintf(ASSEM,"li ");
                    strcpy(temp2,"$a3");
                    fprintf(ASSEM,"%s,%s\n",temp2,token);
                }
            }
            //乘除
            if(muldiv==0)
                fprintf(ASSEM,"mult ");
            else
                fprintf(ASSEM,"div ");
            fprintf(ASSEM,"%s,%s\n",temp1,temp2);
            //保存结果
            strcpy(token,intercode[interi].inter3);
            if(checkiflog()>=0)//给标识符赋值
            {
                //temp3保存存到哪个寄存器
                strcpy(temp3,"$s");
                itoa(tabi,tabtemp,10);
                strcat(temp3,tabtemp);
                fprintf(ASSEM,"mflo %s\n",temp3);
                //存到内存
                fprintf(ASSEM,"sw ");
                fprintf(ASSEM,"%s,%d($a0)\n",temp3,symtab[tabi].address);
            }
            else
            {
                tn=checkiftn();
                if(tn>0)//给中间变量赋值
                {
                    //temp2保存存到哪个寄存器
                    strcpy(temp3,"$");
                    strcat(temp3,token);
                    fprintf(ASSEM,"mflo %s\n",temp3);
                    //存到内存
                    fprintf(ASSEM,"sw ");
                    fprintf(ASSEM,"%s,%d($a1)\n",temp3,4*tn);
                }
            }
        }
        //Label:
        else if(strcmp(intercode[interi].inter1,":")==0)
        {
            fprintf(ASSEM,"%s:\n",intercode[interi].inter0);
        }
        //goto Label
        else if(strcmp(intercode[interi].inter0,"goto")==0)
        {
            fprintf(ASSEM,"j %s\n",intercode[interi].inter1);
        }
        //scanf
        /*li   $v0, 12 # input a
          syscall
          move $s1, $v0*/
        else if(strcmp(intercode[interi].inter0,"scan")==0)
        {
            strcpy(token,intercode[interi].inter1);
            if(checkiflog()>=0)//读值，存入标识符
            {
                fprintf(ASSEM,"li $v0,12\n");
                fprintf(ASSEM,"syscall\n");
                //temp1保存被取到哪个寄存器
                strcpy(temp1,"$s");
                itoa(tabi,tabtemp,10);
                strcat(temp1,tabtemp);
                fprintf(ASSEM,"move %s,$v0\n",temp1);
            }
        }
        //printf
        /*lw $a0, 4($s2)# output
          li  $v0, 11
          syscall
          la   $a0, messege # output
          li   $v0, 4
          syscall*/
        else if(strcmp(intercode[interi].inter0,"print")==0)
        {
            //打印字符串
            strcpy(token,intercode[interi].inter1);
            //printf("%d ",token[0]);
            //printf("%d\n",'\0');
            if(token[0]!='\0')
            {
                fprintf(ASSEM,"la $a0,%s\n",token);
                fprintf(ASSEM,"li $v0,4\n");
                fprintf(ASSEM,"syscall\n");
            }
            //打印表达式
            strcpy(token,intercode[interi].inter2);
            if(checkiflog()>=0)//打印标识符
            {
                fprintf(ASSEM,"lw $a0,%d($a0)\n",symtab[tabi].address);
                fprintf(ASSEM,"li $v0,11\n");
                fprintf(ASSEM,"syscall\n");
            }
            else
            {
                tn=checkiftn();
                if(tn>0)//打印中间变量
                {
                    fprintf(ASSEM,"lw $a0,%d($a1)\n",4*tn);
                    fprintf(ASSEM,"li $v0,11\n");
                    fprintf(ASSEM,"syscall\n");
                }
                else//被赋的值是立即数
                {
                    strcpy(temp2,token);
                    fprintf(ASSEM,"li $a0,%s\n",temp2);
                    fprintf(ASSEM,"li $v0,11\n");
                    fprintf(ASSEM,"syscall\n");
                }
            }
        }
    }
}

int checkiftn()//检查是否为四元式的中间变量并返回序号
{
    int i;
    int num=0;
    if(token[0]=='t')
    {
        for(i=1;i<=3;i++)
        {
            if(token[i]>='0'&&token[i]<='9')//是数字
                num=num*10+(token[i]-'0');
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
                else if(symtab[tabi].kind==3)
                    return 4;
            }
           }
    }
    return -1;//未登录
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
    int subflag=0;
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
				logflag=checkiflog();
                if(logflag==-1){
                    strcpy(symtab[++tabpc].name,token);
                    symtab[tabpc].kind=0;//常量
                    symtab[tabpc].type=1;//int
                    symtab[tabpc].address=interaddr;//地址
                    interaddr+=4;
                    symtab[tabpc].location=symloca;//函数位置
                    strcpy(intercode[++interpc].inter0,"const");
                    strcpy(intercode[interpc].inter1,"int");
                    strcpy(intercode[interpc].inter2,token);
                }
                else{
                    printf("%s DUPLICATE DEFINE\n",token);
                }
                sym=nextsym();
                if(sym==equmark)//等号
                {
                    sym=nextsym();
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
				logflag=checkiflog();
                if(logflag==-1){
                    strcpy(symtab[++tabpc].name,token);
                    symtab[tabpc].kind=0;//常量
                    symtab[tabpc].type=0;//char
                    symtab[tabpc].address=interaddr;//地址
                    interaddr+=4;
                    symtab[tabpc].location=symloca;//函数位置
                    strcpy(intercode[++interpc].inter0,"const");
                    strcpy(intercode[interpc].inter1,"char");
                    strcpy(intercode[interpc].inter2,token);
                }
                else{
                    printf("%d\n",logflag);
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
                            if(logflag==-1){
                                symtab[tabpc].value=token[0];//值
                                strcpy(intercode[interpc].inter3,token);
                            }
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
			logflag=checkiflog();
            if(logflag==-1){
                strcpy(symtab[++tabpc].name,token);
                symtab[tabpc].kind=1;//变量
                symtab[tabpc].type=symtype;
                symtab[tabpc].address=interaddr;//地址
                interaddr+=4;
                symtab[tabpc].location=symloca;//函数位置
                strcpy(intercode[++interpc].inter0,"var");
                strcpy(intercode[interpc].inter1,symtype==1?"int":"char");
                strcpy(intercode[interpc].inter2,token);
            }
            else{
                printf("%s DUPLICATE DEFINE\n",token);
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
                        interaddr+=4*(symnum-1);
                        strcpy(intercode[interpc].inter3,token);
                    }

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
            if(logflag==-1){
                symtab[tabpc].length=symnum;
                interaddr+=4*(symnum-1);
                strcpy(intercode[interpc].inter3,token);
            }
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
			logflag=checkiflog();
            if(logflag==-1){
                strcpy(symtab[++tabpc].name,token);
                symtab[tabpc].type=symtype;
                symtab[tabpc].address=interaddr;
                interaddr+=4;
                symtab[tabpc].location=symloca;//函数位置
                strcpy(intercode[++interpc].inter1,symtype==1?"int":"char");
                strcpy(intercode[interpc].inter2,token);

            }
            else{
                printf("%s DUPLICATE DEFINE\n",token);
                }
            sym=nextsym();
            if(sym==lbracket||sym==comma||sym==semicolon)//左方括号或逗号或分号
            {
                if(logflag==-1){
                    symtab[tabpc].kind=1;//变量
                    strcpy(intercode[interpc].inter0,"var");
                }
                vardec();
            }
            else{
                if(vardecendflag==0){
                    printf("Vardec end\n");
                    fprintf(OUT,"Vardec end\n");
                    vardecendflag=1;
                }
                if (sym==lbrace||sym==lparent){//左花括号或左括号
                    if(logflag==-1){
                        symtab[tabpc].kind=2;//函数
                        symtab[tabpc].value=1;//有返回值
                        symloca=tabpc+1;
                        symtab[tabpc].location=symloca;
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
			logflag=checkiflog();
            if(logflag==-1){
                strcpy(symtab[++tabpc].name,token);
                symtab[tabpc].kind=2;//函数
                symtab[tabpc].type=0;
                symtab[tabpc].address=interaddr;
                interaddr+=4;
                symtab[tabpc].value=0;//无返回值
                symtab[tabpc].location=symloca;//函数位置
                strcpy(intercode[++interpc].inter0,"func");
                strcpy(intercode[interpc].inter1,"void");
                strcpy(intercode[interpc].inter2,token);
            }
            else{
                printf("%s DUPLICATE DEFINE\n",token);
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
            //printf("%d\n",sym);
			logflag=checkiflog();
            if(sym==identsym||sym==chartype){//标识符
                if(logflag==-1){
                    strcpy(symtab[++tabpc].name,token);
                    symtab[tabpc].kind=3;//参数
                    symtab[tabpc].type=symtype;
                    symtab[tabpc].address=interaddr;
                    interaddr+=4;
                    symtab[tabpc].location=symloca;//函数位置
                    strcpy(intercode[++interpc].inter0,"para");
                    strcpy(intercode[interpc].inter1,symtype==1?"int":"char");
                    strcpy(intercode[interpc].inter2,token);
                }
                else{
                    printf("%s DUPLICATE DEFINE\n",token);
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
    printf("Mainfunc begin:\n");
    fprintf(OUT,"Mainfunc begin:\n");
    symloca=tabpc+1;
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
                braceflag++;
                sym=nextsym();
                statement();
                if(sym==rbrace){
                    braceflag--;
                    printf("statement end\n");
                    fprintf(OUT,"statement end\n");
                    sym=nextsym();
                    if(braceflag>0){
                        continue;
                    }
                    else {
                        return;
                    }
                }
                else printf("errorlbrace\n");break;
            case identsym:
            case chartype:
                //printf("%d",checkiflog());
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
    char assignident[MAXIDENTLEN];//被赋值的变量
    char assarrnum[MAXIDENTLEN];//数组长度
    printf("Assign statement\n");
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
    printf("If statement begin:\n");
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
            else
                statement();//语句
            strcpy(intercode[++interpc].inter0,"goto");
            itoa(labelt++,labtemp,10);
            strcpy(iflabel,labtemp);
            strcpy(intercode[interpc].inter1,"Label");
            strcat(intercode[interpc].inter1,labtemp);
            if(sym==rbrace)
                sym=nextsym();
            if(sym==elsesym)//else
            {
                printf("Else statement:\n");
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
                else{
                    statement();//语句
                    strcpy(intercode[++interpc].inter0,"Label");
                    strcat(intercode[interpc].inter0,iflabel);
                    strcpy(intercode[interpc].inter1,":");
                    if(sym==rbrace)
                        return;
                    else printf("iferror\n");
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
    printf("\tcondition\n");
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
    return labelt-1;
}

/*＜循环语句＞ ::= while ‘(’＜条件＞‘)’＜语句＞*/
int whilestatement() //循环语句
{
    char beginlabel[MAXIDENTLEN];//循环开始、goto的label
    char endlabel[MAXIDENTLEN];//循环结束、bz的label
    printf("While statement begin:\n");
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
            sym=nextsym();
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
            else{
                statement();//语句
                strcpy(intercode[++interpc].inter0,"goto");
                strcpy(intercode[interpc].inter1,"Label");
                strcat(intercode[interpc].inter1,beginlabel);
                //循环结束
                strcpy(intercode[++interpc].inter0,"Label");
                strcat(intercode[interpc].inter0,endlabel);
                strcpy(intercode[interpc].inter1,":");
                if(sym==rbrace)
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
    printf("Switch statement:\n");
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
    char caselabel[MAXIDENTLEN];
    char endlabel[MAXIDENTLEN];
    itoa(endofcase++,endlabel,10);
    while(1)
    {
        strcpy(intercode[++interpc].inter0,"==");
        strcpy(intercode[interpc].inter1,switchlabel);
        sym=nextsym();
        if(sym==sinquo||sym==inttype||sym==numtype)//单引号括起来的字母或数字，或者整数
        {
            if(constforcase()==1){//常量
                printf("Case statement\n");
                fprintf(OUT,"Case statement\n");
            }
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
                else{
                    statement();//语句 可能返回右花括号、case、default
                }
                //结束switch
                strcpy(intercode[++interpc].inter0,"goto");
                strcpy(intercode[interpc].inter1,"Labeleoc");
                strcat(intercode[interpc].inter1,endlabel);

                //下一个case开始
                strcpy(intercode[++interpc].inter0,"Label");
                strcat(intercode[interpc].inter0,caselabel);
                strcpy(intercode[interpc].inter1,":");
                if(sym==casesym)//下一个case
                    continue;
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
    printf("Default statement\n");
    fprintf(OUT,"Default statement\n");
    //sym此时为default
    sym=nextsym();
    {
        if(sym==colon)//冒号
        {
            sym=nextsym();
            if(sym==semicolon)//语句为空
                sym=nextsym();//返回右花括号
            else
                statement();//语句
        }
    }
    return;
}

/*＜有返回值函数调用语句＞ ::= ＜标识符＞‘(’＜值参数表＞‘)’|<标识符>*/
int retfuncuse() //有返回值函数调用语句
{
    printf("Return funcuse\n");
    fprintf(OUT,"Return funcuse\n");
    //sym此时为左括号或分号
    strcpy(intercode[++interpc].inter0,"call");
    strcpy(intercode[interpc].inter1,ident);
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
    strcpy(intercode[++interpc].inter0,"call");
    strcpy(intercode[interpc].inter1,ident);
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
    printf("Return statement\n");
    fprintf(OUT,"Return statement\n");
    sym=nextsym();
    //sym此时为return
    if(sym==lparent)//左括号
    {
		sym=nextsym();
        expression();
        strcpy(intercode[++interpc].inter0,"ret");
        strcpy(intercode[interpc].inter1,exprret);
        if(sym==rparent){//右括号
            sym=nextsym();
            return;
        }
    }
    else{//分号
        strcpy(intercode[++interpc].inter0,"ret");
        return;
    }
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
    memset(strret, 0, sizeof(strret));
    printf("\tString\n");
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
    return;//返回了双引号
}

/*＜表达式＞ ::= ［+｜-］＜项＞{＜加法运算符＞＜项＞}*/
int expression()//表达式
{
    char expr[3][MAXIDENTLEN];
    int exprpc=1;
    printf("\tExpression begin\n");
    fprintf(OUT,"\tExpression begin\n");
    if(sym==add||sym==sub){
        if(sym==sub){
            sym=nextsym();
            strcpy(intercode[++interpc].inter0,"oppo");
            strcpy(intercode[interpc].inter3,ident);
        }
        else sym=nextsym();
    }
    while(1)
    {
        item();
        strcpy(expr[exprpc],itemret);
        if(sym==add||sym==sub){
            if(exprpc==1) exprpc=2;
            else if(exprpc==2) exprpc=1;
            if(exprpc==1)
            {
                strcpy(intercode[++interpc].inter0,expr[0]);
                strcpy(intercode[interpc].inter1,expr[1]);
                strcpy(intercode[interpc].inter2,expr[2]);
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
                itoa(exprt++,exprtemp,10);
                strcpy(intercode[interpc].inter3,"t");
                strcat(intercode[interpc].inter3,exprtemp);
                strcpy(expr[1],intercode[interpc].inter3);
            }
            strcpy(exprret,expr[1]);
            printf("\tExpression end\n");
            fprintf(OUT,"\tExpression end\n");
            return;
        }
    }
}

/*＜项＞ ::= ＜因子＞{＜乘法运算符＞＜因子＞}*/
int item() //项
{
    char ite[3][MAXIDENTLEN];
    int itepc=1;
    printf("\tItem\n");
    fprintf(OUT,"\tItem\n");
    while(1)
    {
        factor();
        strcpy(ite[itepc],factret);
        if(sym==mult||sym==divi){
            if(itepc==1) itepc=2;
            else if(itepc==2) itepc=1;
            if(itepc==1)
            {
                strcpy(intercode[++interpc].inter0,ite[0]);
                strcpy(intercode[interpc].inter1,ite[1]);
                strcpy(intercode[interpc].inter2,ite[2]);
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
                itoa(exprt++,exprtemp,10);
                strcpy(intercode[interpc].inter3,"t");
                strcat(intercode[interpc].inter3,exprtemp);
                strcpy(ite[1],intercode[interpc].inter3);
            }
            strcpy(itemret,ite[1]);
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
    printf("\tFactor\n");
    fprintf(OUT,"\tFactor\n");
    switch(sym)
    {
        //printf("%d\n",sym);
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
                    itoa(exprt++,exprtemp,10);
                    strcpy(intercode[interpc].inter3,"t");
                    strcat(intercode[interpc].inter3,exprtemp);
                    strcpy(factret,intercode[interpc].inter3);
                    return;
                }
            }
            else if(sym==lparent)//左括号或分号，有返回值函数调用
            {
                retfuncuse();//返回右括号
                sym=nextsym();
                strcpy(factret,"ret");
                return;//
            }
            else{//标识符
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
        case sub:
        case add://整数前正负号
            sym=nextsym();
            if(sym==inttype||sym==numtype){
                sym=nextsym();
                strcpy(factret,ident);
                return;
            }
            break;
        case inttype:
        case numtype://整数
            sym=nextsym();
            strcpy(factret,ident);
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
    INTER = fopen("inter.txt","w");
    ASSEM = fopen("assem.txt","w");
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

        //打印符号表
        printf("\nname\tkind\ttype\tvalue\taddr\tlen\tloca\n");
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
