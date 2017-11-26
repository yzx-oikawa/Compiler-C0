program PASCALS(INPUT,OUTPUT,PRD,PRR);
{  author:N.Wirth, E.T.H. CH-8092 Zurich,1.3.76 }
{  modified by R.E.Berry
    Department of computer studies
    University of Lancaster

    Variants of this program are used on
    Data General Nova,Apple,and
    Western Digital Microengine machines. }
{   further modified by M.Z.Jin
    Department of Computer Science&Engineering BUAA,0ct.1989
}
const nkw = 27;    { no. of key words }
      alng = 10;   { no. of significant chars in identifiers }
      llng = 121;  { input line length }
      emax = 322;  { max exponent of real numbers }
      emin = -292; { min exponent }
      kmax = 15;   { max no. of significant digits }
      tmax = 100;  { size of table }
      bmax = 20;   { size of block-table }
      amax = 30;   { size of array-table }
      c2max = 20;  { size of real constant table }
      csmax = 30;  { max no. of cases }
      cmax = 800;  { size of code }
      lmax = 7;    { maximum level }
      smax = 600;  { size of string-table }
      ermax = 58;  { max error no. }
      omax = 63;   { highest order code }
      xmax = 32767;  { 2**15-1 }
      nmax = 32767;  { 2**15-1 }
      lineleng = 132; { output line length }
      linelimit = 200;
      stacksize = 1450;
type symbol = ( intcon, realcon, charcon, stringcon,
                notsy, plus, minus, times, idiv, rdiv, imod, andsy, orsy,
                eql, neq, gtr, geq, lss, leq,
                lparent, rparent, lbrack, rbrack, comma, semicolon, period,
                colon, becomes, constsy, typesy, varsy, funcsy,
                procsy, arraysy, recordsy, programsy, ident,
                beginsy, ifsy, casesy, repeatsy, whilesy, forsy,
                endsy, elsesy, untilsy, ofsy, dosy, tosy, downtosy, thensy);
     index = -xmax..+xmax;
     alfa = packed array[1..alng]of char;
     objecttyp = (konstant, vvariable, typel, prozedure, funktion ); 
     types = (notyp, ints, reals, bools, chars, arrays, records );
     symset = set of symbol;
     typset = set of types;
     item = record
               typ: types;
               ref: index;
            end;

     order = packed record
                f: -omax..+omax;
				x: -lmax..+lmax;
                y: -nmax..+nmax
end;
var   ch:         char; { last character read from source program }
      rnum:       real; { real number from insymbol }
	  inum:       integer;     { integer from insymbol }
      sleng:      integer;     { string length }
      cc:         integer;     { character counter }
      lc:         integer;     { program location counter }
      ll:         integer;     { length of current line }
      errpos:     integer;
      t,a,b,sx,c1,c2:integer;  { indices to tables }
      iflag, oflag, skipflag, stackdump, prtables: boolean;
      sy:         symbol;      { last symbol read by insymbol }
      errs:       set of 0..ermax; {错误信息集合}
      id:         alfa;        { identifier from insymbol }
      progname:   alfa;
      stantyps:   typset;
      constbegsys, typebegsys, blockbegsys, facbegsys, statbegsys: symset;
      line:       array[1..llng] of char;
      key:        array[1..nkw] of alfa;
      ksy:        array[1..nkw] of symbol;
      sps:        array[char]of symbol;  { special symbols }
      display:    array[0..lmax] of integer; {按静态层次存放了各分程序在运行栈中分配的数据区的地址}
      tab:        array[0..tmax] of      { indentifier table 符号表}
                 packed record
                     name: alfa; {标识符名字，取前10个字符}
                     link: index; {同一分程序中上一个标识符在表中的位置，第一个标识符的link域值为0}
                     obj:  objecttyp; {标识符种类}
                     typ:  types; {标识符类型}
                     ref:  index; {数组信息向量表atab中的指针值或分程序表btab中的指针值}
					 normal: boolean; {标识符为变量形参时填入false，其他true}
                     lev:  0..lmax; {标识符所在分程序的静态层次，规定主程序的层次为1}
                     adr: integer  {对于变量：在运行栈S中分配存储单元的相对地址
									对于记录域名：相对记录变量起始地址的位移量
									对于过程或函数名：相应目标代码的入口地址
									对于整数、布尔或字符常量名：相应值
									对于实常量名：在实常量表rconst中的指针值
									对于类型名：所需存储单元的数目(即大小)}
end;
     atab:       array[1..amax] of    { array-table  数组信息向量表}
                 packed record
                     inxtyp,eltyp: types;
                     elref,low,high,elsize,size: index
                 end;
     btab:       array[1..bmax] of    { block-table 分程序表}
                 packed record
                     last, {指向分程序中说明的当前(最后)一个标识符在tab表中的位置}
					 lastpar, {指向过程或函数的最后一个参数在tab表中的位置}
					 psize, {参数及该分程序在运行栈S中的内务信息去所占的存储单元数}
					 vsize: index {局部变量、参数及内务信息区在S栈中所占的存储单元总数}
                 end;
     stab:       packed array[0..smax] of char; { string table 字符串常量表}
     rconst:     array[1..c2max] of real;  {实常量表}
     code:       array[0..cmax] of order;  {P代码表}
     psin,psout,prr,prd:text;      { default in pascal p }
     inf, outf, fprr: string;

procedure errormsg; {打印被编译源程序中出错信息的摘要}
  var k : integer;
     msg: array[0..ermax] of alfa;
  begin
    msg[0] := 'undef id  ';    msg[1] := 'multi def ';
    msg[2] := 'identifier';    msg[3] := 'program   ';
	msg[4] := ')         ';    msg[5] := ':         ';
    msg[6] := 'syntax    ';    msg[7] := 'ident,var '; 
	msg[8] := 'of        ';    msg[9] := '(         ';
    msg[10] := 'id,array  ';    msg[11] := '(         ';
    msg[12] := ']         ';    msg[13] := '..        ';
    msg[14] := ';         ';    msg[15] := 'func. type';
    msg[16] := '=         ';    msg[17] := 'boolean   ';
    msg[18] := 'convar typ';    msg[19] := 'type      ';
    msg[20] := 'prog.param';    msg[21] := 'too big   ';
    msg[22] := '.         ';    msg[23] := 'type(case)';
    msg[24] := 'character ';    msg[25] := 'const id  ';
    msg[26] := 'index type';    msg[27] := 'indexbound';
    msg[28] := 'no array  ';    msg[29] := 'type id   ';
    msg[30] := 'undef type';    msg[31] := 'no record ';
    msg[32] := 'boole type';    msg[33] := 'arith type';
	msg[34] := 'integer   ';    msg[35] := 'types     ';
    msg[36] := 'param type';    msg[37] := 'variab id ';
	msg[38] := 'string    ';    msg[39] := 'no.of pars';
    msg[40] := 'real numbr';    msg[41] := 'type      ';
	msg[42] := 'real type ';    msg[43] := 'integer   ';
    msg[44] := 'var,const ';    msg[45] := 'var,proc  ';
	msg[46] := 'types(:=) ';    msg[47] := 'typ(case) ';
    msg[48] := 'type      ';    msg[49] := 'store ovfl';
    msg[50] := 'constant  ';    msg[51] := ':=        ';
    msg[52] := 'then      ';    msg[53] := 'until     ';
    msg[54] := 'do        ';    msg[55] := 'to downto ';
    msg[56] := 'begin     ';    msg[57] := 'end       ';
    msg[58] := 'factor';

    writeln(psout);
    writeln(psout,'key words');
    k := 0;
    while errs <> [] do {如果错误集合非空}
      begin
        while not( k in errs )do {逐个输出错误集合中的错误信息}
		k := k + 1;
		writeln(psout, k, ' ', msg[k] );
        errs := errs - [k] {将错误从集合中删去}
	  end { while errs }
  end { errormsg } ;

procedure endskip; {源程序出错后在被跳读的部分下面印出下划线标志}
  begin                 { underline skipped part of input }
    while errpos < cc do {使下划线与程序长度一致 errpos:错误位置}
      begin
        write( psout, '-');
        errpos := errpos + 1
      end;
    skipflag := false
  end { endskip };

procedure nextch;  {读取下一字符，处理行结束符，印出被编译的源程序}
  begin 			  { read next character; process line end }
    if cc = ll {行指针位于行末}
    then begin
           if eof( psin ) {已读完编译文件}	
           then begin
                  writeln( psout );
                  writeln( psout, 'program incomplete' );
                  errormsg; {打印错误信息摘要}
                  exit;
                end;
           if errpos <> 0
           then begin
                  if skipflag then endskip; {如果有跳读，则在被跳读的部分下面打印下划线}
                  writeln( psout );
                  errpos := 0
                end;
           write( psout, lc: 5, ' '); {打印当前行数}
           ll := 0;
           cc := 0;
           while not eoln( psin ) do {没有读完当前行}
             begin
               ll := ll + 1; {当前行长度++}
               read( psin, ch ); {读一个字符}
               write( psout, ch ); {写一个字符}
               line[ll] := ch {将字符保存在line数组中}
             end; {读完了一行；下面处理回车}
           ll := ll + 1; {当前行长度++}
           readln( psin ); {读回车}
           line[ll] := ' '; {一行的末尾置为空格}
           writeln( psout ); {写回车}
         end;
         cc := cc + 1; {行指针前移}
         ch := line[cc]; {取字符}
  end { nextch };

procedure error( n: integer ); {打印出错位置和出错编号}
begin
  if errpos = 0
  then write ( psout, '****' );
  if cc > errpos
  then begin
         write( psout, ' ': cc-errpos, '^', n:2);
		 errpos := cc + 3;
         errs := errs +[n]
       end
end { error };

procedure fatal( n: integer ); {打印表格溢出信息}
  var msg : array[1..7] of alfa;
  begin
    writeln( psout );
    errormsg;
    msg[1] := 'identifier';   msg[2] := 'procedures';
    msg[3] := 'reals     ';   msg[4] := 'arrays    ';
    msg[5] := 'levels    ';   msg[6] := 'code      ';
    msg[7] := 'strings   ';
	writeln( psout, 'compiler table for ', msg[n], ' is too small');
    exit; {terminate compilation }
  end { fatal };

procedure insymbol;  {reads next symbol} {读取下一单词符号，处理注释行}
label 1,2,3;
  var  i,j,k,e: integer;
  procedure readscale; {处理实数的指数部分}
    var s,sign: integer;
    begin
      nextch; {读下一字符}
      sign := 1; {正负号}
      s := 0;
      if ch = '+' {字符为'+'，正数}
      then nextch
      else if ch = '-' {字符为'-'，负数}
           then begin
                  nextch;
                  sign := -1
                end;
      if not(( ch >= '0' )and (ch <= '9' )) {如果字符不是数字，报错}
      then error(40) {报错：小数点后没有数字}
      else repeat {循环直到读完数字}
           s := 10*s + ord( ord(ch)-ord('0')); {计算指数的数值，前一位乘10加上后一位(ord:将字符转换成ASCII值)}
           nextch;
           until not(( ch >= '0' ) and ( ch <= '9' )); 
      e := s*sign + e {将指数与先前计算的指数e相加}
    end { readscale };

  procedure adjustscale; {根据小数位数和指数大小求出实数数值}
    var s : integer;
        d, t : real;
    begin
      if k + e > emax {位数大于实数范围上限,报错}
      then error(21) {报错：数太大，最多为14位数字，其绝对值不超过10**323}
      else if k + e < emin  {位数小于实数范围下限}
           then rnum := 0 {趋近于0}
           else begin
                  s := abs(e);
                  t := 1.0;
                  d := 10.0;
                  repeat {循环直到s=0}
                    while not odd(s) do {s为偶数}
                      begin
                        s := s div 2; {s=s/2}
                        d := sqr(d)  {d=d^2}
                      end; {s为奇数}
                    s := s - 1;
                    t := d * t
                  until s = 0;
				  if e >= 0  {根据指数的正负进行乘或除，求出实数数值}
                  then rnum := rnum * t
                  else rnum := rnum / t
			end
    end { adjustscale };

  procedure options; {处理编译时的可选项}
    procedure switch( var b: boolean ); {处理编译可选项中的'+''-'标志}
      begin
        b := ch = '+';
        if not b
        then if not( ch = '-' ) {如果既不是'+'也不是'-'，则一直读字符直到遇到'*'或','}
             then begin { print error message }
                    while( ch <> '*' ) and ( ch <> ',' ) do
                      nextch;
                  end
             else nextch
        else nextch
      end { switch };
    begin { options  }
      repeat {一直读字符直到遇到','}
        nextch;
        if ch <> '*'
        then begin
               if ch = 't' {可选项t}
               then begin
                      nextch;
                      switch( prtables )
                    end
               else if ch = 's' {可选项s}
                  then begin
                          nextch;
                          switch( stackdump )
                       end;
             end
      until ch <> ','
    end { options };
  
  begin { insymbol  }
  1: while( ch = ' ' ) or ( ch = chr(9) ) do {如果字符为空格或\t，读下一个字符(chr:将ASCII值转换成字符)}
       nextch;    { space & htab }
    case ch of
      'a','b','c','d','e','f','g','h','i',
      'j','k','l','m','n','o','p','q','r',
      's','t','u','v','w','x','y','z':
        begin { identifier of wordsymbol 字符串}
          k := 0; {字符串长度}
          id := '          ';
          repeat {一直读字符直到下一个字符既不是数字也不是字母}
            if k < alng {字符串长度小于id最大长度(10)}
            then begin
                   k := k + 1;
                   id[k] := ch
                 end;
            nextch
          until not((( ch >= 'a' ) and ( ch <= 'z' )) or (( ch >= '0') and (ch <= '9' )));
          i := 1;
          j := nkw; { binary search } {27}
          repeat {用二分法判断id是否为关键字}
            k := ( i + j ) div 2; {14}
            if id <= key[k]
            then j := k - 1;
            if id >= key[k]
            then i := k + 1;
          until i > j;
          if i - 1 > j {id是关键字}
          then sy := ksy[k] {修改其类型为相应关键字}
          else sy := ident {修改其类型为ident}
        end;
      '0','1','2','3','4','5','6','7','8','9':
        begin { number 数字}
          k := 0; {数字位数}
          inum := 0;
          sy := intcon; {修改类型为整型常量}
		  repeat
            inum := inum * 10 + ord(ch) - ord('0'); {计算整数部分数值}
			k := k + 1; 
            nextch
          until not (( ch >= '0' ) and ( ch <= '9' ));
          if( k > kmax ) or ( inum > nmax ) {数值超出范围，报错}
          then begin
                 error(21); {报错：数太大，最多为14位数字，其绝对值不超过10**323}
                 inum := 0;
                 k := 0
               end;
          if ch = '.' {存在小数部分}
          then begin
                 nextch;
                 if ch = '.'
                 then ch := ':' {..→:}
                 else begin
                        sy := realcon; {修改类型为实数常量}
                        rnum := inum; {将整数部分的数值赋给rnum}
                        e := 0; {小数位数}
                        while ( ch >= '0' ) and ( ch <= '9' ) do
						  begin
                            e := e - 1; 
                            rnum := 10.0 * rnum + (ord(ch) - ord('0')); {计算小数部分数值}
							nextch
                          end;
                        if e = 0 {小数点后面无数字，报错}
                        then error(40);
                        if ch = 'e' {存在指数部分}
                        then readscale; {将实数中的指数与因为小数而产生的指数相加}
                        if e <> 0 then adjustscale {根据小数位数和指数大小求出实数数值}
                      end
                end
          else if ch = 'e' {存在指数部分}
               then begin
					  sy := realcon;
                      rnum := inum;
                      e := 0;
					  readscale; {计算指数}
                      if e <> 0
                      then adjustscale {根据指数大小求出实数数值}
                    end;
        end;
      ':':
        begin
          nextch;
          if ch = '='
          then begin
                 sy := becomes; {修改类型为赋值语句}
                 nextch
               end
          else  sy := colon {修改类型为冒号}
         end;
      '<':
        begin
          nextch;
          if ch = '='
          then begin
                 sy := leq; {修改类型为小于等于号}
                 nextch
               end
          else
            if ch = '>'
            then begin
                   sy := neq; {修改类型为不等号}
                   nextch
                 end
            else  sy := lss {修改类型为小于号}
        end;
      '>':
        begin
          nextch;
          if ch = '='
          then begin
                 sy := geq; {修改类型为大于等于号}
                 nextch
               end
          else  sy := gtr {修改类型为大于号}
        end;
      '.':
        begin
          nextch;
          if ch = '.'
          then begin
                 sy := colon; {'..'修改类型为冒号}
                 nextch
               end
          else sy := period {修改类型为句号}
        end;
      '''':
        begin
          k := 0;
   2:     nextch;
          if ch = '''' {双引号}
          then begin
                 nextch;
                 if ch <> ''''
                 then goto 3
               end;
          if sx + k = smax {字符串长度溢出}
          then fatal(7); {报错并终止程序}
          stab[sx+k] := ch; {将字符填入字符串常量表}
          k := k + 1;
          if cc = 1
          then begin { end of line }
                 k := 0;
               end
          else goto 2;
   3:     if k = 1 {双引号中间有一个字符}
          then begin
                 sy := charcon; {修改类型为字符常量}
                 inum := ord( stab[sx] ) {inum存储该字符的ASCII码值}
               end
          else if k = 0 {双引号中间没有字符,报错}
               then begin
                      error(38); {报错：字符串至少有一个字符}
                      sy := charcon; {修改类型为字符常量}
                      inum := 0 
                    end
               else begin {k>1}
                      sy := stringcon; {修改类型为字符串常量}
                      inum := sx;
                      sleng := k; {字符串长度}
                      sx := sx + k
                    end
        end;
      '(':
        begin
          nextch;
          if ch <> '*'
          then sy := lparent {修改类型为左括号}
          else begin { comment 注释}
                 nextch;
                 if ch = '$'
                 then options; {处理可选项}
                 repeat {一直读直到遇到'*)'}
                   while ch <> '*' do nextch;
                   nextch
                 until ch = ')';
                 nextch;
                 goto 1 
               end
        end;
      '{':
        begin
          nextch;
          if ch = '$'
          then options;
          while ch <> '}' do nextch;{一直读直到遇到右花括号}
          nextch;
          goto 1
        end;
      '+', '-', '*', '/', ')', '=', ',', '[', ']', ';':
        begin {特定字符}
          sy := sps[ch]; {通过特定字符表sps直接修改类型}
          nextch
        end;
      '$','"' ,'@', '?', '&', '^', '!':
        begin {直接出现会报错的字符}
          error(24); {报错：非法字符}
          nextch;
          goto 1
        end
      end { case }
    end { insymbol };

procedure enter(x0:alfa; x1:objecttyp; x2:types; x3:integer ); {把标准类型、过程和函数的名字登录到符号表tab中}
  begin
    t := t + 1;    { enter standard identifier }
    with tab[t] do
      begin
        name := x0;
        link := t - 1;
        obj := x1;
        typ := x2;
        ref := 0;
        normal := true;
        lev := 0;
        adr := x3;
      end
  end; { enter }

procedure enterarray( tp: types; l,h: integer ); {登录数组信息向量表atab}
  begin
    if l > h
    then error(27); {报错：数组的下界超过上界}
    if( abs(l) > xmax ) or ( abs(h) > xmax ) 
    then begin
           error(27); {报错：数组上下界的绝对值超过int类型上限}
           l := 0;
           h := 0;
         end;
    if a = amax
    then fatal(4) {数组溢出}
    else begin
           a := a + 1;
           with atab[a] do
             begin
               inxtyp := tp;
               low := l;
               high := h
             end
         end
  end { enterarray };

procedure enterblock; {登录分程序表btab}
  begin
    if b = bmax
    then fatal(2) {过程溢出}
    else begin {初始化}
           b := b + 1;
           btab[b].last := 0;
           btab[b].lastpar := 0;
         end
  end { enterblock };

procedure enterreal( x: real ); {登录实常数表rconst}
  begin
    if c2 = c2max - 1 {19}
    then fatal(3) {实数溢出}
    else begin
           rconst[c2+1] := x;
           c1 := 1;
           while rconst[c1] <> x do {遍历实常数表，判断是否已经登录过}
             c1 := c1 + 1;
           if c1 > c2 {如果已经登录过}
           then  c2 := c1 {将序号改为之前登录的序号}
         end
  end { enterreal };

procedure emit( fct: integer ); {生成P代码指令(无xy域)}
  begin
    if lc = cmax
    then fatal(6); {代码溢出}
	code[lc].f := fct;
    lc := lc + 1 {P代码指针后移}
end { emit };

procedure emit1( fct, b: integer ); {生成P代码指令(存在y域)}
  begin
    if lc = cmax
    then fatal(6); {代码溢出}
    with code[lc] do
      begin
        f := fct;
        y := b;
      end;
    lc := lc + 1
  end { emit1 };

procedure emit2( fct, a, b: integer ); {生成P代码指令(存在x域和y域)}
  begin
    if lc = cmax 
	then fatal(6); {代码溢出}
    with code[lc] do
      begin
        f := fct;
        x := a;
        y := b
      end;
    lc := lc + 1;
end { emit2 };

procedure printtables; {打印编译生成的符号表、分程序表、实常数表以及P代码}
  var  i: integer;
	   o: order;
      mne: array[0..omax] of
           packed array[1..5] of char;
  begin {定义P代码指令}
    mne[0] := 'LDA  ';    mne[1] := 'LOD  ';   mne[2] := 'LDI  ';
	mne[3] := 'DIS  ';    mne[8] := 'FCT  ';   mne[9] := 'INT  ';
    mne[10] := 'JMP  ';   mne[11] := 'JPC  ';  mne[12] := 'SWT  ';
    mne[13] := 'CAS  ';   mne[14] := 'F1U  ';  mne[15] := 'F2U  ';
    mne[16] := 'F1D  ';   mne[17] := 'F2D  ';  mne[18] := 'MKS  ';
    mne[19] := 'CAL  ';   mne[20] := 'IDX  ';  mne[21] := 'IXX  ';
    mne[22] := 'LDB  ';   mne[23] := 'CPB  ';  mne[24] := 'LDC  ';
	mne[25] := 'LDR  ';   mne[26] := 'FLT  ';  mne[27] := 'RED  ';
	mne[28] := 'WRS  ';   mne[29] := 'WRW  ';  mne[30] := 'WRU  ';
    mne[31] := 'HLT  ';   mne[32] := 'EXP  ';  mne[33] := 'EXF  ';
    mne[34] := 'LDT  ';   mne[35] := 'NOT  ';  mne[36] := 'MUS  ';
	mne[37] := 'WRR  ';   mne[38] := 'STO  ';  mne[39] := 'EQR  ';
	mne[40] := 'NER  ';   mne[41] := 'LSR  ';  mne[42] := 'LER  ';
    mne[43] := 'GTR  ';   mne[44] := 'GER  ';  mne[45] := 'EQL  ';
	mne[46] := 'NEQ  ';   mne[47] := 'LSS  ';  mne[48] := 'LEQ  ';
    mne[49] := 'GRT  ';   mne[50] := 'GEQ  ';  mne[51] := 'ORR  ';
    mne[52] := 'ADD  ';   mne[53] := 'SUB  ';  mne[54] := 'ADR  ';
    mne[55] := 'SUR  ';   mne[56] := 'AND  ';  mne[57] := 'MUL  ';
    mne[58] := 'DIV  ';   mne[59] := 'MOD  ';  mne[60] := 'MUR  ';
    mne[61] := 'DIR  ';   mne[62] := 'RDL  ';  mne[63] := 'WRL  ';

	writeln(psout); {打印提示信息}
    writeln(psout);
    writeln(psout);
    writeln(psout,'   identifiers  link  obj  typ  ref  nrm  lev  adr');
    writeln(psout);
    for i := btab[1].last to t do
      with tab[i] do
        writeln( psout, i,' ', name, link:5, ord(obj):5, ord(typ):5,ref:5, ord(normal):5,lev:5,adr:5);
    writeln( psout );
    writeln( psout );
    writeln( psout );
    writeln( psout, 'blocks   last  lpar  psze  vsze' );
    writeln( psout );
    for i := 1 to b do
       with btab[i] do
         writeln( psout, i:4, last:9, lastpar:5, psize:5, vsize:5 );
    writeln( psout );
    writeln( psout );
    writeln( psout );
    writeln( psout, 'arrays xtyp etyp eref low high elsz size');
    writeln( psout );
    for i := 1 to a do
      with atab[i] do
        writeln( psout, i:4, ord(inxtyp):9, ord(eltyp):5, elref:5, low:5, high:5, elsize:5, size:5);
    writeln( psout );
    writeln( psout );
    writeln( psout );
    writeln( psout, 'code:');
    writeln( psout );
    for i := 0 to lc-1 do
      begin
write( psout, i:5 );
        o := code[i];
write( psout, mne[o.f]:8, o.f:5 );
        if o.f < 31
        then if o.f < 4
             then write( psout, o.x:5, o.y:5 )
             else write( psout, o.y:10 )
        else write( psout, '          ' );
        writeln( psout, ',' )
      end;
    writeln( psout );
    writeln( psout, 'Starting address is ', tab[btab[1].last].adr:5 )
  end { printtables };


procedure block( fsys: symset; isfun: boolean; level: integer ); {分析处理分程序}
  type conrec = record
                  case tp: types of
                    ints, chars, bools : ( i:integer );
                    reals :( r:real )
                end;
  var dx : integer ;  { data allocation index 变量存储分配的索引}
      prt: integer ;  { t-index of this procedure }
      prb: integer ;  { b-index of this procedure }
	  x  : integer ;

  procedure skip( fsys:symset; n:integer); {跳读源程序，直至取来的符号属于给出的符号集为止，并打印出错标志}
    begin
      error(n); {报错}
      skipflag := true;
      while not ( sy in fsys ) do
        insymbol;
      if skipflag then endskip {打印下划线}
    end { skip };

  procedure test( s1,s2: symset; n:integer ); {测试当前符号是否合法，若不合法，打印出错标志并进行跳读}
    begin
      if not( sy in s1 )
      then skip( s1 + s2, n ) {跳读}
    end { test };

  procedure testsemicolon; {测试当前符号是否为分号}
    begin
      if sy = semicolon
      then insymbol
      else begin
             error(14); {报错：应是';'}
             if sy in [comma, colon] {如果是逗号或冒号}
             then insymbol
           end;
      test( [ident] + blockbegsys, fsys, 6 ) {如果类型不合法，报错：非法符号}
    end { testsemicolon };


  procedure enter( id: alfa; k:objecttyp ); {在符号表中登录分程序说明部分出现的名字}
    var j,l : integer;
    begin
      if t = tmax {判断符号表是否已满}
      then fatal(1) {表溢出}
      else begin
             tab[0].name := id;
             j := btab[display[level]].last; {找到该分程序当前(最后一个)标识符在tab表中的位置}
             l := j;
             while tab[j].name <> id do {循环直到在tab中找到id}
               j := tab[j].link; {找到该分程序上一个标识符在tab表中的位置}
             if j <> 0
             then error(1) {报错：标识符重复定义}
             else begin 
                    t := t + 1;
                    with tab[t] do
                      begin {填表}
                        name := id;
                        link := l;
                        obj := k;
                        typ := notyp;
                        ref := 0;
                        lev := level;
                        adr := 0;
                        normal := false { initial value }
                      end;
                    btab[display[level]].last := t {更新当前(最后一个)标识符}
                  end
           end
    end { enter };

  function loc( id: alfa ):integer; {查找标识符在符号表中的位置}
    var i,j : integer;        { locate if in table }
    begin
      i := level; {标识符所在分程序的静态层次}
      tab[0].name := id;  { sentinel }
      repeat
        j := btab[display[i]].last; {找到当前层当前(最后一个)标识符在tab表中的位置}
        while tab[j].name <> id do {循环直到在tab中找到id}
		  j := tab[j].link; {找到当前层上一个标识符在tab表中的位置}
        i := i - 1;
	  until ( i < 0 ) or ( j <> 0 );
      if j = 0
      then error(0); {报错：该标识符未定义}
      loc := j {函数返回值？}
    end { loc } ;

  procedure entervariable; {在符号表中登录变量名}
    begin
      if sy = ident {类型是标识符}
      then begin
             enter( id, vvariable );
             insymbol
           end
      else error(2) {报错：应是标识符}
    end { entervariable };

  procedure constant( fsys: symset; var c: conrec ); {处理常量，并由参数c返回该常量的类型和数值}
    var x, sign : integer;
    begin
      c.tp := notyp; {初始化}
	  c.i := 0;
      test( constbegsys, fsys, 50 ); {如果类型不合法，报错：应是常量}
      if sy in constbegsys
      then begin
             if sy = charcon {类型是字符}
             then begin
                    c.tp := chars; {记录类型}
                    c.i := inum; {记录ASCII码}
                    insymbol 
                  end
             else begin
                  sign := 1; {不是字符常量}
                  if sy in [plus, minus] 
                  then begin
                         if sy = minus {负号}
                         then sign := -1;
                         insymbol
                       end; 
                  if sy = ident {是标识符}
                  then begin
                         x := loc(id); {查找标识符在符号表中的位置}
                         if x <> 0 {找到了}
                         then
                           if tab[x].obj <> konstant {标识符不是向量}
                           then error(25) {报错：在常量定义中，等号后面必须是常数或常量标识符}
                           else begin
                                  c.tp := tab[x].typ; {记录类型}
                                  if c.tp = reals {如果标识符是实数，需要调用实常数表，否则不需要}
                                  then c.r := sign*rconst[tab[x].adr]
                                  else c.i := sign*tab[x].adr
                                end;
                         insymbol
                       end
                  else if sy = intcon {是整数}
                       then begin
                              c.tp := ints; {记录类型}
                              c.i := sign*inum; {记录数值}
                              insymbol
                            end
                       else if sy = realcon {是实数}
                            then begin
                                   c.tp := reals; {记录类型}
                                   c.r := sign*rnum; {记录数值}
                                   insymbol
                                 end
                            else skip(fsys,50) {跳过非法符号并打印出错标志}
                end;
                test(fsys,[],6) {如果类型不合法，报错：非法符号}
           end
    end { constant };

  procedure typ( fsys: symset; var tp: types; var rf,sz:integer ); {处理类型描述，由参数得到类型tp、指向类型详细信息表的指针ref和该类型大小sz}
    var eltp : types;
      elrf, x : integer;
      elsz, offset, t0, t1 : integer;

	procedure arraytyp( var aref, arsz: integer ); {处理数组类型，由参数返回指向该数组信息向量表的指针aref和数组大小arsz}
	  var eltp : types; {数组元素类型}
        low, high : conrec; {上下界}
        elrf, elsz: integer; {指针和大小}
        begin
        constant( [colon, rbrack, rparent, ofsy] + fsys, low ); {获得数组下界}
        if low.tp = reals {下界是实数}
        then begin
               error(27); {报错：实数上下界非法}
               low.tp := ints; 
               low.i := 0
             end;
        if sy = colon {下界后读到'..'}
        then insymbol
        else error(13); {报错：应是'..'}
        constant( [rbrack, comma, rparent, ofsy ] + fsys, high ); {获得数组上界}
        if high.tp <> low.tp {上下界类型不同}
        then begin
               error(27); {报错：上下界类型必须相同}
               high.i := low.i
             end;
        enterarray( low.tp, low.i, high.i ); {将数组登录到数组信息向量表atab}
        aref := a; {a：数组信息向量表atab的索引变量}
        if sy = comma {下界后读到','，说明是多维数组}
        then begin
               insymbol;
               eltp := arrays; {数组元素类型为数组}
               arraytyp( elrf, elsz ) {递归}
             end
        else begin
               if sy = rbrack {下界后读到']'}
               then insymbol
               else begin
                      error(12); {报错：应是']'}
                      if sy = rparent {如果是')'，容错}
                      then insymbol 
                    end;
               if sy = ofsy {下界后读到'of'}
               then insymbol
               else error(8); {报错：应是'of'}
               typ( fsys, eltp, elrf, elsz ) {处理类型}
             end;
             with atab[aref] do {填数组信息向量表}
               begin
                 arsz := (high-low+1) * elsz; {计算数组大小(需要的存储空间)}
                 size := arsz; {记录数组大小(需要的存储空间)}
                 eltyp := eltp; {记录数组的元素类型}
                 elref := elrf; {记录数组登录的位置}
                 elsize := elsz {记录每个元素的大小}
               end
        end { arraytyp };
    begin { typ  }
      tp := notyp; {初始化类型}
      rf := 0; {指针}
      sz := 0; {大小}
      test( typebegsys, fsys, 10 ); {如果类型不合法，报错：类型定义必须以标识符、array或record开头}
      if sy in typebegsys
      then begin
             if sy = ident {是标识符}
             then begin
                    x := loc(id); {查找标识符在符号表中的位置}
                    if x <> 0 {找到了}
                    then with tab[x] do
                           if obj <> typel
                           then error(29) {报错：应是类型标识符}
                           else begin
                                  tp := typ;
                                  rf := ref;
                                  sz := adr;
                                  if tp = notyp
                                  then error(30) {报错：该类型未定义}
                                end;
                    insymbol
                  end
             else if sy = arraysy {是数组(即开头为array)}
                  then begin
                         insymbol;
                         if sy = lbrack {下一个字符是'['}
                         then insymbol
                         else begin
                                error(11); {报错：应是'['}
                                if sy = lparent {如果是'('，容错}
                                then insymbol
                              end;
                         tp := arrays; {修改类型为数组}
                         arraytyp(rf,sz) {处理数组，获得指针和数组大小}
                         end
             else begin {是记录(即开头为records)}
                    insymbol;
                    enterblock; {登录分程序表btab}
                    tp := records;
                    rf := b; {b:分程序表btab的索引变量}
                    if level = lmax
                    then fatal(5); {层次溢出}
                    level := level + 1;
                    display[level] := b; {建立分层次索引}
                    offset := 0;
                    while not ( sy in fsys - [semicolon,comma,ident]+ [endsy] ) do
                      begin { field section 处理记录内部的变量}
                        if sy = ident {是标识符}
                        then begin
                               t0 := t; {t:符号表tab的索引变量}
                               entervariable; {在符号表中登录变量名}
                               while sy = comma do {同种类型的变量名用逗号分隔}
                                 begin
                                   insymbol;
                                   entervariable {继续登录变量名}
                                 end;
                               if sy = colon {变量名结束，遇到冒号}
                               then insymbol
                               else error(5); {报错:应是':'，在说明类型时必须有冒号}
                               t1 := t; {记录符号表位置}
                               typ( fsys + [semicolon, endsy, comma,ident], eltp, elrf, elsz ); {递归处理记录中的类型}
                               while t0 < t1 do
                               begin {逐个修改符号表表项}
                                 t0 := t0 + 1;
                                 with tab[t0] do
                                   begin
                                     typ := eltp; {记录类型}
                                     ref := elrf; {记录位置}
                                     normal := true; {记录是否为变量形参}
                                     adr := offset; {记录变量相对于起始位置的偏移量}
                                     offset := offset + elsz {下一个变量的偏移量=偏移量+当前变量大小}
                                   end
                               end
                             end; { sy = ident }
                        if sy <> endsy {不是end，变量声明未结束}
                        then begin
                               if sy = semicolon {是分号}
                               then insymbol
                               else begin
                                      error(14); {报错：应是';'}
                                      if sy = comma {如果是逗号，容错}
                                      then insymbol
                                    end;
                                    test( [ident,endsy, semicolon],fsys,6 ) {如果类型不合法，报错：非法符号}
                             end
                      end; { field section }
                    btab[rf].vsize := offset; {在分程序表中记录局部变量,参数以及display区所占的空间总数}
                    sz := offset; {typ过程返回的类型大小}
                    btab[rf].psize := 0; {该分程序的参数占用空间为0，因为记录类型没有参数}
                    insymbol;
                    level := level - 1 {返回上一层次}
                  end; { record }
             test( fsys, [],6 ) {如果类型不合法，报错：非法符号}
           end;
      end { typ };

  procedure parameterlist; { formal parameter list 处理过程或函数说明中的形参表，将形参及有关信息登录到符号表中}
    var tp : types;
        valpar : boolean; {标记是否为值形参(valueparameter)}
        rf, sz, x, t0 : integer;
    begin
      insymbol;
      tp := notyp; {初始化}
      rf := 0; {指针}
      sz := 0; {大小}
      test( [ident, varsy], fsys+[rparent], 7 ); {如果类型不合法，报错：形参表中，形参说明应以标识符或var开头}
      while sy in [ident, varsy] do
        begin
          if sy <> varsy {不是var(即是标识符)}
          then valpar := true
          else begin {是var}
                 insymbol;
                 valpar := false
               end;
          t0 := t; {t:符号表tab的索引变量}
          entervariable; {在符号表中登录变量名}
          while sy = comma do {同种类型的变量名用逗号分隔}
            begin
              insymbol;
              entervariable; {继续登录变量名}
            end;
          if sy = colon {变量名结束，遇到冒号}
          then begin
                 insymbol;
                 if sy <> ident 
                 then error(2) {报错：应是标识符}
                 else begin
                        x := loc(id); {查找标识符在符号表中的位置}
                        insymbol;
                        if x <> 0 {找到了}
                        then with tab[x] do
                          if obj <> typel
                          then error(29) {报错：应是类型标识符}}
                          else begin
                                 tp := typ; {记录类型}
                                 rf := ref; {记录位置}
                                 if valpar {如果是值形参}
                                 then sz := adr {记录形参在符号表中的位置}
                                 else sz := 1 {否则记为1}
                               end;
                      end;
                 test( [semicolon, rparent], [comma,ident]+fsys, 14 ) {如果类型不合法，报错：应是';'}
                 end
          else error(5); {报错:应是':'，在说明类型时必须有冒号}
          while t0 < t do 
            begin {从符号表的t0位置到当前指针位置，逐个修改符号表表项}
              t0 := t0 + 1;
              with tab[t0] do
                begin
                  typ := tp; {记录类型}
                  ref := rf; {记录位置}
				  adr := dx; {记录形参的相对地址}
                  lev := level; {记录层次}
                  normal := valpar; {记录是否为变量形参}
				  dx := dx + sz {更新偏移量}
                end
            end;
            if sy <> rparent {不是')'}
            then begin
                   if sy = semicolon {是';'，声明未结束}
                   then insymbol
                   else begin
                          error(14); {报错：应是';'}
                          if sy = comma {如果是逗号，容错}
                          then insymbol
                        end;
                        test( [ident, varsy],[rparent]+fsys,6) {如果类型不合法，报错：非法符号}
                 end
        end { while };
      if sy = rparent {是')'，声明已结束}
      then begin
             insymbol;
             test( [semicolon, colon],fsys,6 ) {如果类型不合法，报错：非法符号}
           end
      else error(4) {报错；应是')'}
    end { parameterlist };

  procedure constdec; {处理常量声明，将常量名及其相应信息填入符号表}
    var c : conrec;
    begin
      insymbol;
      test([ident], blockbegsys, 2 ); {如果类型不合法，报错：应是标识符}
      while sy = ident do {是标识符}
        begin
          enter(id, konstant); {登录到符号表tab中，类型为常量}
          insymbol;
          if sy = eql {下一个字符是'='}
          then insymbol
          else begin
                 error(16); {报错：应是'=','=:'只能在赋值语句里使用，而不能在说明中使用}
                 if sy = becomes {如果是'=:'，容错}
                 then insymbol
               end;
          constant([semicolon,comma,ident]+fsys,c); {处理常量}
          tab[t].typ := c.tp; {记录类型}
          tab[t].ref := 0; {常量ref记为0}
          if c.tp = reals {类型为实数}
          then begin
		         enterreal(c.r); {在实常数表rconst中登录该常数}
                 tab[t].adr := c1; {记录登录位置}
               end
          else tab[t].adr := c.i; {类型为整数时，记录登录位置}
          testsemicolon {测试当前符号是否为分号}
        end
    end { constdec };

  procedure typedeclaration; {处理类型声明，将类型名及其相应信息填入符号表}
    var tp: types;
        rf, sz, t1 : integer;
    begin
      insymbol;
      test([ident], blockbegsys,2 ); {如果类型不合法，报错：应是标识符}
      while sy = ident do {是标识符}
        begin
          enter(id, typel); {登录到符号表tab中，类型为类型}
          t1 := t; {t:符号表tab的索引变量}
          insymbol;
          if sy = eql {下一个字符是'='}
          then insymbol
          else begin
                 error(16); {报错：应是'=','=:'只能在赋值语句里使用，而不能在说明中使用}
                 if sy = becomes {如果是':='，容错}
                 then insymbol
               end;
          typ([semicolon,comma,ident]+fsys, tp,rf,sz); {处理类型}
          with tab[t1] do {填符号表}
            begin
              typ := tp; {记录类型}
              ref := rf; {记录位置}
              adr := sz  {记录大小}
            end;
          testsemicolon {测试当前符号是否为分号}
        end
    end { typedeclaration };

  procedure variabledeclaration; {处理变量声明，将变量名及其相应信息填入符号表}
    var tp : types;
        t0, t1, rf, sz : integer;
    begin
      insymbol;
	  while sy = ident do {是标识符}
        begin
          t0 := t; {t:符号表tab的索引变量}
          entervariable; {在符号表中登录变量名}
          while sy = comma do {同种类型的变量名用逗号分隔}
            begin
              insymbol;
              entervariable; {继续登录变量名}
            end;
          if sy = colon {变量名结束，遇到冒号}
          then insymbol
          else error(5); {报错:应是':'，在说明类型时必须有冒号}
          t1 := t; {记录符号表位置}
          typ([semicolon,comma,ident]+fsys, tp,rf,sz ); {处理类型}
          while t0 < t1 do
            begin {逐个修改符号表表项}
              t0 := t0 + 1; 
              with tab[t0] do
                begin
                  typ := tp; {记录类型}
                  ref := rf; {记录位置}
                  lev := level; {记录层次}
                  adr := dx; {记录形参的相对地址}
                  normal := true; {记录是否为变量形参}
                  dx := dx + sz {更新偏移量}
                end
            end;
          testsemicolon {测试当前符号是否为分号}
        end
    end { variabledeclaration };

  procedure procdeclaration; {处理过程或函数说明，将过程(函数)名填入符号表，递归调用block分析处理程序(level+1)}
    var isfun : boolean; {标记是否为函数}
    begin
      isfun := sy = funcsy;
      insymbol;
      if sy <> ident 
      then begin
             error(2); {报错：应是标识符}
             id :='          '
           end;
      if isfun {是函数}
      then enter(id,funktion)   {登录到符号表tab中，类型为函数}
      else enter(id,prozedure); {登录到符号表tab中，类型为过程}
      tab[t].normal := true;  {记录是否为变量形参}
      insymbol;
      block([semicolon]+fsys, isfun, level+1 ); {递归调用block分析处理程序(level+1)}
      if sy = semicolon
      then insymbol
      else error(14); {报错：应是';'}
      emit(32+ord(isfun)) {exit 生成退出过程的指令}
    end { proceduredeclaration };


  procedure statement( fsys:symset ); {分析处理各种语句}
    var i : integer;

	procedure expression(fsys:symset; var x:item); {分析处理表达式，由参数x返回求值结果的类型}
	forward;
    procedure selector(fsys:symset; var v:item); {处理结构变量：数组下标变量或记录成员变量;由参数v返回求值结果的类型}
    var x : item;
        a,j : integer;
    begin { sy in [lparent, lbrack, period] }
      repeat
        if sy = period {是句号，处理记录成员变量(引用记录成员变量的方式为'记录名.成员名')}
        then begin
               insymbol; { field selector }
               if sy <> ident
               then error(2) {报错：应是标识符}
               else begin
                      if v.typ <> records
                      then error(31) {报错：没有这样的记录}
                      else begin { search field identifier 寻找类型标识符}
                             j := btab[v.ref].last; {找到该分程序当前(最后一个)标识符在tab表中的位置}
                             tab[0].name := id;
                             while tab[j].name <> id do {循环直到在tab中找到id}
                               j := tab[j].link; {找到该分程序上一个标识符在tab表中的位置}
                             if j = 0
                             then error(0); {报错：该标识符未定义}
                             v.typ := tab[j].typ; {获得属性}
                             v.ref := tab[j].ref; {获得位置}
                             a := tab[j].adr; {获得该成员变量相对于记录变量起始地址的偏移量}
                             if a <> 0
                             then emit1(9,a) {生成计算偏移量的指令}
                           end;
                      insymbol
                    end
             end
        else begin { array selector 处理数组下标变量}
               if sy <> lbrack
               then error(11); {报错：应是'['}
               repeat
                 insymbol;
                 expression( fsys+[comma,rbrack],x); {递归调用expression分析数组下标}
                 if v.typ <> arrays
                 then error(28) {报错：没有这样的数组}
                 else begin
                        a := v.ref; {获得位置}
                        if atab[a].inxtyp <> x.typ
                        then error(26) {报错：下标表达式类型必须与数组说明中的下标类型相同}
                        else if atab[a].elsize = 1 {元素长度为1}
                             then emit1(20,a)  {生成取下标变量地址{元素长度=1}的指令}
                             else emit1(21,a); {生成取下标变量地址的指令}
                        v.typ := atab[a].eltyp; {获得元素类型}
                        v.ref := atab[a].elref  {获得元素位置}
                      end
               until sy <> comma; {读到逗号说明是高维数组}
               if sy = rbrack {读到']'，声明结束}
               then insymbol
               else begin
                      error(12); {报错：应是']'}
                      if sy = rparent {如果是')'，容错}}
                      then insymbol
                    end
             end
      until not( sy in[lbrack, lparent, period]); {循环直到读完}
      test( fsys,[],6) {如果类型不合法，报错：非法符号}
    end { selector };

    procedure call( fsys: symset; i:integer ); {处理非标准的过程或函数调用}
	  var x : item;
          lastp,cp,k : integer;
	  begin
        emit1(18,i); { mark stack 生成标记栈的指令}
        lastp := btab[tab[i].ref].lastpar; {获得过程或函数的最后一个参数在tab表中的位置}
        cp := i; {获得过程或函数在符号表中的位置}
        if sy = lparent {是'('}
        then begin { actual parameter list 处理实参列表}
               repeat
                 insymbol;
                 if cp >= lastp
                 then error(39) {报错：实参个数与形参个数不等}
                 else begin
                        cp := cp + 1;
                        if tab[cp].normal }
                        then begin { value parameter 值形参}
                               expression( fsys+[comma, colon,rparent],x); {递归调用expression分析表达式}}
                               if x.typ = tab[cp].typ
                               then begin
                                      if x.ref <> tab[cp].ref
                                      then error(36) {报错：实参和对应形参类型应相同}
                                      else if x.typ = arrays {是数组}
                                           then emit1(22,atab[x.ref].size) {生成数组的装入块指令}
                                           else if x.typ = records {是记录}
                                                then emit1(22,btab[x.ref].vsize) {生成记录的装入块指令}
                                    end
                               else if ( x.typ = ints ) and ( tab[cp].typ = reals )
                                    then emit1(26,0) {生成整数转换成浮点数的指令}
                                    else if x.typ <> notyp
                                         then error(36); {报错：实参和对应形参类型应相同，除非形参为实型的值形参，而实参为整型}
                             end
                        else begin { variable parameter 变量形参}
                               if sy <> ident
                               then error(2) {报错：应是标识符}
                               else begin
                                      k := loc(id); {查找标识符在符号表中的位置}
                                      insymbol;
                                      if k <> 0 {找到了}
                                      then begin
                                             if tab[k].obj <> vvariable
                                             then error(37); {报错：应是变量}
                                             x.typ := tab[k].typ; {获得类型}
                                             x.ref := tab[k].ref; {获得位置}
                                             if tab[k].normal {是值形参}
                                             then emit2(0,tab[k].lev,tab[k].adr) {生成将变量地址装入栈顶的指令}
											 else emit2(1,tab[k].lev,tab[k].adr); {生成将变量值装入栈顶的指令}
											 if sy in [lbrack, lparent, period]
                                             then selector(fsys+[comma,colon,rparent],x); {处理结构变量}
                                             if ( x.typ <> tab[cp].typ ) or ( x.ref <> tab[cp].ref )
                                             then error(36) {报错：实参和对应形参类型应相同，除非形参为实型的值形参，而实参为整型}
                                           end
                                    end
                             end {variable parameter }
                      end;
                 test( [comma, rparent],fsys,6) {如果类型不合法，报错：非法符号}
               until sy <> comma; {读到逗号，声明结束}
               if sy = rparent
               then insymbol
               else error(4) {报错；应是')'}
             end;
        if cp < lastp
        then error(39); { too few actual parameters 报错：实参个数与形参个数不等}
        emit1(19,btab[tab[i].ref].psize-1 ); {生成调用用户过程或函数的指令}
        if tab[i].lev < level {如果符号所在层次小于当前层次}
        then emit2(3,tab[i].lev, level ) {生成更新display区的指令}
      end { call };

    function resulttype( a, b : types) :types; {处理整型或实型两个操作数运算时的类型转换}
      begin
        if ( a > reals ) or ( b > reals ) {超出实数上限}
        then begin
               error(33); {报错：该算数表达式的类型不合法}
               resulttype := notyp
             end
        else if ( a = notyp ) or ( b = notyp ) {a和b中至少有一个无类型}
             then resulttype := notyp {结果为无类型}
             else if a = ints
                  then if b = ints {a整b整}
                       then resulttype := ints {结果为整型}
                       else begin {a整b实}
                              resulttype := reals; {结果为实型}
                              emit1(26,1) {生成将a转换为浮点数的指令}
                            end
                  else begin {a实}
                         resulttype := reals; {结果为实型}
                         if b = ints {b也是整型}
                         then emit1(26,0) {生成将b转换为浮点数的指令}
                       end
      end { resulttype } ;

    procedure expression( fsys: symset; var x: item ); {分析处理表达式，由参数x返回求值结果的类型}
	  var y : item;
          op : symbol;

      procedure simpleexpression( fsys: symset; var x: item ); {处理简单表达式，由参数x返回求值结果的类型}
        var y : item; 
            op : symbol;

        procedure term( fsys: symset; var x: item ); {处理项，由参数x返回求值结果的类型}
          var y : item;
              op : symbol;

          procedure factor( fsys: symset; var x: item ); {处理因子，由参数x返回求值结果的类型}
            var i,f : integer;

            procedure standfct( n: integer ); {处理标准函数调用}
              var ts : typset;
			  begin  { standard function no. n 第n个标准函数}
                if sy = lparent
                then insymbol
                else error(9); {报错：应是'('}
                if n < 17
                then begin
                       expression( fsys+[rparent], x ); {递归调用expression分析表达式}}
                       case n of
                       { abs, sqr } 0,2: begin
                                           ts := [ints, reals]; 
                                           tab[i].typ := x.typ; {记录函数返回值类型}
                                           if x.typ = reals {如果返回值为实型}
                                           then n := n + 1 {调整标准函数为计算实数的函数}
                                         end;
                       { odd, chr } 4,5: ts := [ints];
                       { odr }        6: ts := [ints,bools,chars];
                       { succ,pred } 7,8 : begin
                                             ts := [ints, bools,chars];
                                             tab[i].typ := x.typ {记录函数返回值类型}
                                           end;
                       { round,trunc } 9,10,11,12,13,14,15,16:
                       { sin,cos,... }     begin
                                             ts := [ints,reals];
                                             if x.typ = ints {如果返回值为整型}
                                             then emit1(26,0) {生成转换为浮点数的指令}
                                           end;
                     end; { case }
                     if x.typ in ts {返回值类型符合符号集}
                     then emit1(8,n) {生成调用相应标准函数的指令}
                     else if x.typ <> notyp
                          then error(48); {报错：该标准函数的变元表达式类型不正确}
                   end
                else begin { n in [17,18] }
                       if sy <> ident
                       then error(2) {报错：应是标识符}
                       else if id <> 'input    '
                            then error(0) {报错：该标识符未定义}
                            else insymbol;
                       emit1(8,n); {生成调用相应标准函数的指令}
                     end;
				x.typ := tab[i].typ; {保存返回值类型}
				if sy = rparent
                then insymbol
                else error(4) {报错；应是')'}
              end { standfct } ;
            begin { factor }
              x.typ := notyp; {初始化类型}
              x.ref := 0; {指针}
              test( facbegsys, fsys,58 ); {如果类型不合法，报错：因子必须以标识符、常量、not或'('开始}
              while sy in facbegsys do
                begin
                  if sy = ident {是标识符}
                  then begin
                         i := loc(id); {查找标识符在符号表中的位置}
                         insymbol;
                         with tab[i] do
                           case obj of
                             konstant: begin {是常量}
                                         x.typ := typ; {保存类型}
                                         x.ref := 0;   {保存指针(置0)}
                                         if x.typ = reals {如果是实型}
										 then emit1(25,adr) {生成装入实数的指令}
                                         else emit1(24,adr) {生成装入常量的指令}
									   end;
                             vvariable:begin {是变量}
                                         x.typ := typ; {保存类型}
                                         x.ref := ref; {保存位置}
                                         if sy in [lbrack, lparent, period] {变量存在子结构}
                                         then begin
                                                if normal {是值形参}
												then f := 0  {将变量地址装入栈顶}
												else f := 1; {将变量值装入栈顶}
                                                emit2(f,lev,adr); {生成上述指令}
												selector(fsys,x); {递归调用selector分析处理子结构}
                                                if x.typ in stantyps {如果子结构的类型在类型集合中}
                                                then emit(34) {生成取栈顶内容为地址的指令}
                                              end
                                         else begin
                                                if x.typ in stantyps {如果类型在类型集合中}
                                                then if normal {是值形参}
                                                     then f := 1  {将变量值装入栈顶}
                                                     else f := 2  {将变量值间接装入栈顶}
                                                else if normal {是值形参}
													 then f := 0  {将变量地址装入栈顶}
													 else f := 1; {将变量值装入栈顶}
                                                emit2(f,lev,adr) {生成上述指令}
											  end
                                       end;
                             typel,prozedure: error(44); {是类型或过程，报错：表达式中不能出现类型或过程标识符}
                             funktion: begin {是函数}
                                         x.typ := typ; {保存类型}
                                         if lev <> 0 {层次不为0，即不是标准函数}
                                         then call(fsys,i) {处理非标准的过程或函数调用}
                                         else standfct(adr) {处理标准函数调用}
                                       end
                           end { case,with }
                       end
                  else if sy in [ charcon,intcon,realcon ]
                       then begin
                              if sy = realcon {是实数常量}
                              then begin
									 x.typ := reals; {修改类型为实数常量}
                                     enterreal(rnum); {在实常数表rconst中登录该常数}
									 emit1(25,c1) {生成装入实数的指令}
                                   end
                              else begin
                                     if sy = charcon
                                     then x.typ := chars {修改类型为字符常量}
                                     else x.typ := ints; {修改类型为整型常量}
                                     emit1(24,inum) {生成装入常量的指令}
                                   end;
                              x.ref := 0; {保存指针(置0)}
                              insymbol
                            end
                       else if sy = lparent {是'('}
                            then begin
                                   insymbol;
                                   expression(fsys + [rparent],x); {递归调用expression分析表达式}
                                   if sy = rparent
                                   then insymbol
                                   else error(4) {报错；应是')'}
                                 end
                             else if sy = notsy {是not}
                                  then begin
                                         insymbol;
                                         factor(fsys,x); {递归调用factor分析因子}
                                         if x.typ = bools {因子返回值为布尔型}
                                         then emit(35) {生成逻辑非的指令}
                                         else if x.typ <> notyp
                                              then error(32) {报错：not、and和or的操作数必须是布尔型}
                                       end;
                  test(fsys,facbegsys,6) {如果类型不合法，报错：非法符号}
                end { while }
            end { factor };
          begin { term   }
            factor( fsys + [times,rdiv,idiv,imod,andsy],x); {分析因子}
            while sy in [times,rdiv,idiv,imod,andsy] do {如果读到'*''/''DIV''MOD''AND'说明后面还有因子}
              begin
                op := sy; {获得类型}
                insymbol;
                factor(fsys+[times,rdiv,idiv,imod,andsy],y ); {递归调用factor分析因子}
                if op = times {乘法}
                then begin
                       x.typ := resulttype(x.typ, y.typ); {得到计算后的类型}
                       case x.typ of
                         notyp: ; {无类型：不做处理}
                         ints : emit(57); {整型：生成整型乘的指令}
                         reals: emit(60); {实型：生成实型乘的指令}
                       end
                     end
                else if op = rdiv {除法}
                     then begin
                            if x.typ = ints {x是整型}
                            then begin
                                   emit1(26,1); {生成将x转换为浮点数的指令}
                                   x.typ := reals; {修改类型为实型}
                                 end;
                            if y.typ = ints {y是整型}
                            then begin
                                   emit1(26,0); {生成将y转换为浮点数的指令}
                                   y.typ := reals; {修改类型为实型}
                                 end;
                            if (x.typ = reals) and (y.typ = reals) {xy均为实型}
                            then emit(61) {生成实型除的指令}
                            else begin
                                   if( x.typ <> notyp ) and (y.typ <> notyp)
                                   then error(33); {报错：该算数表达式的类型不合法}
                                   x.typ := notyp {修改类型为无类型}
                                 end
                          end
                     else if op = andsy {与运算}
                          then begin
                                 if( x.typ = bools )and(y.typ = bools) {xy均为布尔型}
                                 then emit(56) {生成逻辑与的指令}
                                 else begin
                                        if( x.typ <> notyp ) and (y.typ <> notyp)
                                        then error(32);  {报错：not、and和or的操作数必须是布尔型}
                                        x.typ := notyp {修改类型为无类型}
                                      end
                               end
                          else begin { op in [idiv,imod] }
                                 if (x.typ = ints) and (y.typ = ints) {xy均为整型}
                                 then if op = idiv 
                                      then emit(58) {生成整型除的指令}
                                      else emit(59) {生成求余数的指令}
                                 else begin
                                        if ( x.typ <> notyp ) and (y.typ <> notyp)
                                        then error(34); {报错：div和mod运算的操作数必须是整型}
                                        x.typ := notyp {修改类型为无类型}
                                      end
                               end
              end { while }
          end { term };
        begin { simpleexpression }
          if sy in [plus,minus]{是正负号}
          then begin
                 op := sy; {获得类型}
                 insymbol;
                 term( fsys+[plus,minus],x); {分析项}
                 if x.typ > reals
                 then error(33) {报错：该算数表达式的类型不合法}
                 else if op = minus {减法}
                      then emit(36) {生成取相反数的指令}
               end
          else term(fsys+[plus,minus,orsy],x); {分析项}
          while sy in [plus,minus,orsy] do
            begin
              op := sy; {获得类型}
              insymbol;
              term(fsys+[plus,minus,orsy],y); {分析项}
              if op = orsy {是或运算}
              then begin
                     if ( x.typ = bools )and(y.typ = bools) {xy均为布尔型}
                     then emit(51) {生成逻辑或的指令}
                     else begin
                            if( x.typ <> notyp) and (y.typ <> notyp)
                            then error(32); {报错：not、and和or的操作数必须是布尔型}
                            x.typ := notyp {修改类型为无类型}
                          end
                   end
              else begin
                     x.typ := resulttype(x.typ,y.typ); {得到计算后的类型}
                     case x.typ of
                       notyp: ; {无类型：不做处理}
                       ints: if op = plus 
                             then emit(52)  {生成整型加的指令}
                             else emit(53); {生成整型减的指令}
                       reals:if op = plus
                             then emit(54)  {生成实型加的指令}
                             else emit(55)  {生成实型减的指令}
                     end { case }
                   end
            end { while }
          end { simpleexpression };
      begin { expression  }
        simpleexpression(fsys+[eql,neq,lss,leq,gtr,geq],x); {处理简单表达式}
        if sy in [eql,neq,lss,leq,gtr,geq]
        then begin
               op := sy; {获得类型}
               insymbol;
               simpleexpression(fsys,y);  {处理简单表达式}
               if(x.typ in [notyp,ints,bools,chars]) and (x.typ = y.typ) {xy类型相同}
               then case op of
                      eql: emit(45); {生成整型相等比较的指令}
                      neq: emit(46); {生成整型不等比较的指令}
                      lss: emit(47); {生成整型小于比较的指令}
                      leq: emit(48); {生成整型小于等于比较的指令}
                      gtr: emit(49); {生成整型大于比较的指令}
                      geq: emit(50); {生成整型大于等于比较的指令}
                    end
               else begin
                      if x.typ = ints {x是整型}
                      then begin
                             x.typ := reals; {修改类型为实型}
                             emit1(26,1) {生成将x转换成浮点数的指令}
                           end
                      else if y.typ = ints {y是整型}
                           then begin
                                  y.typ := reals; {修改类型为实型}
                                  emit1(26,0) {生成将y转换成浮点数的指令}
                                end;
                      if (x.typ = reals)and(y.typ=reals) {xy均为实型}
                      then case op of
                             eql: emit(39); {生成实型相等比较的指令}
                             neq: emit(40); {生成实型不等比较的指令}
                             lss: emit(41); {生成实型小于比较的指令}
                             leq: emit(42); {生成实型小于等于比较的指令}
                             gtr: emit(43); {生成实型大于比较的指令}
                             geq: emit(44); {生成实型大于等于比较的指令}
                           end
                      else error(35) {报错：相比较的对象类型必须相同。数组只能比较他们的元素}
                    end;
               x.typ := bools
             end
      end { expression };

    procedure assignment( lv, ad: integer ); {处理赋值语句}
      var x,y: item;
          f  : integer;
      begin   { tab[i].obj in [variable,prozedure] 变量或过程}
		x.typ := tab[i].typ; {获得类型}
		x.ref := tab[i].ref; {获得位置}
        if tab[i].normal {是值形参}
        then f := 0 {将变量地址装入栈顶}
        else f := 1; {将变量值装入栈顶}
        emit2(f,lv,ad); {生成上述指令}
        if sy in [lbrack,lparent,period]
        then selector([becomes,eql]+fsys,x); {分析下标}
        if sy = becomes
        then insymbol
        else begin
               error(51); {报错：应是':='}
               if sy = eql {如果是等号，容错}
               then insymbol
             end;
        expression(fsys,y); {分析':='右边的表达式}
        if x.typ = y.typ {xy类型相同}
        then if x.typ in stantyps {类型合法}
             then emit(38) {生成将栈顶内容存入次栈顶地址的单元，即完成赋值}
             else if x.ref <> y.ref
                  then error(46) {报错：在赋值语句中被赋变量应与表达式类型相同}
                  else if x.typ = arrays {x是数组}
                       then emit1(23,atab[x.ref].size)  {生成拷贝atab中的项的指令}
                       else emit1(23,btab[x.ref].vsize) {生成拷贝btab中的记录项的指令}
        else if(x.typ = reals )and (y.typ = ints)
        then begin
               emit1(26,0); {生成将y转换为浮点数的指令}
               emit(38) {生成将栈顶内容存入次栈顶地址的单元，即完成赋值}
             end
        else if ( x.typ <> notyp ) and ( y.typ <> notyp )
             then error(46) {报错：在赋值语句中被赋变量应与表达式类型相同}
      end { assignment };

    procedure compoundstatement; {处理复合语句}
       begin
        insymbol;
        statement([semicolon,endsy]+fsys); {分析表达式}
        while sy in [semicolon]+statbegsys do
          begin
            if sy = semicolon
            then insymbol
            else error(14); {报错：应是';'}
            statement([semicolon,endsy]+fsys) {分析分号后的表达式}
          end;
        if sy = endsy
        then insymbol
        else error(57) {报错：应是end}
      end { compoundstatement };

    procedure ifstatement; {处理if语句}
	  var x : item;
          lc1,lc2: integer;
	  begin
        insymbol;
        expression( fsys+[thensy,dosy],x); {分析表达式}
        if not ( x.typ in [bools,notyp])
        then error(17); {报错：在if,while或until后面的表达式必须是boolean型}
        lc1 := lc; {获得当前行}
        emit(11);  { jmpc 生成判断栈顶内容，如果为假就跳转地址的指令}
        if sy = thensy
        then insymbol
        else begin
               error(52); {报错：应是then}
               if sy = dosy
               then insymbol
             end;
        statement( fsys+[elsesy]); {分析then后面的语句}
        if sy = elsesy {存在else分支}
        then begin
               insymbol;
               lc2 := lc; {获得当前行}
			   emit(10); {生成无条件跳转地址的指令}
               code[lc1].y := lc;
               statement(fsys); {分析else后面的语句}
               code[lc2].y := lc
             end
        else code[lc1].y := lc
	  end { ifstatement };

    procedure casestatement; {处理case语句}
      var x : item;
		i,j,k,lc1 : integer;
		casetab : array[1..csmax]of
                     packed record
                       val,lc : index
                     end;
        exittab : array[1..csmax] of integer;

	  procedure caselabel; {处理case语句中的标号，将各标号对应的目标代码入口地址填入casetab表中，并检查标号有无重复定义}
        var lab : conrec;
		  k : integer;
        begin
          constant( fsys+[comma,colon],lab ); {分析标号(常量)，保存为lab}
          if lab.tp <> x.typ
          then error(47) {报错：case语句中标号必须是与case子句表达式类型相同的常量}
          else if i = csmax
               then fatal(6) {代码溢出}
               else begin
                      i := i+1;
					  k := 0;
                      casetab[i].val := lab.i; {记录标号的值}
                      casetab[i].lc := lc; {记录标号对应的目标代码入口地址}
					  repeat {从零开始遍历标号，检查是否被重复定义}
                        k := k+1
                      until casetab[k].val = lab.i;
                      if k < i 
                      then error(1); { multiple definition 报错：标识符重复定义}
                    end
        end { caselabel };

      procedure onecase; {处理case语句中的一个分支}
        begin
          if sy in constbegsys
          then begin
                 caselabel; {分析标号}
                 while sy = comma do {读到逗号，说明一个分支中有多个标号}
                   begin
                     insymbol; 
                     caselabel {分析标号}
                   end;
                 if sy = colon 
                 then insymbol
                 else error(5); {报错:应是':'，在说明类型时必须有冒号}
                 statement([semicolon,endsy]+fsys); {分析冒号右侧的语句}
				 j := j+1;
                 exittab[j] := lc; {记录当前case分支结束的代码位置}
				 emit(10) {生成无条件跳转到上述地址的指令，结束这一分支}
               end
          end { onecase };
		  
      begin  { casestatement  }
        insymbol;
        i := 0; {标号}
        j := 0; {分支}
        expression( fsys + [ofsy,comma,colon],x ); {分析case后的表达式}
        if not( x.typ in [ints,bools,chars,notyp ])
		then error(23); {报错：case后面的表达式必须是integer, char或boolean型}
        lc1 := lc; {记录P代码位置}
        emit(12); {jmpx} {生成跳转到给定地址查找情况表的指令SWT，注意这里暂时没有给定跳转的地址} 
		if sy = ofsy
        then insymbol
        else error(8); {报错：应是of}
        onecase; {分析case中的一个分支}
        while sy = semicolon do {读到分号，说明还有下一个分支}
          begin
            insymbol;
            onecase {继续分析下一个分支}
          end;
        code[lc1].y := lc; {确定了情况表的开始地址,回填给之前声明的SWT指令,确保其能够成功跳转}
        for k := 1 to i do {遍历所有标号}
          begin
            emit1( 13,casetab[k].val); {生成在情况表中登记标号值的指令}
            emit1( 13,casetab[k].lc);  {生成在情况表中登记标号目标代码入口地址的指令}
          end;
        emit1(10,0); {生成无条件跳转的指令，结束情况表的建立}
        for k := 1 to j do {遍历所有分支}
		code[exittab[k]].y := lc; {记录每个case分支退出之后的跳转地址}
		if sy = endsy
        then insymbol
        else error(57) {报错：应是end}
      end { casestatement };

    procedure repeatstatement; {处理repeat语句}
      var x : item;
          lc1: integer;
      begin
        lc1 := lc; {记录P代码位置}
        insymbol;
        statement( [semicolon,untilsy]+fsys); {分析repeat后的语句}
        while sy in [semicolon]+statbegsys do
          begin
            if sy = semicolon
            then insymbol
            else error(14); {报错：应是';'}
            statement([semicolon,untilsy]+fsys) {分析';'后的语句}
          end;
        if sy = untilsy {读到until, 循环体结束}
        then begin
               insymbol;
               expression(fsys,x); {分析until后的表达式}
               if not(x.typ in [bools,notyp] )
               then error(17); {报错：在if,while或until后面的表达式必须是boolean型}
               emit1(11,lc1); {生成判断栈顶内容(循环条件)，如果为假就跳转地址的指令}
             end
        else error(53) {报错：应是until}
      end { repeatstatement };

    procedure whilestatement; {处理while语句}
      var x : item;
          lc1,lc2 : integer;
      begin
        insymbol;
        lc1 := lc; {记录P代码位置}
        expression( fsys+[dosy],x); {分析while后的表达式}
        if not( x.typ in [bools, notyp] )
	    then error(17); {报错：在if,while或until后面的表达式必须是boolean型}
        lc2 := lc; {记录P代码位置}
        emit(11); {生成判断栈顶内容(循环条件)，如果为假就跳转地址的指令}
		if sy = dosy
        then insymbol
        else error(54); {报错：应是do}
		statement(fsys); {分析do后的语句}
        emit1(10,lc1); {生成无条件跳转的指令，结束循环}
        code[lc2].y := lc {记录结束循环之后的跳转地址}
end { whilestatement };

    procedure forstatement; {处理for语句}
      var cvt : types;
		  x :  item;
          i,f,lc1,lc2 : integer;
	  begin
        insymbol;
        if sy = ident {是标识符}
        then begin
               i := loc(id); {查找标识符在符号表中的位置}
               insymbol;
               if i = 0 {找不到}
               then cvt := ints {默认类型为整型}
               else if tab[i].obj = vvariable {找到了，且是变量}
                    then begin
                           cvt := tab[i].typ; {获得类型}
                           if not tab[i].normal {是变量形参，即存储的是值而不是地址}
                           then error(37) {报错：应是变量}
						   else emit2(0,tab[i].lev, tab[i].adr ); {生成将变量地址装入栈顶的指令}
						   if not ( cvt in [notyp, ints, bools, chars])
                           then error(18) {报错：在for循环后面的循环变量只能是integer,char或boolean型}
                         end
                    else begin
                           error(37); {报错：应是变量}
                           cvt := ints {默认类型为整型}
                         end
			 end
        else skip([becomes,tosy,downtosy,dosy]+fsys,2); {跳过无用符号}
        if sy = becomes {读到了':='}
        then begin
               insymbol;
               expression( [tosy, downtosy,dosy]+fsys,x); {处理':='后的表达式}
               if x.typ <> cvt
               then error(19); {报错：for语句中初值或终值表达式必须与循环变量类型相同}
             end
        else skip([tosy, downtosy,dosy]+fsys,51); {跳过无用符号}
        f := 14; {默认生成F1U指令}
        if sy in [tosy,downtosy] {to:增量步长型for；downto:减量步长型for}
        then begin
               if sy = downtosy 
               then f := 16; {改为生成F1D指令}
               insymbol;
               expression([dosy]+fsys,x); {分析to/downto之后的表达式}
               if x.typ <> cvt
               then error(19) {报错：for语句中初值或终值表达式必须与循环变量类型相同}
             end
        else skip([dosy]+fsys,55); {跳过无用符号}
        lc1 := lc; {记录P代码位置}
        emit(f); {生成上述指令}
        if sy = dosy
        then insymbol
        else error(54); {报错：应是do}
        lc2 := lc; {记录P代码位置}
		statement(fsys); {分析do后的语句}
        emit1(f+1,lc2);  {生成F2U/F2D指令，结束循环}
        code[lc1].y := lc {记录结束循环之后的跳转地址}
end { forstatement };

    procedure standproc( n: integer ); {处理标准输入/输出过程调用}
      var i,f : integer;
		  x,y : item;
      begin
        case n of
          1,2 : begin { read }
                  if not iflag
                  then begin
                         error(20); {报错：程序头部未包含参数'output'或'input'}
                         iflag := true
                       end;
                  if sy = lparent
                  then begin
                         repeat
                           insymbol;
                           if sy <> ident
                           then error(2) {报错：应是标识符}
                           else begin
                                  i := loc(id); {查找标识符在符号表中的位置}
                                  insymbol;
                                  if i <> 0 {找到了}
                                  then if tab[i].obj <> vvariable
                                       then error(37) {报错：应是变量}
                                       else begin
                                              x.typ := tab[i].typ; {获得类型}
                                              x.ref := tab[i].ref; {获得位置}
                                              if tab[i].normal {是值形参}
											  then f := 0  {生成将变量地址装入栈顶的指令}
                                              else f := 1; {生成将变量值装入栈顶的指令}
                                              emit2(f,tab[i].lev,tab[i].adr);
											  if sy in [lbrack,lparent,period]
                                              then selector( fsys+[comma,rparent],x); {分析结构变量}
                                              if x.typ in [ints,reals,chars,notyp]
											  then emit1(27,ord(x.typ)) {生成读的指令}
											  else error(41) {报错：read或write的参数类型不正确}
                                            end
                                end;
                           test([comma,rparent],fsys,6); {如果类型不合法，报错：非法符号}
                         until sy <> comma;
                         if sy = rparent
                         then insymbol
                         else error(4) {报错；应是')'}
                       end;
                  if n = 2
                  then emit(62) {生成读完一行的指令}
                end;
          3,4 : begin { write }
                  if sy = lparent
                  then begin
                         repeat
                           insymbol;
                           if sy = stringcon {是字符串常量}
                           then begin
                                  emit1(24,sleng); {生成装入字面常量的指令}
                                  emit1(28,inum); {生成写字符的指令}
                                  insymbol
                                end
                           else begin
								  expression(fsys+[comma,colon,rparent],x); {分析表达式}
								  if not( x.typ in stantyps )
                                  then error(41); {报错：read或write的参数类型不正确}
                                  if sy = colon {读到冒号，说明给定了域宽}
                                  then begin
										 insymbol;
                                         expression( fsys+[comma,colon,rparent],y); {继续分析表达式}
										 if y.typ <> ints
                                         then error(43); {报错：write语句的域宽应为整型}
                                         if sy = colon 
                                         then begin
                                                if x.typ <> reals
                                                then error(42); {报错：该表达式应为实型}
                                                insymbol;
                                                expression(fsys+[comma,rparent],y); {继续分析表达式}
                                                if y.typ <> ints
                                                then error(43); {报错：write语句的域宽应为整型}
                                                emit(37) {生成写实数(给定域宽)的指令}
                                              end
                                         else emit1(30,ord(x.typ)) {生成写(给定域宽)的指令}
                                       end
								  else emit1(29,ord(x.typ)) {生成写(隐含域宽)的指令}
								end
                         until sy <> comma;
                         if sy = rparent
                         then insymbol
                         else error(4) {报错；应是')'}
                       end;
                  if n = 4
                  then emit(63) {生成换行写的指令}
                end; { write }
        end { case };
      end { standproc } ;
    begin { statement }
      if sy in statbegsys+[ident]
      then case sy of
             ident : begin
                       i := loc(id);
                       insymbol;
                       if i <> 0
                       then case tab[i].obj of
							  konstant,typel : error(45); {报错：应是变量或过程/函数标识符}
                              vvariable:       assignment( tab[i].lev,tab[i].adr); {分析赋值语句}
							  prozedure:       if tab[i].lev <> 0
                                               then call(fsys,i) {分析非标准的过程调用}
                                               else standproc(tab[i].adr); {分析标准输入/输出过程调用}
                              funktion:        if tab[i].ref = display[level]
                                               then assignment(tab[i].lev+1,0) {分析赋值语句}
                                               else error(45) {报错：应是变量或过程/函数标识符}
                            end { case }
                     end;
             beginsy : compoundstatement;
             ifsy    : ifstatement;
             casesy  : casestatement;
             whilesy : whilestatement;
             repeatsy: repeatstatement;
             forsy   : forstatement;
           end;  { case }
      test( fsys, [],14); {如果类型不合法，报错：应是';'}
    end { statement };
  begin  { block }
    dx := 5; {变量存储分配的索引，每个分程序在运行栈中的数据区开头应留出5个单元}
    prt := t;
    if level > lmax
    then fatal(5); {层次溢出}
    test([lparent,colon,semicolon],fsys,14); {如果类型不合法，报错：应是';'}
    enterblock; {登录分程序表btab}
    prb := b;
    display[level] := b;
    tab[prt].typ := notyp;
    tab[prt].ref := prb;
    if ( sy = lparent ) and ( level > 1 )
    then parameterlist; {处理过程或函数说明中的形参表，将形参及有关信息登录到符号表中}
    btab[prb].lastpar := t;
    btab[prb].psize := dx;
    if isfun {是函数}
    then if sy = colon
         then begin
                insymbol; { function type }
                if sy = ident
                then begin
                       x := loc(id); {查找标识符在符号表中的位置}
                       insymbol;
                       if x <> 0 {找到了}
                       then if tab[x].typ in stantyps
                            then tab[prt].typ := tab[x].typ
                            else error(15) {函数结果必须是integer,real,boolean或char类型}
                     end
                else skip( [semicolon]+fsys,2 ) {跳过无用符号}
              end
         else error(5); {报错:应是':'，在说明类型时必须有冒号}
    if sy = semicolon
    then insymbol
    else error(14); {报错：应是';'}
    repeat
      if sy = constsy 
      then constdec; {处理常量声明}
      if sy = typesy
      then typedeclaration; {处理类型声明}
      if sy = varsy
      then variabledeclaration; {处理变量声明}
      btab[prb].vsize := dx;
      while sy in [procsy,funcsy] do
        procdeclaration; {处理过程或函数说明}
      test([beginsy],blockbegsys+statbegsys,56)  {如果类型不合法，报错：应是begin}
    until sy in statbegsys;
    tab[prt].adr := lc; {记录P代码指针}
    insymbol;
    statement([semicolon,endsy]+fsys); {处理语句}
    while sy in [semicolon]+statbegsys do
      begin
        if sy = semicolon
        then insymbol
        else error(14); {报错：应是';'}
        statement([semicolon,endsy]+fsys);  {继续处理语句}
      end;
    if sy = endsy
    then insymbol
    else error(57); {报错：应是end}
    test( fsys+[period],[],6 )  {如果类型不合法，报错：非法符号}
  end { block };

  
procedure interpret; {P代码解释执行程序}
  var ir : order ;         { instruction buffer }
      pc : integer;        { program counter }
      t  : integer;        { top stack index }
	  b  : integer;        { base index }
      h1,h2,h3: integer;
      lncnt,ocnt,blkcnt,chrcnt: integer;     { counters }
      ps : ( run,fin,caschk,divchk,inxchk,stkchk,linchk,lngchk,redchk ); {错误类型}
	  fld: array [1..4] of integer;  { default field widths }
      display : array[0..lmax] of integer;
      s  : array[1..stacksize] of   { blockmark:     }
            record
              case cn : types of        { s[b+0] = fct result }
                ints : (i: integer );   { s[b+1] = return adr }
                reals :(r: real );      { s[b+2] = static link }
                bools :(b: boolean );   { s[b+3] = dynamic link }
                chars :(c: char )       { s[b+4] = table index }
			end;

  procedure dump; {程序运行时，卸出打印现场剖析信息(display,t,b以及运行栈S的内容)}
    var p,h3 : integer;
	begin
      h3 := tab[h2].lev;
      writeln(psout);
      writeln(psout);
      writeln(psout,'       calling ', tab[h2].name );
      writeln(psout,'         level ',h3:4);
      writeln(psout,' start of code ',pc:4);
      writeln(psout);
      writeln(psout);
      writeln(psout,' contents of display ');
      writeln(psout);
      for p := h3 downto 0 do
        writeln(psout,p:4,display[p]:6);
      writeln(psout);
      writeln(psout);
      writeln(psout,' top of stack  ',t:4,' frame base ':14,b:4);
      writeln(psout);
      writeln(psout);
      writeln(psout,' stack contents ':20);
      writeln(psout);
      for p := t downto 1 do
        writeln( psout, p:14, s[p].i:8);
      writeln(psout,'< = = = >':22)
    end; {dump }

  procedure inter0;
    begin
      case ir.f of
        0 : begin { load addrss } {LDA, 把变量地址装入栈顶}
              t := t + 1; {栈顶指针上移}
              if t > stacksize {栈溢出}
              then ps := stkchk {记录错误类型}
              else s[t].i := display[ir.x]+ir.y {x为层次，y为变量相对地址}
            end;
        1 : begin  { load value } {LOD, 把变量值装入栈顶}
              t := t + 1;
              if t > stacksize
              then ps := stkchk
              else s[t] := s[display[ir.x]+ir.y]
            end;
        2 : begin  { load indirect } {LDI, 把变量值间接装入栈顶}
              t := t + 1;
              if t > stacksize
              then ps := stkchk
              else s[t] := s[s[display[ir.x]+ir.y].i] {取两次值}
            end;
        3 : begin  { update display } {DIS, 更新display区}
              h1 := ir.y; {相对地址}
              h2 := ir.x; {层次}
              h3 := b; {基地址}
              repeat
                display[h1] := h3; {保存变量所在层次的数据区基地址}
                h1 := h1-1; {层次-1}
                h3 := s[h3+2].i
              until h1 = h2
            end;
        8 : case ir.y of {FCT, 标准函数(0..18,由y给定)}
              0 : s[t].i := abs(s[t].i); {整型求绝对值}
              1 : s[t].r := abs(s[t].r); {实型求绝对值}
              2 : s[t].i := sqr(s[t].i); {整型求平方}
              3 : s[t].r := sqr(s[t].r); {实型求平方}
              4 : s[t].b := odd(s[t].i); {判断奇偶性，奇数返回1}
              5 : s[t].c := chr(s[t].i); {ASCII码转换为字符}
              6 : s[t].i := ord(s[t].c); {字符转换为ASCII码}
              7 : s[t].c := succ(s[t].c); {后继字符，如a的后继是b}
              8 : s[t].c := pred(s[t].c); {前导字符}
              9 : s[t].i := round(s[t].r); {四舍五入取整}
              10 : s[t].i := trunc(s[t].r); {求整数部分}
              11 : s[t].r := sin(s[t].r); {求正弦}
              12 : s[t].r := cos(s[t].r); {求余弦}
              13 : s[t].r := exp(s[t].r); {求指数e^x}
              14 : s[t].r := ln(s[t].r);  {求自然对数ln(x)}
              15 : s[t].r := sqrt(s[t].r); {求平方根}
              16 : s[t].r := arcTan(s[t].r); {求反正切(...)}
              17 : begin {判断输入是否读完}
                     t := t+1;
                     if t > stacksize
                     then ps := stkchk
                     else s[t].b := eof(prd)
                   end;
              18 : begin {判断当前行是否读完}
                     t := t+1;
                     if t > stacksize
                     then ps := stkchk
                     else s[t].b := eoln(prd)
                   end;
            end;
        9 : s[t].i := s[t].i + ir.y; { offset } {INT, 将栈顶元素加上y}
      end { case ir.y }
    end; { inter0 }

  procedure inter1;
    var h3, h4: integer;
	begin
      case ir.f of
        10 : pc := ir.y ; { jump } {JMP, 无条件转移到y}
        11 : begin  { conditional jump } {JPC, 如栈顶内容为假，转移到y}
               if not s[t].b
			   then pc := ir.y;
               t := t - 1 {退栈}
			 end;
        12 : begin { switch } {SWT, 转移到y，查找情况表}
               h1 := s[t].i;
               t := t-1;
               h2 := ir.y;
               h3 := 0;
               repeat
                 if code[h2].f <> 13
                 then begin
                        h3 := 1;
                        ps := caschk {记录错误类型}
                      end
                 else if code[h2].y = h1
                      then begin
                             h3 := 1;
                             pc := code[h2+1].y
                           end
                      else h2 := h2 + 2
               until h3 <> 0
             end;
        14 : begin { for1up } {F1U, 增量步长型for循环体的入口测试}
               h1 := s[t-1].i; {获得计数变量的初值}
               if h1 <= s[t].i {如果初值小于等于终值}
               then s[s[t-2].i].i := h1 {开始循环，将初值赋给计数变量}
               else begin {结束循环}
                      t := t - 3; {退栈三格(s[t-2]~s[t]分别为：计数变量的地址、初值和终值)}
                      pc := ir.y {退出循环，按给定地址转移}
                    end
             end;
        15 : begin { for2up } {F2U, 增量步长型for循环体的再入测试}
               h2 := s[t-2].i; {获得计数变量的地址}
               h1 := s[h2].i+1; {获得计数变量自增一的值}
               if h1 <= s[t].i {如果初值小于等于终值}
               then begin 
                      s[h2].i := h1; {计数变量++}
                      pc := ir.y {按给定地址转移}
                    end
               else t := t-3; {结束循环，退栈三格}
             end;
        16 : begin  { for1down } {F1D, 减量步长型for循环体的入口测试}
               h1 := s[t-1].i;
               if h1 >= s[t].i
               then s[s[t-2].i].i := h1
               else begin
                      pc := ir.y;
                      t := t - 3
                    end
             end;
        17 : begin  { for2down } {F2D, 减量步长型for循环体的再入测试}
               h2 := s[t-2].i;
               h1 := s[h2].i-1;
               if h1 >= s[t].i
               then begin
                      s[h2].i := h1;
                      pc := ir.y
                    end
               else t := t-3;
             end;
        18 : begin  { mark stack } {MKS, 标记栈}
               h1 := btab[tab[ir.y].ref].vsize; {获得当前过程所需栈空间的大小}
               if t+h1 > stacksize {栈溢出}
               then ps := stkchk {记录错误类型}
               else begin
                      t := t+5; {留出5个单元的内务信息区}
                      s[t-1].i := h1-1; {在内务信息区的第四个单元存放vsize-1}
                      s[t].i := ir.y  {在第五个单元存放过程名在tab中的位置}
                    end
             end;
        19 : begin  { call } {CAL, 调用用户过程或函数}
               h1 := t-ir.y;  { h1 points to base 数据区基地址}
               h2 := s[h1+4].i;  { h2 points to tab 过程名在tab中的位置}
               h3 := tab[h2].lev; {当前过程的层次数}
               display[h3+1] := h1; {新建层次，并在display数组中保存当前层次的数据区基地址}
               h4 := s[h1+3].i+h1; {数据区顶端的局部变量单元}
               s[h1+1].i := pc; {RA 返回地址}
               s[h1+2].i := display[h3]; {SL 静态链}
               s[h1+3].i := b; {DL 动态链}
               for h3 := t+1 to h4 do {将数据区的局部变量全部初始化为0}
                 s[h3].i := 0;
               b := h1; {更新基地址寄存器，使其指向本数据区的基地址}
               t := h4; {更新栈顶指针，使其指向数据区顶端的局部变量单元}
               pc := tab[h2].adr;
               if stackdump
               then dump
             end;
      end { case }
    end; { inter1 }

  procedure inter2;
    begin
      case ir.f of
        20 : begin   { index1 } {IDX, 取下标变量地址(元素长度为1)}
               h1 := ir.y;  { h1 points to atab }
               h2 := atab[h1].low; {下标下界}
               h3 := s[t].i;
               if h3 < h2
               then ps := inxchk {记录错误类型}
               else if h3 > atab[h1].high {下标上界}
                    then ps := inxchk {记录错误类型}
                    else begin
                           t := t-1;
                           s[t].i := s[t].i+(h3-h2)
                         end
             end;
        21 : begin  { index } {IXX, 取下标变量地址}
               h1 := ir.y ; { h1 points to atab }
               h2 := atab[h1].low;
               h3 := s[t].i;
               if h3 < h2
               then ps := inxchk
               else if h3 > atab[h1].high
                    then ps := inxchk
                    else begin
                           t := t-1;
                           s[t].i := s[t].i + (h3-h2)*atab[h1].elsize
                         end
             end;
        22 : begin  { load block } {LDB, 装入块}
               h1 := s[t].i; {获得栈顶值}
               t := t-1;
               h2 := ir.y+t; {计算所需栈空间的大小}
               if h2 > stacksize {栈溢出}
               then ps := stkchk {记录错误类型}
               else while t < h2 do  {将h1指向的块的值装入栈顶}
                      begin
                        t := t+1;
                        s[t] := s[h1];
                        h1 := h1+1
                      end
             end;
        23 : begin  { copy block } {CPB, 复制块}
               h1 := s[t-1].i;
               h2 := s[t].i;
               h3 := h1+ir.y;
               while h1 < h3 do
                 begin
                   s[h1] := s[h2];
                   h1 := h1+1;
                   h2 := h2+1
                 end;
               t := t-2
             end; 
        24 : begin  { literal } {CPB, 装入字面常量}
               t := t+1;
               if t > stacksize
               then ps := stkchk {记录错误类型}
               else s[t].i := ir.y {将常量装入栈顶}
             end;
        25 : begin  { load real } {LDC, 装入实数}
               t := t+1;
               if t > stacksize
               then ps := stkchk {记录错误类型}
               else s[t].r := rconst[ir.y] {将实常量表中的相应地址的元素装入栈顶}
             end;
        26 : begin  { float } {FLT, 转换浮点数}
               h1 := t-ir.y;
               s[h1].r := s[h1].i
             end;
        27 : begin  { read } {RED, 读(y表示类型)}
               if eof(prd)
               then ps := redchk {记录错误类型}
               else case ir.y of
                      1 : read(prd, s[s[t].i].i); {整型}
                      2 : read(prd, s[s[t].i].r); {实型}
                      4 : read(prd, s[s[t].i].c); {字符型}
                    end;
               t := t-1
             end;
        28 : begin   { write string } {WRS, 写字符}
			   h1 := s[t].i;
               h2 := ir.y;
               t := t-1;
               chrcnt := chrcnt+h1;
			   if chrcnt > lineleng {行长度超过范围}
               then ps := lngchk; {记录错误类型}
               repeat
                 write(prr,stab[h2]);
                 h1 := h1-1;
                 h2 := h2+1
               until h1 = 0
             end;
        29 : begin  { write1 } {WRW, 写——隐含域宽}
               chrcnt := chrcnt + fld[ir.y];
               if chrcnt > lineleng
               then ps := lngchk 
               else case ir.y of
                      1 : write(prr,s[t].i:fld[1]);
                      2 : write(prr,s[t].r:fld[2]);
                      3 : if s[t].b {布尔型}
                          then write('true')
                          else write('false');
                      4 : write(prr,chr(s[t].i));
                    end;
               t := t-1
             end;
      end { case }
    end; { inter2 }

  procedure inter3;
    begin
      case ir.f of
        30 : begin { write2 } {WRU, 写——给定域宽}
               chrcnt := chrcnt+s[t].i;
               if chrcnt > lineleng
               then ps := lngchk
               else case ir.y of
                      1 : write(prr,s[t-1].i:s[t].i);
                      2 : write(prr,s[t-1].r:s[t].i);
                      3 : if s[t-1].b
                          then write('true')
                          else write('false');
                    end;
               t := t-2
             end;
        31 : ps := fin; {HLT, 停止}
        32 : begin  { exit procedure } {EXP, 退出过程}
               t := b-1; {退栈}
               pc := s[b+1].i; {PC指向RA}
               b := s[b+3].i {基地址指向DL}
             end;
        33 : begin  { exit function } {EXF, 退出函数}
               t := b; {退栈，保留函数名}
               pc := s[b+1].i;
               b := s[b+3].i
             end;
        34 : s[t] := s[s[t].i]; {LDT, 取栈顶单元内容为地址的单元内容}
        35 : s[t].b := not s[t].b; {NOT, 逻辑非}
        36 : s[t].i := -s[t].i; {MUS, 求负}
        37 : begin {WRR, 写实数——给定域宽}
               chrcnt := chrcnt + s[t-1].i;
               if chrcnt > lineleng
               then ps := lngchk
               else write(prr,s[t-2].r:s[t-1].i:s[t].i);
               t := t-3
             end;
        38 : begin  { store } {STO, 将栈顶内容存入以次栈顶为地址的单元}
               s[s[t-1].i] := s[t];
               t := t-2
             end;
        39 : begin {EQR, 实型等于比较}
               t := t-1;
               s[t].b := s[t].r=s[t+1].r
             end;
      end { case }
    end; { inter3 }

  procedure inter4;
    begin
      case ir.f of
        40 : begin {NER, 实型不等比较}
               t := t-1;
               s[t].b := s[t].r <> s[t+1].r
             end;
        41 : begin {LSR, 实型小于比较}
               t := t-1;
               s[t].b := s[t].r < s[t+1].r
             end;
        42 : begin {LER, 实型小于等于比较}
               t := t-1;
               s[t].b := s[t].r <= s[t+1].r
             end;
        43 : begin {GTR, 实型大于比较}
               t := t-1;
               s[t].b := s[t].r > s[t+1].r
             end;
        44 : begin {GER, 实型大于等于比较}
               t := t-1;
               s[t].b := s[t].r >= s[t+1].r
             end;
        45 : begin {EQL, 整型相等比较}
               t := t-1;
               s[t].b := s[t].i = s[t+1].i
             end;
        46 : begin {NEQ, 整型不等比较}
               t := t-1;
               s[t].b := s[t].i <> s[t+1].i
             end;
        47 : begin {LSS, 整型小于比较}
               t := t-1;
               s[t].b := s[t].i < s[t+1].i
             end;
        48 : begin {LEQ, 整型小于等于比较}
               t := t-1;
               s[t].b := s[t].i <= s[t+1].i
             end;
        49 : begin {GRT, 整型大于比较}
               t := t-1;
               s[t].b := s[t].i > s[t+1].i
             end;
      end { case }
    end; { inter4 }

  procedure inter5;
    begin
      case ir.f of
        50 : begin {GEQ, 整型大于等于比较}
               t := t-1;
               s[t].b := s[t].i >= s[t+1].i
             end;
        51 : begin {ORR, 逻辑或}
               t := t-1;
               s[t].b := s[t].b or s[t+1].b
             end;
        52 : begin {ADD, 整型加}
               t := t-1;
               s[t].i := s[t].i+s[t+1].i
             end;
        53 : begin {SUB, 整型减}
               t := t-1;
               s[t].i := s[t].i-s[t+1].i
             end;
        54 : begin {ADR, 实型加}
               t := t-1;
               s[t].r := s[t].r+s[t+1].r;
             end;
        55 : begin {SUR, 实型减}
               t := t-1;
               s[t].r := s[t].r-s[t+1].r;
             end;
        56 : begin {AND, 逻辑与}
               t := t-1;
               s[t].b := s[t].b and s[t+1].b
             end;
        57 : begin {MUL, 整型乘}
               t := t-1;
               s[t].i := s[t].i*s[t+1].i
             end;
        58 : begin {DIV, 整型除} 
               t := t-1;
               if s[t+1].i = 0
               then ps := divchk
               else s[t].i := s[t].i div s[t+1].i
             end;
        59 : begin {MOD, 取模(求余)}
               t := t-1;
               if s[t+1].i = 0
               then ps := divchk
               else s[t].i := s[t].i mod s[t+1].i
             end;
      end { case }
    end; { inter5 }

  procedure inter6;
    begin
      case ir.f of
        60 : begin {MUR, 实型乘}
               t := t-1;
               s[t].r := s[t].r*s[t+1].r;
             end;
        61 : begin {DIR, 实型除}
               t := t-1;
               s[t].r := s[t].r/s[t+1].r;
             end;
        62 : if eof(prd) {RDL, readln(读完一行)}
             then ps := redchk {记录错误类型}
             else readln;
        63 : begin {WRL, writeln(换行写)}
               writeln(prr);
               lncnt := lncnt+1;
               chrcnt := 0;
               if lncnt > linelimit {行数超出范围}
               then ps := linchk {记录错误类型}
             end
      end { case };
    end; { inter6 }
  begin { interpret }
    s[1].i := 0;
    s[2].i := 0;
    s[3].i := -1;
    s[4].i := btab[1].last;
    display[0] := 0;
    display[1] := 0;
    t := btab[2].vsize-1;
    b := 0;
    pc := tab[s[4].i].adr;
    lncnt := 0;
    ocnt := 0;
    chrcnt := 0;
    ps := run;
    fld[1] := 10;
    fld[2] := 22;
    fld[3] := 10;
    fld[4] := 1;
    repeat {逐条执行PCODE}
      ir := code[pc];
      pc := pc+1;
      ocnt := ocnt+1;
      case ir.f div 10 of
		0 : inter0;
        1 : inter1;
        2 : inter2;
        3 : inter3;
        4 : inter4;
		5 : inter5;
        6 : inter6;
      end; { case }
    until ps <> run;

    if ps <> fin
    then begin
           writeln(prr);
           write(prr, ' halt at', pc :5, ' because of ');
           case ps of {根据ps保存的错误类型输出相应错误信息}
             caschk  : writeln(prr,'undefined case');
             divchk  : writeln(prr,'division by 0');
             inxchk  : writeln(prr,'invalid index');
             stkchk  : writeln(prr,'storage overflow');
             linchk  : writeln(prr,'too much output');
             lngchk  : writeln(prr,'line too long');
             redchk  : writeln(prr,'reading past end or file');
           end;
           h1 := b;
           blkcnt := 10;    { post mortem dump }
           repeat
             writeln( prr );
             blkcnt := blkcnt-1;
             if blkcnt = 0
             then h1 := 0;
             h2 := s[h1+4].i;
             if h1 <> 0
             then writeln( prr, '',tab[h2].name, 'called at', s[h1+1].i:5);
             h2 := btab[tab[h2].ref].last; {找到该分程序当前(最后一个)标识符在tab表中的位置}
             while h2 <> 0 do {找到了}
               with tab[h2] do
                 begin
                   if obj = vvariable {是变量}
                   then if typ in stantyps {类型合法}
                        then begin
                               write(prr,'',name,'='); {打印变量名}
                               if normal {不是变量形参}
                               then h3 := h1+adr {获得地址}
                               else h3 := s[h1+adr].i; {获得运行栈中相应地址的值}
                               case typ of {根据类型打印相应值}
                                 ints : writeln(prr,s[h3].i); 
                                 reals: writeln(prr,s[h3].r); 
                                 bools: if s[h3].b
                                        then writeln(prr,'true')
                                        else writeln(prr,'false');
                                 chars: writeln(prr,chr(s[h3].i mod 64 ))
                               end
                             end;
                   h2 := link {找到该分程序上一个标识符在tab表中的位置}
                 end;
             h1 := s[h1+3].i
           until h1 < 0
         end;
    writeln(prr);
    writeln(prr,ocnt,' steps');
  end; { interpret }



procedure setup; {建立保留字表key、保留字对应类型表ksy和特定字符表sps}
  begin
    key[1] := 'and       ';
    key[2] := 'array     ';
    key[3] := 'begin     ';
    key[4] := 'case      ';
    key[5] := 'const     ';
    key[6] := 'div       ';
    key[7] := 'do        ';
    key[8] := 'downto    ';
    key[9] := 'else      ';
    key[10] := 'end       ';
    key[11] := 'for       ';
    key[12] := 'function  ';
    key[13] := 'if        ';
    key[14] := 'mod       ';
    key[15] := 'not       ';
    key[16] := 'of        ';
    key[17] := 'or        ';
    key[18] := 'procedure ';
    key[19] := 'program   ';
    key[20] := 'record    ';
    key[21] := 'repeat    ';
    key[22] := 'then      ';
    key[23] := 'to        ';
    key[24] := 'type      ';
    key[25] := 'until     ';
    key[26] := 'var       ';
    key[27] := 'while     ';

    ksy[1] := andsy;
    ksy[2] := arraysy;
    ksy[3] := beginsy;
    ksy[4] := casesy;
    ksy[5] := constsy;
    ksy[6] := idiv;
    ksy[7] := dosy;
    ksy[8] := downtosy;
    ksy[9] := elsesy;
    ksy[10] := endsy;
    ksy[11] := forsy;
    ksy[12] := funcsy;
    ksy[13] := ifsy;
    ksy[14] := imod;
    ksy[15] := notsy;
    ksy[16] := ofsy;
    ksy[17] := orsy;
    ksy[18] := procsy;
    ksy[19] := programsy;
    ksy[20] := recordsy;
    ksy[21] := repeatsy;
    ksy[22] := thensy;
    ksy[23] := tosy;
    ksy[24] := typesy;
    ksy[25] := untilsy;
    ksy[26] := varsy;
    ksy[27] := whilesy;


    sps['+'] := plus;
    sps['-'] := minus;
    sps['*'] := times;
    sps['/'] := rdiv;
    sps['('] := lparent;
    sps[')'] := rparent;
    sps['='] := eql;
    sps[','] := comma;
    sps['['] := lbrack;
    sps[']'] := rbrack;
    sps[''''] := neq;
    sps['!'] := andsy;
    sps[';'] := semicolon;
  end { setup };

procedure enterids; {在tab表中登录全部标准类型、函数和过程名及有关信息}
  begin
    enter('          ',vvariable,notyp,0); { sentinel }
    enter('false     ',konstant,bools,0);
    enter('true      ',konstant,bools,1);
    enter('real      ',typel,reals,1);
    enter('char      ',typel,chars,1);
    enter('boolean   ',typel,bools,1);
    enter('integer   ',typel,ints,1);
    enter('abs       ',funktion,reals,0);
    enter('sqr       ',funktion,reals,2);
    enter('odd       ',funktion,bools,4);
    enter('chr       ',funktion,chars,5);
    enter('ord       ',funktion,ints,6);
    enter('succ      ',funktion,chars,7);
    enter('pred      ',funktion,chars,8);
    enter('round     ',funktion,ints,9);
    enter('trunc     ',funktion,ints,10);
    enter('sin       ',funktion,reals,11);
    enter('cos       ',funktion,reals,12);
    enter('exp       ',funktion,reals,13);
    enter('ln        ',funktion,reals,14);
    enter('sqrt      ',funktion,reals,15);
    enter('arctan    ',funktion,reals,16);
    enter('eof       ',funktion,bools,17);
    enter('eoln      ',funktion,bools,18);
    enter('read      ',prozedure,notyp,1);
    enter('readln    ',prozedure,notyp,2);
    enter('write     ',prozedure,notyp,3);
    enter('writeln   ',prozedure,notyp,4);
    enter('          ',prozedure,notyp,0);
  end;


begin  { main }      
  setup; {初始化}
  constbegsys := [ plus, minus, intcon, realcon, charcon, ident ]; {常量}
  typebegsys := [ ident, arraysy, recordsy ]; {类型}
  blockbegsys := [ constsy, typesy, varsy, procsy, funcsy, beginsy ]; {分程序}
  facbegsys := [ intcon, realcon, charcon, ident, lparent, notsy ]; {因子}
  statbegsys := [ beginsy, ifsy, whilesy, repeatsy, forsy, casesy ]; {表达式}
  stantyps := [ notyp, ints, reals, bools, chars ]; {函数结果类型}
  lc := 0; {P代码表code的索引变量，pc} 
  ll := 0; {当前行长度}
  cc := 0; {当前行指针}
  ch := ' '; {当前符号}
  errpos := 0; {错误位置}
  errs := []; {错误集合}
  writeln( 'NOTE input/output for users program is console : ' ); {显示提示信息}
  writeln;
  write( 'Source input file ?'); {提示用户输入将要编译的源程序文件名}
  readln( inf );
  assign( psin, inf );
  reset( psin );
  write( 'Source listing file ?'); {提示用户输入编译输出的文件名}
  readln( outf );
  assign( psout, outf ); 
  rewrite( psout );
  assign ( prd, 'con' );
  write( 'result file : ' ); {提示用户输入编译结果的文件名}
  readln( fprr );
  assign( prr, fprr );
  reset ( prd );
  rewrite( prr );

  t := -1; {符号表tab的索引变量}
  a := 0;  {数组信息向量表atab的索引变量}
  b := 1; {分程序表btab的索引变量}
  sx := 0; {字符串常量表stab的索引变量}
  c2 := 0; {实向量表rconst的索引变量}
  display[0] := 1;
  iflag := false;
  oflag := false;
  skipflag := false;
  prtables := false;
  stackdump := false;

  insymbol; {读取下一单词符号，处理注释行}

  if sy <> programsy 
  then error(3) {报错：程序必须以program开始}
  else begin
         insymbol;
         if sy <> ident 
         then error(2) {报错：应是标识符}
         else begin
                progname := id; {存储程序名}
                insymbol;
                if sy <> lparent
                then error(9) {报错：应是'('}
                else repeat
                       insymbol;
                       if sy <> ident
                       then error(2) {报错：应是标识符}
                       else begin {必须由input或output开始}
                              if id = 'input     '
                              then iflag := true
                              else if id = 'output    '
                                   then oflag := true
                                   else error(0); {报错：该标识符未定义}
                              insymbol
                            end
                     until sy <> comma;
                if sy = rparent
                then insymbol
                else error(4); {报错；应是')'}
                if not oflag then error(20) {报错：程序头部未包含参数'input'或'output'}
              end
       end;
  enterids; {在tab表中登录全部标准类型、函数和过程名及有关信息}
  with btab[1] do
    begin
      last := t;
      lastpar := 1;
      psize := 0;
      vsize := 0;
    end;
  block( blockbegsys + statbegsys, false, 1 ); {分析处理分程序}
  if sy <> period
  then error(2); {报错：应是标识符}
  emit(31);  { halt 生成停止的指令}
  if prtables
  then printtables; {打印编译生成的符号表、分程序表、实常数表以及P代码}
  if errs = []
  then interpret {生成P代码解释执行程序}
  else begin
         writeln( psout );
         writeln( psout, 'compiled with errors' );
         writeln( psout );
         errormsg; {打印被编译源程序中出错信息的摘要}
       end;
  writeln( psout );
  close( psout );
  close( prr )
end.                                              
