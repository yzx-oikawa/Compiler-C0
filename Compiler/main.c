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
int blankflag=0; //0-�����ո� 1-�������ո�
int vardecbegflag=0;//1-����������ʼ
int vardecendflag=0;//1-������������
int braceflag=0;
char ch; //�ַ�
char token[100]; //�ַ���

int nextsym();
int constdec();//����˵��
int constdef();//��������
int vardec();//����˵��
int vardef(); //��������
int differ();
int retfuncdef(); //�з���ֵ��������
int voidfuncdef(); //�޷���ֵ��������
int parameters(); //����
int mainfunc(); //������
int expression(); //���ʽ
int item(); //��
int factor(); //����
int statements(); //�������
int statement();//���
int assignstatement(); //��ֵ���
int ifstatement(); //�������
int condition();//����
int whilestatement(); //ѭ�����
int switchstatement(); //������
int casestatement(); //��������
int defaultstatement(); //ȱʡ
int retfuncuse(); //�з���ֵ�����������
int voidfuncuse(); //�޷���ֵ�����������
int valuepara(); //ֵ������
int readstatement(); //�����
int writestatement(); //д���
int returnstatement(); //�������

/*������˵���� ::=  const���������壾;{ const���������壾;}*/
int constdec(FILE *IN)
{
    printf("Constdec begin:\n");
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
                printf("Constdec end\n");
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
    printf("\tConstdef begin:\n");
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
                        else//�ֺ�
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
                                sym=nextsym(IN);
                                if(sym==comma)//����
                                    continue;
                                else//�ֺ�
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

/*������˵����  ::= ���������壾;{���������壾;}*/
int vardec(FILE *IN)
{
    if(vardecbegflag==0){
        printf("Vardec begin:\n");
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

/*���������壾  ::= �����ͱ�ʶ����(����ʶ����|����ʶ������[�����޷�����������]��){,(����ʶ����|����ʶ������[�����޷�����������]�� )}  */
int vardef(FILE *IN)
{
    printf("\tVardef begin:\n");
    while(1)
    {
        //sym�������Ż��߶��Ż��߷ֺ�
        if(sym==semicolon)
        {
            //printf("\tVardef end\n");
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
                        else//�ֺ�
                        {
                            //printf("\tVardef end\n");
                            return;
                        }
                    }
                }
            }
            else if(sym==comma)//����
                continue;
            else
            {
               // printf("\tVardef end\n");
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
                    //printf("\tVardef end\n");
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
                //printf("test\n");
                //printf("%d\n",sym);
                if(vardecendflag==0){
                    printf("Vardec end\n");
                    vardecendflag=1;
                }
                if (sym==lbrace||sym==lparent)//�����Ż�������
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

/*���з���ֵ�������壾  ::=  ������ͷ������(������������)�� ��{����������䣾��}��
                            |������ͷ������{����������䣾��}�� */
int retfuncdef(FILE *IN)
{
    while(1){
        printf("Return funcdef begin:\n");
        if(sym==lbrace)//������
        {
            sym=nextsym(IN);
            statements(IN);//�������
            if(sym==rbrace)//�һ�����
            {
                printf("Return fundef end:\n");
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
                    sym=nextsym(IN);
                    statements(IN);//�������
                    if(sym==rbrace)//�һ�����
                    {
                        printf("Return fundef end:\n");
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
    printf("Void fundef begin:\n");
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
                        sym=nextsym(IN);
                        statements(IN);//���
                        if(sym==rbrace)//�һ�����
                        {
                            printf("Void fundef end:\n");
                            return;
                        }
                    }
                }
            }
            else if(sym==lbrace)//������
            {
                sym=nextsym(IN);
                statements(IN);//�������
                if(sym==rbrace)//�һ�����
                {
                    printf("Void fundef end:\n");
                    return;
                }
            }
        }
    }
}

/*����������  ::= void main��(����)����{����������䣾��}�� */
int mainfunc(FILE *IN)
{
    //sym������main
    printf("Mainfunc begin:\n");
    sym=nextsym(IN);
    if(sym==lparent)//������
    {
        sym=nextsym(IN);
        if(sym==rparent)//������
        {
            sym=nextsym(IN);
            //printf("%d\n",sym);
            if(sym==lbrace)//������
            {
                sym=nextsym(IN);
                statements(IN);//�������
                //printf("%d\n",sym);
                //sym=nextsym(IN);
                if(sym==rbrace)//�һ�����
                {
                    printf("Mainfunc end\n");
                    return;
                }
            }
        }
    }
}

/*��������䣾   ::=  �ۣ�����˵�����ݣۣ�����˵�����ݣ�����䣾��*/
int statements(FILE *IN)
{
    printf("Statements begin:\n");
    //while(sym!=rbrace)//�����һ�����
    //{
    vardecbegflag=0;
    vardecendflag=0;
    //��������
    if(sym==constsym){
        constdec(IN);
    }
    //��������
    //printf("%d\n",sym);
    if(sym==intsym||sym==charsym)//���ͱ�ʶ��
        differ(IN);
    //}
    //���
    statement(IN);
    printf("Statements end\n");
    if(sym==rbrace) return;
    else printf("mainerror\n");
    return;
}

/*����䣾 ::= ��������䣾����ѭ����䣾| ��{��������䣾����}��
            �����з���ֵ����������䣾; |���޷���ֵ����������䣾;
            ������ֵ��䣾;��������䣾;����д��䣾;�����գ�;
            |�������䣾����������䣾*/
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
                if(sym==rbrace)//������
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
                if(sym==lparent)//�����ţ�Ϊ�����������
                {
                    voidfuncuse(IN);
                    continue;
                }
                else if(sym==equmark||sym==lbracket)//�ȺŻ������ţ�Ϊ��ֵ���
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

/*����ֵ��䣾 ::= ����ʶ�����������ʽ��|����ʶ������[�������ʽ����]��=�����ʽ��*/
int assignstatement(FILE *IN) //��ֵ���
{
    printf("Assign statement\n");
    //��ʱsym�ǵȺŻ�������
    if(sym==lbracket)//�����ţ����鸳ֵ
    {
        expression(IN);//���ʽ
        sym=nextsym(IN);
        {
            if(sym==rbracket)//�ҷ�����
            {
                sym=nextsym(IN);
                if(sym==equmark)//�Ⱥ�
                {
                    expression(IN);//���ʽ
                    sym=nextsym(IN);
                    sym=nextsym(IN);//�ֺ�
                    //printf("%d\n",sym);
                    return;
                }
            }
        }
    }
    else if(sym==equmark)//�Ⱥ�
    {
        expression(IN);//���ʽ
        sym=nextsym(IN);
        sym=nextsym(IN);//�ֺ�
        //printf("%d\n",sym);
        return;
    }

    return;
}

/*��������䣾::= if ��(������������)������䣾else����䣾*/
int ifstatement(FILE *IN) //������
{
    printf("If statement begin:\n");
    //��ʱsym��if
    sym=nextsym(IN);
    if(sym==lparent)//������
    {
        condition(IN);//����
        sym=nextsym(IN);
        if(sym==rparent)//������
        {
            sym=nextsym(IN);
            statement(IN);//���
            if(sym==rbrace)
                sym=nextsym(IN);
            if(sym==elsesym)//else
            {
                printf("Else statement:\n");
                sym=nextsym(IN);
                statement(IN);//���
                if(sym==rbrace){
                    //printf("If statement end\n");
                    return;
                }
                else printf("iferror\n");
            }
        }
    }

}

/*�������� ::=  �����ʽ������ϵ������������ʽ���������ʽ��*/
int condition(FILE *IN)
{
    printf("\tcondition\n");
    //��ʱsym��������
    sym=nextsym(IN);
    expression(IN);//���ʽ
    sym=nextsym(IN);
    if(sym>=les&&sym<=equal)//��ϵ�����
    {
        sym=nextsym(IN);
        expression(IN);
    }
    return;
}

/*��ѭ����䣾   ::=  while ��(������������)������䣾*/
int whilestatement(FILE *IN) //ѭ�����
{
    printf("While statement begin:\n");
    //��ʱsym��while
    sym=nextsym(IN);
    if(sym==lparent)//������
    {
        condition(IN);//����
        sym=nextsym(IN);
        if(sym==rparent)//������
        {
            sym=nextsym(IN);
            statement(IN);//���
            if(sym==rbrace){
                //printf("While statement end\n");
                return;
            }
            else printf("whileerror\n");
        }
    }
}
int switchstatement(FILE *IN) //������
{
    printf("Switch statement\n");
}
int casestatement(FILE *IN) //��������
{
    printf("Case statement\n");
}
int defaultstatement(FILE *IN) //ȱʡ
{
    printf("Default statement\n");
}
int retfuncuse(FILE *IN) //�з���ֵ�����������
{
    printf("Return funcuse\n");
}
int voidfuncuse(FILE *IN) //�޷���ֵ�����������
{
    printf("Void funcuse\n");
}

/*������䣾 ::=  scanf ��(������ʶ����{,����ʶ����}��)��*/
int readstatement(FILE *IN) //�����
{
    printf("Read statement\n");
    //��ʱsym��scanf
    sym=nextsym(IN);
    if(sym==lparent)//������
    {
        while(1)
        {
            sym=nextsym(IN);
            if(sym==identsym||sym==chartype)//��ʶ��
            {
                sym=nextsym(IN);
                if(sym==comma)//��һ����ʶ��
                    continue;
                else{//������
                    sym=nextsym(IN);//�ֺ�
                    return;
                }

            }
        }
    }
}
int writestatement(FILE *IN) //д���
{
    printf("Write statement\n");
}
int returnstatement(FILE *IN) //�������
{
    printf("Return statement\n");
}


int parameters(FILE *IN)
{
    printf("Parameters\n");
}

int expression(FILE *IN)//���ʽ
{
    printf("\tExpression\n");
}
int item(FILE *IN) //��
{
    printf("\tItem\n");
}
int factor(FILE *IN) //����
{
    printf("\tFactor\n");
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
        while(isdigit(ch))
        {
            token[++t]=ch;
            ch=fgetc(IN);
        }
        if(!feof(IN)) //�����ļ����һ���ַ�Ϊ����ʱ���������ѭ��
            fseek(IN,-1L,SEEK_CUR);
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
            fseek(IN,-1L,SEEK_CUR); //��һ���ַ�
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
            if(sym==intsym||sym==charsym)//���ͱ�ʶ��
                differ(IN);
            if(sym==voidsym)
            {
                sym=nextsym(IN);
                if(sym!=mainsym)//�޷���ֵ����
                    voidfuncdef(IN);
                else //������
                    mainfunc(IN);
            }
        }
    }
    return 0;
}
