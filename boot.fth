(( code: 65536 cells, then vars ))
65536 cell * memory + (vha) ! (ha) @ (la) @
: here  (ha)  @ ;
: vhere (vha) @ ;
: last  (la)  @ ;
: ->code ( n--a ) cell * memory + ;
: , ( n-- ) here ->code ! 1 (ha) +! ;
: allot ( n-- ) (vha) +! ;
: const ( n-- ) addword (lit) , , (exit) , ;
: var   ( n-- ) vhere const allot ;

(( these are used by "rb" ))
const -la-    const -ha-    vhere const -vha-

: immediate 1 last cell + c! ; immediate
: inline    2 last cell + c! ; immediate

: begin here ; immediate
: while  (jmpnz)  , , ; immediate
: -while (njmpnz) , , ; immediate
:  until (jmpz)   , , ; immediate
: -until (njmpz)  , , ; immediate
: again  (jmp)    , , ; immediate

: if (jmpz)   , here 0 ,  ; immediate
: -if (njmpz) , here 0 ,  ; immediate
: if0 (jmpnz) , here 0 ,  ; immediate
: then here swap ->code ! ; immediate

: a+    a@+ drop  ; inline
: a-    a@- drop  ; inline
: c@a   a@  c@    ; inline
: c@a+  a@+ c@    ; inline
: c@a-  a@- c@    ; inline
: c!a   a@  c!    ; inline
: c!a+  a@+ c!    ; inline
: c!a-  a@- c!    ; inline
: adrop a> drop   ; inline

: b+    b@+ drop  ; inline
: b-    b@- drop  ; inline
: c!b   b@ c!     ; inline
: c!b+  b@+ c!    ; inline
: c@b+  b@+ c@    ; inline
: bdrop b> drop   ; inline

( STATES/MODES )
: define  1 ; inline
: compile 2 ; inline
: interp  3 ; inline
: comment 4 ; inline
: comp? state @ compile = ;

( quote subroutine )
: t4 ( --a ) vhere dup >b  >in @ 1+ >a
   begin
      c@a '"' = if
         0 c!b+  a> 1+ >in !
         comp? if0 bdrop exit then
         b> (vha) ! (lit) , , exit
      then c@a+ c!b+
   again ;

: z"  t4 ; immediate
: ."  t4 comp? if (ztype) , exit then ztype ; immediate

( Files )
: fopen-r z" rb"  fopen ;
: fopen-w z" wb"  fopen ;

( number format / print )
: #neg 0 >a dup 0 < if com 1+ a+ then ;
: <#   ( n--n' ) #neg last 32 - >b 0 c!b ;
: hold ( c--n )  b- c!b ;
: #n   ( n-- )   '0' + dup '9' > if 7 + then hold ;
: #.   ( -- )    '.' hold ;
: #    ( n--n' ) base @ /mod swap #n ;
: #s   ( n-- )   begin # -while drop ;
: #>   ( --a )   a> if '-' hold then b> ;
: (.) ( n-- ) <# #s #> ztype ;
: .   ( n-- ) (.) : space 32 emit ;

: 2dup  over over ; inline
: 2drop drop drop ; inline
: ?dup  -if dup then ;
: min   ( n m--n|m ) 2dup > if swap then drop ;
: max   ( n m--n|m ) 2dup < if swap then drop ;
: fill  ( a n c-- )  >r swap 1- r> for 2dup 1+ c! next 2drop ;
: s-end ( s--e )     dup s-len + ; inline
: s-cat ( d s--d )   over s-end swap s-cpy drop ;

( Blocks )
: blk-max 1023 ; inline
: blk-sz  1024 ; inline
memory mem-sz + 2 1024 dup * * - const blks
cell var t0  1 t0 !
: blk@ ( --n ) t0 @ ;
: blk! ( n-- ) 0 max blk-max min t0 ! ;
: blk-data ( --a ) blk@ blk-sz * blks + ;
: blk-end  ( --a ) blk-data blk-sz 1- + ;
: blk-clr  ( -- )  blk-data blk-sz 0 fill ;
16 var t1
: blk-fn ( --a ) t1 z" block-" s-cpy blk@ <# # # #s #> s-cat z" .fth" s-cat ;
: blk-rd ( -- ) blk-clr  blk-fn fopen-r ?dup
   if >a blk-data blk-sz a@ fread drop a> fclose then ;
: blk-wr ( -- ) blk-fn fopen-w ?dup
   if >a blk-data blk-sz a@ fwrite drop a> fclose then ;

( load )
: t1  0 blk-end c! ;
: load ( n-- )  blk! blk-rd blk-data t1 outer ;
: load-next  blk@ 1+ blk! blk-rd blk-data t1 >in ! ;

( everything from here on could be moved to blocks )

: source-loc memory 100000 + ;
: rb ( reboot )
   -vha- (vha) !  -la- (la) !  -ha- (ha) !
   z" boot.fth" fopen-r -if dup then if >a
      source-loc >b
      50000 for 0 c!b+ next bdrop
      source-loc 50000 a@ fread drop a> fclose
      source-loc >in !
   then ;

( T reg/stack words )
: t+    t@+ drop  ; inline
: t-    t@- drop  ; inline
: c@t   t@  c@    ; inline
: c@t+  t@+ c@    ; inline
: c@t-  t@- c@    ; inline
: c!t   t@  c!    ; inline
: c!t+  t@+ c!    ; inline
: c!t-  t@- c!    ; inline
: t@+c  t@ dup cell + t! ;
: tdrop t> drop   ; inline
: atdrop adrop tdrop ;

: val   ( -- )  addword (lit) , 111 , (exit) , ;
: (val) ( -- )  here 2 - ->code const ;
: ->file ( fh-- ) (output-fp) ! ;
: ->stdout 0 ->file ;
: bl 32 ; inline
: tab 9 emit ; inline
: cr 13 emit 10 emit ; inline
: spaces for bl emit next ; inline
: negate com 1+ ; inline
: abs dup 0 < if negate then ;

: .nw  ( n w-- ) >r <# r> ?dup if 1- for # next then #s #> ztype ;
: .nwb ( n w b-- ) base @ >b base ! .nw b> base ! ;
: .2 ( n-- ) 2 .nw ;
: .3 ( n-- ) 3 .nw ;
: .4 ( n-- ) 4 .nw ;
: hex     $10 base ! ;
: decimal #10 base ! ;
: binary  %10 base ! ;
: .hex  ( n-- ) base @ >t hex .2 t> base ! ;
: .dec  ( n-- ) base @ >t decimal .3 t> base ! ;
: .bin  ( n-- ) base @ >t binary . t> base ! ;
: .$hex ( n-- ) base @ >t '$' emit hex (.) t> base ! ;
: .#dec ( n-- ) base @ >t '#' emit decimal (.) t> base ! ;
: .%bin ( n-- ) base @ >t '%' emit binary (.) t> base ! ;

: execute ( xt-- ) >r ;
: :noname here compile state ! ;
: cells cell * ; inline
: cell+ cell + ; inline
: 2+ 1+ 1+ ; inline
: 2* dup + ; inline
: 2/ 2 / ; inline
: mod /mod drop ; inline
: ? @ . ;
: nip  swap drop ; inline
: tuck swap over ; inline
: <= ( a b--f ) > 0= ;
: >= ( a b--f ) < 0= ;
: btw  ( n l h--f ) >a over <  swap a> <  and ;
: btwi ( n l h--f ) >a over <= swap a> <= and ;
: vc, vhere c! 1 allot ;
: v, vhere ! cell allot ;

: 0sp 0 (dsp) ! ;
: unloop (lsp) @ 3 - 0 max (lsp) ! ;
: depth (dsp) @ 1- ;
: lpar '(' emit ; inline
: rpar ')' emit ; inline
: .s lpar space depth ?dup if
      for i 1+ cells dstk + @ . next
   then rpar ;

( strings )
: s-catc ( dst ch--dst )  over s-end tuck c! 0 swap 1+ c! ;
: s-catn ( dst num--dst ) <# #s #> over s-end swap s-cpy drop ;
: s-scat ( src dst--dst ) swap s-cat ;
: s-rtrim ( str--str ) dup >b b@ s-end 1- >a begin
      a@ b@ < if 0 b> c! adrop exit then
      c@a- bl > if 0 a> 2+ c! bdrop exit then
   again ;
: pad  ( --a ) vhere $100 + ;
: pad2 ( --a ) vhere $200 + ;
: pad3 ( --a ) vhere $300 + ;

( words )
: dict-end memory mem-sz + 1- 7 com and ;
: de>xt    @ ; inline
: de>flags cell + c@ ; inline
: de>len   cell + 1+ c@ ; inline
: de>name  cell + 2+ ; inline
: .word de>name ztype ;
: .de-word .word t@+ 9 > if 0 t! cr exit then tab ;

: words last >a 1 >t 0 >b begin
   a@ de>len  7 > if t+ then
   a@ de>len 12 > if t+ then
   a@ .de-word a@ de-sz + a! b+
   a@ dict-end < while
   lpar b> . ." words)" adrop ;
: words-n last >t for i 8 mod if0 cr then t@ .word tab t@ de-sz + t! next tdrop ;

( Screen )
: csi          27 emit '[' emit ;
: ->cr      ( c r-- ) csi (.) ';' emit (.) 'H' emit ;
: ->rc      ( r c-- ) swap ->cr ;
: cls          csi ." 2J" 1 dup ->cr ;
: clr-eol      csi ." 0K" ;
: cur-on       csi ." ?25h" ;
: cur-off      csi ." ?25l" ;
: cur-block    csi ." 2 q" ;
: cur-bar      csi ." 5 q" ;

: color ( bg fg-- ) csi (.) ';' emit (.) 'm' emit ;
: bg    ( color-- ) csi ." 48;5;" (.) 'm' emit ;
: fg    ( color-- ) csi ." 38;5;" (.) 'm' emit ;
: c-red 203 ; inline
: black   0 fg ;      : red    c-red fg ;
: green  40 fg ;      : yellow 226 fg ;
: blue   63 fg ;      : purple 201 fg ;
: cyan  117 fg ;      : grey   246 fg ;
: white 255 fg ;
: colors 31 >a 7 for a@ fg ." color #" a@+ . cr next white adrop ;

(( Keys ))
256  59 + const key-f1
256  60 + const key-f2
256  61 + const key-f3
256  62 + const key-f4
256  71 + const key-home   (( VT: 27 91 72 ))
256  72 + const key-up     (( VT: 27 91 65 ))
256  73 + const key-pgup   (( VT: 27 91 53 126 ))
256  75 + const key-left   (( VT: 27 91 68 ))
256  77 + const key-right  (( VT: 27 91 67 ))
256  79 + const key-end    (( VT: 27 91 70 ))
256  80 + const key-down   (( VT: 27 91 66 ))
256  81 + const key-pgdn   (( VT: 27 91 54 126 ))
256  82 + const key-ins    (( VT: 27 91 50 126 ))
256  83 + const key-del    (( VT: 27 91 51 126 ))
256 119 + const key-chome  (( VT: 27 91 ?? ??? ))
256 117 + const key-cend   (( VT: 27 91 ?? ??? ))
: vk2 ( --k ) key 126 = if0 27 exit then
   a@ 50 = if key-ins   exit then
   a@ 51 = if key-del   exit then
   a@ 53 = if key-pgup  exit then
   a@ 54 = if key-pgdn  exit then    27 ;
: vk1 ( --k ) key a!
   a@ 68 = if key-left  exit then
   a@ 67 = if key-right exit then
   a@ 65 = if key-up    exit then
   a@ 66 = if key-down  exit then
   a@ 72 = if key-home  exit then
   a@ 70 = if key-end   exit then
   a@ 49 > a@ 55 < and if vk2 exit then   27 ;
: vt-key ( --k )  key dup 91 = if drop vk1 exit then
   79 = if key 80 - key-f1 + exit then 27 ;
: vkey ( --k ) key dup if0 drop #256 key + exit then ( Windows FK )
   dup 224 = if drop #256 key + exit then ( Windows )
   dup  27 = if drop vt-key exit then ; ( VT )

( Accept )
: printable? ( c--f ) 31 127 btw ;
: bs 8 emit ; inline
: accept ( dst-- ) dup >r >t 0 >a
  begin key a!
      a@   3 =  a@ 27 = or if 0 r> c! atdrop exit then
      a@  13 = if 0 c!t atdrop rdrop exit then
      a@   8 = if 127 a! then ( Windows: 8=backspace )
      a@ 127 = if r@ t@ < if t- bs space bs then then
      a@ printable? if a@ dup c!t+ emit then
  again ;

(( Editor ))
: rows 16 ; inline       : cols 64 ; inline
: last-row 15 ; inline   : last-col 63 ; inline
vhere const ed-colors
219   vc, (( 0: default - purple ))
c-red vc, (( 1: define  - red ))
 76   vc, (( 2: compile - green ))
226   vc, (( 3: interp  - yellow ))
255   vc, (( 4: comment - white ))

: ed-color@ ( n-- ) ed-colors + c@ ;
: ed-color! ( fg n-- ) ed-colors + c! ;

cell var (r)  : row! (r) ! ;    : row@ (r) @ ;
cell var (c)  : col! (c) ! ;    : col@ (c) @ ;
blk-sz var ed-blk
ed-blk blk-sz + 1- const ed-eob
: norm-pos ( pos--new ) ed-blk max ed-eob min ;
: pos->rc ( pos-- ) norm-pos ed-blk - cols /mod row! col! ;
: cr->pos ( col row--pos ) cols * + ed-blk + ed-eob min ;
: rc->pos ( --pos ) col@ row@ cr->pos ;
: ed-eol  ( --pos ) last-col row@ cr->pos ;
1 var t1  : mode!  t1 c! ;  : mode@  t1 c@ ;
1 var t1  : show?  t1 c@ ;  : shown  0 t1 c! ;  : show!  1 t1 c! ;
1 var t2  : dirty? t2 c@ ;  : clean! 0 t2 c! ;  : dirty! 1 t2 c! show! ;
: mv ( r c-- )  (c) +! (r) +!  rc->pos  pos->rc ;
: ed-c! ( ch col row-- ) cr->pos c! dirty! ;
: ed-ch! ( c-- ) col@ row@ ed-c! ;
: ed-ch@ ( --c ) rc->pos c@ ;
: ed-bl ( -- ) ed-blk >a blk-sz for c@a if0 bl c!a then a+ next adrop ;
: blk->ed ( -- ) blk-data ed-blk blk-sz cmove ed-bl ;
: ed-load ( -- ) blk-rd blk->ed clean! show! 0 0 row! col! ;
: ->norm  0 mode! ;    : norm?  mode@  0 = ;
: ->repl  1 mode! ;    : repl?  mode@  1 = ;
: ->ins   2 mode! ;    : ins?   mode@  2 = ;
: q!     99 mode! ;    : quit?  mode@ 99 = ;  
: ed-emit ( ch-- )
   dup 31 > if emit exit then ( regular char )
   dup  0 5 btw if dup ed-color@ fg then ( change color )
   drop space ;
: .scr 1 dup ->rc white ed-blk >a rows for
      cols for c@a+ ed-emit next cr
   next adrop ;
: ->cur  col@ 1+ row@ 1+ ->cr ;
: ->foot 1 rows 1+ ->cr ;
: ->cmd ->foot cr ;
: .foot ->foot cyan ." Block #" blk@ .
   bl dirty? if drop '*' then emit space
   norm? if green  ."  -norm- "    then
   repl? if yellow ."  -replace- " then
   ins?  if purple ."  -insert- "  then white
   lpar row@ 1+ (.) ',' emit col@ 1+ . '-' emit space
   rc->pos c@ dup .#dec '/' emit .$hex rpar clr-eol ;
: show cur-off show? if .scr shown then .foot ->cur cur-on ;
: mv-left  col@ 1- 0 max col! ;
: mv-right col@ 1+ last-col min col! ;
: mv-up    row@ 1- 0 max row! ;
: mv-down  row@ 1+ last-row min row! ;
: mv-end   last-col col! begin
      col@ 0= ed-ch@ bl > or if exit then mv-left
   again ;
: ins-bl  rc->pos dup 1+ last-col col@ - cmove bl ed-ch! ;
: ins-bl2 rc->pos dup 1+ ed-eob over - 1+ cmove bl ed-ch! ;
: replace-char a@ printable? if a@ ed-ch! mv-right then ;
: insert-char  a@ printable? if ins-bl a@ ed-ch! mv-right then ;
: del-c rc->pos >a a@ 1+ a> cols col@ - cmove dirty! 32 ed-eol c! ;
: del-z rc->pos >a a@ 1+ a@ ed-eob a> - cmove dirty! 32 ed-eob c! ;
: clr-line  rc->pos col@ - cols bl fill dirty! ;
: clr-toend rc->pos cols col@ - bl fill dirty! ;
: ed-goto ( blk-- ) blk! ed-load ;
: insert-line  row@ last-row < if
      ed-eob >a  a@ cols - >t  0 row@ cr->pos >r
      begin c@t- c!a- t@ r@ < until atdrop rdrop
   then clr-line ;
: ?insert-line ins? if0 mv-down 0 col! exit then
   mv-down insert-line mv-up
   rc->pos pad3 cols cmove clr-toend
   mv-down 0 col! pad3 rc->pos cols cmove ;
: yanked pad2 ;
: yank-line  0 row@ cr->pos yanked cols cmove ;
: put-line   yanked 0 row@ cr->pos cols cmove ;
: del-line yank-line row@ rows < if
      0 row@ cr->pos >t  t@ cols + >a  ed-eob >r
      begin c@a+ c!t+  a@ r@ > until atdrop rdrop
   then row@  rows 1- row!  clr-line  row! ;
: join-lines row@ last-row < if
      col@ >t mv-down del-line mv-up
      yanked >b mv-end begin
         mv-right c@b+ ed-ch! col@ last-col <
      while bdrop t> col!
   then ;
: ed-prev-word rc->pos 1- >t begin
      t@ ed-blk < if t> pos->rc exit then
      c@t- 33 < if t> 1+ pos->rc exit then
   again ;
: ed-next-word rc->pos 1+ >t begin
      t@ ed-eob > if t> pos->rc exit then
      c@t+ 33 < if t> 1- pos->rc exit then
   again ;
: rl blk@ ed-goto ;
: w! ed-blk blk-data blk-sz cmove blk-wr clean! ;
: w  dirty? if w! then ;
: q  dirty? if0 q! exit then ." use 'wq' or 'q!'" ;
: wq w q ;
: ed! w blk! ed-load ;
: do-cmd ->cmd ':' emit clr-eol pad accept
   space pad outer show! ;
: next-pg w blk@ 1+ ed-goto ;
: prev-pg w blk@ 1- 0 max ed-goto ;

( switch: case-table process )
: case   ( ch-- )  v, find drop v, ;   ( case-table entry - single word )
: case!  ( ch-- )  v, here v, compile state ! ;   ( case-table entry - code )
: switch ( tbl-- ) >t begin
   t@ @ if0 tdrop exit then
   t@+c @ a@ = if t> @ >r exit then
   t@ cell+ t! again ;

(( delete commands ))
vhere const ed-del-cases
'x'    case   del-c
'Z'    case   del-z
'd'    case   del-line
'$'    case   clr-toend
0 v, 0 v, (( end ))

(( VI-like commands ))
vhere const ed-ctrl-cases
  3         case   ->norm
  8         case!  mv-left ins? if del-c then ;
  9         case!  0 8 mv ;
 10         case   mv-down
 12         case   mv-right
 11         case   mv-up
 13         case   ?insert-line
 24         case   del-c
 27         case   ->norm
127         case!  mv-left ins? if del-c then ;
key-left    case   mv-left
key-right   case   mv-right
key-up      case   mv-up
key-down    case   mv-down
key-home    case!  0 col! ;
key-end     case   mv-end
key-ins     case!  ins? if ->norm exit then ->ins ;
key-del     case   del-c
key-pgup    case   prev-pg
key-pgdn    case   next-pg
key-chome   case!  0 dup row! col! ;
key-cend    case!  rows 1- row! 0 col! ;
key-f1      case!  define  ed-ch! ;
key-f2      case!  compile ed-ch! ;
key-f3      case!  interp  ed-ch! ;
key-f4      case!  comment ed-ch! ;
0 v, 0 v, (( end ))

vhere const ed-cases
'j'  case   mv-down
'k'  case   mv-up
'h'  case   mv-left
'l'  case   mv-right
bl   case   mv-right
'1'  case!  define  ed-ch! ;
'2'  case!  compile ed-ch! ;
'3'  case!  interp  ed-ch! ;
'4'  case!  comment ed-ch! ;
'_'  case!  0 col! ;
'$'  case   mv-end
':'  case!  do-cmd ;
'r'  case!  red '?' emit key a! replace-char ;
'R'  case   ->repl
'x'  case   del-c
'X'  case!  mv-left del-c ;
'a'  case!  mv-right ->ins ;
'A'  case!  mv-end mv-right ->ins ;
'b'  case   ins-bl
'B'  case   ins-bl2
'C'  case!  clr-toend ->ins ;
'd'  case!  show! red '?' emit key a! ed-del-cases switch ;
'D'  case   clr-toend
'i'  case   ->ins
'I'  case!  0 col! ->ins ;
'J'  case   join-lines
'p'  case!  mv-down insert-line put-line ;
'P'  case!  insert-line put-line ;
'q'  case!  0 8 mv ;
'Q'  case!  0 8 negate mv ;
'O'  case!  insert-line ->ins ;
'o'  case!  mv-down insert-line ->ins ;
'w'  case   ed-next-word
'W'  case   ed-prev-word
'Y'  case   yank-line
'Z'  case   del-z
'g'  case!  rows 0  row! 0 col! ;
'G'  case!  rows 1- row! 0 col! ;
'+'  case   next-pg
'-'  case   prev-pg
'#'  case!  cls show! ;
0 v, 0 v, (( end ))

: process-key ( --, key is in a )
   a@ bl < a@ 126 > or if ed-ctrl-cases switch exit then
   ins?  if insert-char exit then
   repl? if replace-char exit then
   ed-cases switch ;
: ed-loop begin show vkey >a process-key adrop quit? until ;
: ed-init cls 0 mode! 0 dup row! col! blk@ ed-goto ;
: ed  ( -- ) ed-init ed-loop ->cmd interp state ! ;
: edit ( n-- )  blk! ed ;

( fgl: forget the last word )
: fgl last dup de-sz + (la) ! de>xt (ha) ! ;  

cell var t0    cell var t1    cell var t2
: marker here t0 ! last t1 ! vhere t2 ! ;
: forget t0 @ (ha) ! t1 @ (la) ! t2 @ (vha) ! ;

: .version ." cf v" version <# # # #. # # #. #s #> ztype ;

marker
green .version white ."  - Chris Curl " cr
yellow ."  Memory: " white mem-sz . ." bytes" cr
yellow ."    Code: " white here . ." opcodes used" cr
yellow ."    Dict: " white dict-end last - de-sz / . ." words defined" cr

1 load
