#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define KEYNUM 14

const char *keyword[]={"const","int","char","void","main",
                       "if","else","while","switch","case",
                       "default","scanf","printf","return"};
const char *mnemonic[]={"CONST","INT","CHAR","VOID","MAIN",
                        "IF","ELSE","WHILE","SWITCH","CASE",
                        "DEFAULT","SCANF","PRINTF","RETURN"};

const enum symbol {add, sub, mult, divi, les, loe, mor, moe, noteq, equal,
                   comma, colon, semicolon, sinquo, douquo, equmark,
                   lparent, rparent, lbracket, rbracket, lbrace, rbrace,
                   //identsym, inttype, chartype, strtype, realtype,
                   constsym, intsym, charsym, voidsym, mainsym, ifsym, elsesym, whilesym,
                   switchsym, casesym, defaultsym, scanfsym, printfsym, returnsym};

int in; //整数
float fl; //实数
char ch; //字符
char token[100]; //字符串
void nextsym();
int No=1;

void nextsym(FILE *IN, FILE *OUT)
{
    int t=0, i;
    memset(token, 0, sizeof(token));
    ch=fgetc(IN);
    if(isdigit(ch))
    {
        int type=0;//0-int,1-float
        token[t]=ch;
        ch=fgetc(IN);
        //printf("%c\n",ch);
        while(isdigit(ch)|ch=='.')
        {
            if(isdigit(ch))
            {
                token[++t]=ch;
                ch=fgetc(IN);
                continue;
            }
            else if (ch=='.')
            {
                type=1;
                token[++t]=ch;
                ch=fgetc(IN);
                while (isdigit(ch))
                {
                    token[++t]=ch;
                    ch=fgetc(IN);
                    continue;
                }
                break;
            }
        }
        if(!feof(IN)) //否则文件最后一个字符为数字时会进入无限循环
            fseek(IN,-1L,SEEK_CUR);
        if(type==0)
            fprintf(OUT,"%d\tINT\t",No++);
        else
            fprintf(OUT,"%d\tFLOAT\t",No++);
        for(i=0;i<=t;i++)
            fprintf(OUT,"%c",token[i]);
        fprintf(OUT,"\n");
    }
    else if (isalpha(ch))
    {
        int type=-1;
        token[t]=ch;
        ch=fgetc(IN);
        while(isdigit(ch)|isalpha(ch))
        {
            token[++t]=ch;
            ch=fgetc(IN);
            continue;
        }
        if(!feof(IN)) //否则文件最后一个字符为字母时会进入无限循环
            fseek(IN,-1L,SEEK_CUR);
        //判断是否为保留字
        for(i=0;i<KEYNUM;i++)
        {
           // printf("%s=%s? ",token,keyword[i]);
            if(strcmp(token,keyword[i])==0)
                type=i;
        }
        if(type!=-1)
            fprintf(OUT,"%d\t%s\t",No++,mnemonic[type]);
        else
            fprintf(OUT,"%d\tIDENT\t",No++);
        for(i=0;i<=t;i++)
            fprintf(OUT,"%c",token[i]);
        fprintf(OUT,"\n");
    }
    else if ((ch=='=')|(ch=='<')|(ch=='>')|(ch=='!'))
    {
        token[t]=ch;
        ch=fgetc(IN);
        if(ch=='=')
        {
            token[++t]=ch;
            switch(token[0])
            {
                case '=':
                    fprintf(OUT,"%d\tEQUAL\t",No++); break;
                case '<':
                    fprintf(OUT,"%d\tLOE\t",No++); break;
                case '>':
                    fprintf(OUT,"%d\tMOE\t",No++); break;
                case '!':
                    fprintf(OUT,"%d\tNOTEQ\t",No++); break;
            }
            for(i=0;i<=t;i++)
                fprintf(OUT,"%c",token[i]);
            fprintf(OUT,"\n");
        }
        else
        {
            fseek(IN,-1L,SEEK_CUR); //退一个字符
            switch(token[0])
            {
                case '=':
                    fprintf(OUT,"%d\tEQUMARK\t",No++); break;
                case '<':
                    fprintf(OUT,"%d\tLES\t",No++); break;
                case '>':
                    fprintf(OUT,"%d\tMOR\t",No++); break;
            }
            if(token[0]=='!');
            else{
                for(i=0;i<=t;i++)
                    fprintf(OUT,"%c",token[i]);
                fprintf(OUT,"\n");
            }
        }

    }
    else
    {
        switch(ch)
        {
            //跳过空白字符
            case ' ':
            case '\n':
            case '\t':
                nextsym(IN,OUT); break;
            case '+':
                fprintf(OUT,"%d\tADD\t%c\n",No++,ch); break;
            case '-':
                fprintf(OUT,"%d\tSUB\t%c\n",No++,ch); break;
            case '*':
                fprintf(OUT,"%d\tMUL\t%c\n",No++,ch); break;
            case '/':
                fprintf(OUT,"%d\tDIV\t%c\n",No++,ch); break;
            case ',':
                fprintf(OUT,"%d\tCOMMA\t%c\n",No++,ch); break;
            case ':':
                fprintf(OUT,"%d\tCOLON\t%c\n",No++,ch); break;
            case ';':
                fprintf(OUT,"%d\tSEMICOL\t%c\n",No++,ch); break;
            case '\'':
                fprintf(OUT,"%d\tSINGUO\t%c\n",No++,ch); break;
            case '\"':
                fprintf(OUT,"%d\tDOUQUO\t%c\n",No++,ch); break;
            case '(':
                fprintf(OUT,"%d\tLPARENT\t%c\n",No++,ch); break;
            case ')':
                fprintf(OUT,"%d\tRPARENT\t%c\n",No++,ch); break;
            case '[':
                fprintf(OUT,"%d\tLBRACK\t%c\n",No++,ch); break;
            case ']':
                fprintf(OUT,"%d\tRBRACK\t%c\n",No++,ch); break;
            case '{':
                fprintf(OUT,"%d\tLBRACE\t%c\n",No++,ch); break;
            case '}':
                fprintf(OUT,"%d\tPBRACE\t%c\n",No++,ch); break;
        }
    }

}

int main()
{
	FILE *IN, *OUT;
    char file_addr[100];
    char buffer[100];
    int sym;
    char temp;
    IN = fopen("C:\\Users\\Administrator\\Desktop\\in.txt","r");
    OUT = fopen("C:\\Users\\Administrator\\Desktop\\out.txt","w");
    if(IN == NULL){
            printf("NO SUCH FILE!\n");
    }

    while(!feof(IN))
    {
        nextsym(IN,OUT);
    }
    return 0;
}
