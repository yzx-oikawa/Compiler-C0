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
int blankflag=0; //0-�����ո� 1-�������ո�
int vardecbegflag=0;//1-����������ʼ
int vardecendflag=0;//1-������������
char ch; //�ַ�
char token[100]; //�ַ���

int nextsym();
int constdec();//����˵��
int constdef();//��������
int vardec();//����˵��
int vardef(); //��������
int differ();
int frufuncdef(); //�з���ֵ��������
int voidfuncdef(); //�޷���ֵ��������
int parameters(); //����
int mainfunc(); //������
int expression(); //���ʽ
int item(); //��
int factor(); //����
int statement(); //���
int assignstatement(); //��ֵ���
int ifstatement(); //������
int whilestatement(); //ѭ�����
int switchstatement(); //������
int casestatement(); //��������
int defaultstatement(); //ȱʡ
int frufuncuse(); //�з���ֵ�����������
int voidfuncuse(); //�޷���ֵ�����������
int valuepara(); //ֵ������
int readstatement(); //�����
int writestatement(); //д���
int returnstatement(); //�������

/*������˵���� ::=  const���������壾;{ const���������壾;}*/
int constdec(FILE *IN)
{
    printf("Constant declaration begin:\n");
    while(1)
    {
        constdef(IN);
        //sym=nextsym(IN);
        //printf("1%d\n",sym);
        if(sym==semicolon)//�ֺ�
        {
            sym=nextsym(IN);
            //printf("2%d\n",sym);
            if(sym==constsym)//��һ������˵��
                continue;
            else
            {
                printf("Constant declaration end\n");
                return;
            }
        }
        else return;
    }
}

/*���������壾   ::=   int����ʶ��������������{,����ʶ��������������}
                    | char����ʶ���������ַ���{,����ʶ���������ַ���}*/
int constdef(FILE *IN)
{
    printf("\tConstant definition begin:\n");
    sym=nextsym(IN);
    if(sym==intsym){
        while(1)
        {
            sym=nextsym(IN);
            if(sym==identsym||sym==chartype)//��ʶ��
            {
                //��¼���ű�
                sym=nextsym(IN);
                if(sym==equmark)//�Ⱥ�
                {
                    sym=nextsym(IN);
                    if(sym==inttype)//����
                    {
                        sym=nextsym(IN);
                        if(sym==comma)//����
                            continue;
                        else
                        {
                            printf("\tConstant definition end\n");
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
            if(sym==identsym||sym==chartype)//��ʶ��
            {
                //��¼���ű�
                sym=nextsym(IN);
                if(sym==equmark)//�Ⱥ�
                {
                    sym=nextsym(IN);
                    if(sym==sinquo)//������
                    {
                        sym=nextsym(IN);
                        if(sym==chartype)//�ַ�
                        {
                            sym=nextsym(IN);
                            if(sym==sinquo)//������
                            {
                                sym==nextsym(IN);
                                if(sym==comma)//����
                                    continue;
                                else
                                {
                                    printf("\tConstant definition end\n");
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

/*������˵����  ::= ���������壾;{���������壾;}*/
int vardec(FILE *IN)
{
    if(vardecbegflag==0){
        printf("Variable declaration begin:\n");
        vardecbegflag=1;
    }
    while(1)
    {
        //sym�������Ż��߶��Ż��߷ֺ�
        vardef(IN);
        if(sym==semicolon)//�ֺ�
        {
            sym=nextsym(IN);
            if(sym==intsym||sym==charsym)//��һ������˵�����ߺ�������
                return;
            else
            {
                if(vardecendflag==0){
                    printf("Variable declaration end\n");
                    vardecendflag=1;
                }
                return;
            }
        }
        else return;
    }
}

/*���������壾  ::= �����ͱ�ʶ����(����ʶ����|����ʶ������[�����޷�����������]��){,(����ʶ����|����ʶ������[�����޷�����������]�� )}  */
int vardef(FILE *IN)
{
    printf("\tVariable definition begin:\n");
    while(1)
    {
        //sym�������Ż��߶��Ż��߷ֺ�
        if(sym==semicolon)
        {
            printf("\tVariable definition end\n");
            return;
        }
        sym=nextsym(IN);
        if(sym==identsym||sym==chartype)//��ʶ��
        {
            sym=nextsym(IN);
            if(sym==lbracket)//�����ţ�������
            {
                sym=nextsym(IN);
                if(sym==inttype)//�޷�������
                {
                    sym=nextsym(IN);
                    if(sym==rbracket)//�ҷ�����
                    {
                        sym=nextsym(IN);
                        if(sym==comma)//����
                            continue;
                        else
                        {
                            printf("\tVariable definition end\n");
                            return;
                        }
                    }
                }
            }
            else if(sym==comma)//����
                continue;
            else
            {
                printf("\tVariable definition end\n");
                return;
            }
        }
        else if(sym==inttype)//�޷�������
        {
            sym=nextsym(IN);
            if(sym==rbracket)//�ҷ�����
            {
                sym=nextsym(IN);
                if(sym==comma)//����
                    continue;
                else//�ֺ�
                {
                    printf("\tVariable definition end\n");
                    return;
                }
            }
        }
    }
}

/*���ֱ����������з���ֵ��������*/
int differ(FILE *IN)
{
    //���������ͱ�ʶ��
    while(1){
        sym=nextsym(IN);
        if(sym==identsym||sym==chartype)//��ʶ��
        {
            sym=nextsym(IN);
            if(sym==lbracket||sym==comma||sym==semicolon)//�����Ż򶺺Ż�ֺ�
            {
                vardec(IN);
            }
            else{
                if(vardecendflag==0){
                    printf("Variable declaration end\n");
                    vardecendflag=1;
                }
                if (sym==lbrace||sym==lparent)//�����Ż�������
                {
                    frufuncdef(IN);
                }
            }
        }
        else if(sym==intsym||sym==charsym)
            continue;
        else if(sym==voidsym)
            return;
    }
}
/*���з���ֵ�������壾  ::=  ������ͷ������(������������)�� ��{����������䣾��}��
                            |������ͷ������{����������䣾��}�� */
int frufuncdef(FILE *IN)
{
    while(1){
        printf("Fruitful function definition begin:\n");
        if(sym==lbrace)//������
        {
            statement(IN);//���
            sym=nextsym(IN);
            if(sym==rbrace)//�һ�����
            {
                printf("Fruitful function definition end:\n");
                return;
            }
        }
        else if(sym==lparent)//������
        {
            parameters(IN);//����
            sym=nextsym(IN);
            if(sym==rparent)//������
            {
                sym=nextsym(IN);
                if(sym==lbrace)//������
                {
                    statement(IN);//���
                    sym=nextsym(IN);
                    if(sym==rbrace)//�һ�����
                    {
                        printf("Fruitful function definition end:\n");
                        return;
                    }
                }
            }
        }
    }
}
/*���޷���ֵ�������壾  ::= void����ʶ����(������������)����{����������䣾��}��
                          | void����ʶ����{����������䣾��}��*/
int voidfuncdef(FILE *IN)
{
    //sym�Ǳ�ʶ��
    printf("Void function definition begin:\n");
    while(1)
    {
        if(sym==identsym||sym==chartype)//��ʶ��
        {
            sym=nextsym(IN);
            if(sym==lparent)//������
            {
                parameters(IN);//����
                sym=nextsym(IN);
                if(sym==rparent)//������
                {
                    sym=nextsym(IN);
                    if(sym==lbrace)//������
                    {
                        statement(IN);//���
                        sym=nextsym(IN);
                        if(sym==rbrace)//�һ�����
                        {
                            printf("Void function definition end:\n");
                            return;
                        }
                    }
                }
            }
            else if(sym==lbrace)//������
            {
                statement(IN);//���
                sym=nextsym(IN);
                if(sym==rbrace)//�һ�����
                {
                    printf("Void function definition end:\n");
                    return;
                }
            }
        }
    }
}

int statement(FILE *IN)
{
    printf("\tstatement\n");
}
int parameters(FILE *IN)
{
    printf("\tparameters\n");
}
int nextsym(FILE *IN)
{
    int t=0, i;
    memset(token, 0, sizeof(token)); //������飡
    ch=fgetc(IN);
    if(feof(IN)) return -1;
    if(isdigit(ch))//���������
    {
        token[t]=ch;
        ch=fgetc(IN);
        //printf("%c\n",ch);
        while(isdigit(ch))
        {
            token[++t]=ch;
            ch=fgetc(IN);
        }
        if(!feof(IN)) //�����ļ����һ���ַ�Ϊ����ʱ���������ѭ��
            fseek(IN,-1L,SEEK_CUR);
        //fprintf(OUT,"%d\tINT\t",No++);
        //for(i=0;i<=t;i++)
         //   fprintf(OUT,"%c",token[i]);
        //fprintf(OUT,"INTTYPE\n");
        return inttype;
    }
    else if (isalpha(ch)) //�������ĸ
    {
        int type=-1;
        token[t]=tolower(ch); //ͳһ���Сд
        ch=fgetc(IN);
        while(isdigit(ch)|isalpha(ch))
        {
            token[++t]=tolower(ch);
            ch=fgetc(IN);
            continue;
        }
        if(!feof(IN)) //�����ļ����һ���ַ�Ϊ��ĸʱ���������ѭ��
            fseek(IN,-1L,SEEK_CUR);
        //�ж��Ƿ�Ϊ������
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
        else //�Ǳ�ʶ��
        {
            if(t==0) return chartype;
            else return identsym;
        }
        //for(i=0;i<=t;i++)
        //    fprintf(OUT,"%c",token[i]);
        //fprintf(OUT,"\n");
    }
    else if ((ch=='=')|(ch=='<')|(ch=='>')|(ch=='!')) //����ǱȽ������
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
            fseek(IN,-1L,SEEK_CUR); //��һ���ַ�
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
    else //�������ַ�
    {
        switch(ch)
        {
            //�����հ��ַ�
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
            if(sym==intsym||sym==charsym)//���ͱ�ʶ��
                differ(IN);
            if(sym==voidsym)
            {
                sym=nextsym(IN);
                if(sym!=mainsym)
                    voidfuncdef(IN);
                else;
            }
        }
    }
    return 0;
}
