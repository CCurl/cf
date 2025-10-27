(( Core ))

65536 cell * memory + (vha) ! (ha) @ (la) @
: here  (ha)  @ ;
: vhere (vha) @ ;
: last  (la)  @ ;
: ->code ( n--a ) cell * memory + ;
: , ( n-- ) here ->code ! 1 (ha) +! ;
: allot ( n-- ) (vha) +! ;
: const ( n-- ) addword (lit) , , (exit) , ;
: var ( n-- )  vhere const allot ;

const -la-
const -ha-
vhere const -vha-

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

( ***************************************************** )
( STATES/MODES )
: define  1 ; inline
: compile 2 ; inline
: interp  3 ; inline
: comment 4 ; inline
( ***************************************************** )

: a+   a@+ drop  ; inline
: a-   a@- drop  ; inline
: c@a  a@  c@    ; inline
: c@a+ a@+ c@    ; inline
: c@a- a@- c@    ; inline
: c!a  a@  c!    ; inline
: c!a+ a@+ c!    ; inline
: c!a- a@- c!    ; inline
: adrop  a> drop ; inline
: b+   b@+ drop  ; inline
: c!b+ b@+ c!    ; inline
: bdrop  b> drop ; inline
: t+   t@+ drop  ; inline
: t-   t@- drop  ; inline
: c@t  t@  c@    ; inline
: c@t+ t@+ c@    ; inline
: c@t- t@- c@    ; inline
: c!t  t@  c!    ; inline
: c!t+ t@+ c!    ; inline
: c!t- t@- c!    ; inline
: t@+c t@ dup cell + t! ;
: tdrop  t> drop ; inline
: atdrop adrop tdrop ;

: comp? state @ compile = ;
( quote subroutine )
: t4  vhere dup >t  >in @ 1+ >a
   begin
      c@a '"' = if
         0 c!t+  a> 1+ >in !
         comp? if0 tdrop exit then
         t> (vha) ! (lit) , , exit
      then c@a+ c!t+
   again ;

: z"  t4 ; immediate
: ." t4 comp? if (ztype) , exit then ztype ; immediate

( Files )
: fopen-r z" rb"  fopen ;
: fopen-w z" wb"  fopen ;
: ->file ( fh-- ) (output-fp) ! ;
: ->stdout 0 ->file ;

: source-loc memory 100000 + ;
: rb ( reboot )
   -vha- (vha) !  -la- (la) !  -ha- (ha) !
   z" boot.fth" fopen-r -if dup then if >a
      source-loc >b
      50000 for 0 c!b+ next bdrop
      source-loc 50000 a@ fread drop a> fclose
      source-loc >in !
   then ;

: bl 32 ; inline
: tab 9 emit ; inline
: cr 13 emit 10 emit ; inline
: spaces for bl emit next ; inline
: negate com 1+ ; inline
: abs dup 0 < if negate then ;
: ?dup -if dup then ;

: #neg 0 >a dup 0 < if negate a+ then ;
: <#   ( n--n' ) #neg last 32 - >t 0 t@ c! ;
: hold ( c--n )  t- c!t ;
: #n   ( n-- )   '0' + dup '9' > if 7 + then hold ;
: #.   ( -- )    '.' hold ;
: #    ( n--n' ) base @ /mod swap #n ;
: #s   ( n-- )   begin # -while drop ;
: #>   ( --a )   a> if '-' hold then t> ;

: (.)  ( n-- ) <# #s #> ztype ;
: .  ( n-- ) (.) : space bl emit ;
: .nw >r <# r> ?dup if 1- for # next then #s #> ztype ;
: .nwb base @ >b base ! .nw b> base ! ;
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
: 2dup over over ; inline
: 2drop drop drop ; inline
: mod /mod drop ; inline
: ? @ . ;
: nip  swap drop ; inline
: tuck swap over ; inline
: min ( a b--c ) 2dup > if swap then drop ;
: max ( a b--c ) 2dup < if swap then drop ;
: vc, vhere c! 1 allot ;
: v, vhere ! cell allot ;

: 0sp 0 (dsp) ! ;
: unloop (lsp) @ 3 - 0 max (lsp) ! ;
: depth (dsp) @ 1- ;
: lpar '(' emit ; inline
: rpar ')' emit ; inline
: .comma ',' emit ; inline
: .s lpar space depth ?dup if
      for i 1+ cells dstk + @ . next
   then rpar ;

: dict-end memory mem-sz + 1- 7 com and ;
: de>xt    @ ; inline
: de>flags cell + c@ ; inline
: de>len   cell + 1+ c@ ; inline
: de>name  cell + 2+ ; inline
: .word de>name ztype ;
: .de-word .word t@+ 10 > if 0 t! cr exit then tab ;

: words last >a 1 >t 0 >b begin
    a@ de>len  7 > if t+ then
    a@ de>len 12 > if t+ then
    a@ .de-word a@ de-sz + a! b+
    a@ dict-end < while
    lpar b> . ." words)" adrop ;
: words-n last >t for i 8 mod if0 cr then t@ .word tab t@ de-sz + t! next tdrop ;

: fill ( addr num ch-- ) >t >r >a  r> for t@ c!a+ next atdrop ;

: s-end  ( str--end )     dup s-len + ; inline
: s-cat  ( dst src--dst ) over s-end swap s-cpy drop ;
: s-catc ( dst ch--dst )  over s-end tuck c! 0 swap 1+ c! ;
: s-catn ( dst num--dst ) <# #s #> over s-end swap s-cpy drop ;
: s-scat ( src dst--dst ) swap s-cat ;
: pad ( --a ) vhere 100 + ;

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

( Blocks )
: rows 25 ; inline
: cols 80 ; inline
: blk-max 499 ; inline
rows cols * const blk-sz
blk-sz blk-max * var blks
cell   var t0
: blk@ ( --n ) t0 @ ;
: blk! ( n-- ) t0 ! ;
: blk-data ( --a ) blk@ blk-sz * blks + ;
: blk-clr ( -- ) blk-data blk-sz 0 fill ;
16 var t1
: blk-fn ( --a ) t1 z" block-" s-cpy blk@ <# # # #s #> s-cat z" .fth" s-cat ;
: blk-rd ( -- ) blk-clr  blk-fn fopen-r ?dup
   if >a blk-data blk-sz a@ fread drop a> fclose then ;
: blk-wr ( -- ) blk-fn fopen-w ?dup
   if >a blk-data blk-sz a@ fwrite drop a> fclose then ;

(( Keys ))
#256  #59 + const key-f1
#256  #60 + const key-f2
#256  #61 + const key-f3
#256  #62 + const key-f4
#256  #71 + const key-home   (( VT: 27 91 72 ))
#256  #72 + const key-up     (( VT: 27 91 65 ))
#256  #73 + const key-pgup   (( VT: 27 91 53 126 ))
#256  #75 + const key-left   (( VT: 27 91 68 ))
#256  #77 + const key-right  (( VT: 27 91 67 ))
#256  #79 + const key-end    (( VT: 27 91 70 ))
#256  #80 + const key-down   (( VT: 27 91 66 ))
#256  #81 + const key-pgdn   (( VT: 27 91 54 126 ))
#256  #82 + const key-ins    (( VT: 27 91 50 126 ))
#256  #83 + const key-del    (( VT: 27 91 51 126 ))
#256 #119 + const key-chome  (( VT: 27 91 ?? ??? ))
#256 #117 + const key-cend   (( VT: 27 91 ?? ??? ))
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
: vt-key ( --k )  key 91 = if vk1 exit then 27 ;
: vkey ( --k ) key dup if0 drop #256 key + exit then ( Windows FK )
    dup 224 = if drop #256 key + exit then ( Windows )
    dup  27 = if drop vt-key exit then ; ( VT )

( Accept )
: printable? ( c--f ) dup 31 > swap 127 < and ;
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
vhere const ed-colors
219   vc, (( 0: default - purple ))
c-red vc, (( 1: define  - red ))
 76   vc, (( 2: compile - green ))
226   vc, (( 3: interp  - yellow ))
255   vc, (( 4: comment - white ))

: ed-color@ ( n-- ) ed-colors + c@ ;
: ed-color! ( fg n-- ) ed-colors + c! ;

blk-sz var ed-blk
ed-blk blk-sz + 1- const last-ch
cell var (r)  : row! (r) ! ;       : row@ (r) @ ;
cell var (c)  : col! (c) ! ;       : col@ (c) @ ;
1 var t1      : mode! t1 c! ;  : mode@ t1 c@ ;
1 var t1      : show? t1 c@ ;
1 var (dirty) : dirty? (dirty) c@ ;
: shown 0 t1 c! ;
: show! 1 t1 c! ;
: dirty! 1 (dirty) c! show! ;
: clean 0 (dirty) c! ;
: norm-pos ( pos--new ) ed-blk max last-ch min ;
: cr->pos ( col row--pos ) cols * + ed-blk + ;
: rc->pos ( --pos ) col@ row@ cr->pos ;
: pos->rc ( pos-- ) norm-pos ed-blk - cols /mod row! col! ;
: mv ( r c-- )  (c) +! (r) +!  rc->pos  pos->rc ;
: row-last ( r--a ) cols 1- swap cr->pos ;
: ed-ch! ( c-- ) rc->pos c! dirty! ;
: ed-ch@ ( --c ) rc->pos c@ ;
: ed-clr ( -- ) ed-blk blk-sz 0 fill ;
: t4 ( -- ) cols for i col! c@a+ >b
   b@ 10 = b@ 0= or if bdrop unloop exit then
   b> rc->pos c! next ;
: blk->ed ( -- ) ed-clr blk-data >a rows for i row! t4 next adrop ;
: ed-load ( -- ) blk-rd blk->ed clean show! ed-blk pos->rc ;
: t4 ( -- ) rows for 0 i cr->pos ztype 10 emit next ;
: ed-save blk-fn fopen-w ?dup if >a a@ ->file t4 ->stdout a> fclose then ;
: ->norm  0 mode! ;    : norm?  mode@  0 = ;
: ->repl  1 mode! ;    : repl?  mode@  1 = ;
: ->ins   2 mode! ;    : ins?   mode@  2 = ;
: quit!  99 mode! ;    : quit?  mode@ 99 = ;  
: ed-emit ( ch-- )
   dup 31 > if emit exit then ( regular char )
   dup  5 < over 0 > and if dup ed-color@ fg then ( change color )
   drop 32 emit ;
: .scr 1 dup ->rc white ed-blk >a rows for
      cols for c@a+ ed-emit next cr
   next adrop ;
: ->cur  col@ 1+ row@ 1+ ->cr ;
: ->foot 1 rows 1 + ->cr ;
: ->cmd ->foot cr ;
: ./ '/' emit ;
: .foot ->foot cyan ." Block #" blk@ .
   bl dirty? if drop '*' then emit space
   lpar row@ 1+ (.) .comma col@ 1+ (.) rpar
   norm? if green  ."  -norm-"    then
   repl? if yellow ."  -replace-" then
   ins?  if purple ."  -insert-"  then white 
   rc->pos c@ dup space lpar .#dec ./ .$hex rpar clr-eol ;
: show show? if cur-off .scr cur-on shown then .foot ->cur  ;
: mv-left 0 dup 1-      mv ;   : mv-right 0 1 mv ;
: mv-up   0 dup 1- swap mv ;   : mv-down  1 0 mv ;
: ins-bl  rc->pos dup 1+ cols col@ - 1- cmove 32 ed-ch! ;
: ins-bl2 rc->pos dup 1+ dup last-ch swap - 1+ cmove 32 ed-ch! ;
: replace-char! ( ch-- ) ed-ch! mv-right ;
: replace-char  a@ printable? if a@ replace-char! then ;
: insert-char   a@ printable? if ins-bl a@ replace-char! then ;
: del-ch rc->pos dup 1+ swap cols col@ - cmove 0 row@ row-last c! dirty! ;
: clr-line rc->pos col@ - cols 0 fill dirty! ;
: ed-goto ( blk-- ) blk! ed-load show! ;
: ins-line  row@ rows < if 
      last-ch >a  a@ cols - >t  0 row@ cr->pos >r
      begin c@t- c!a- t@ r@ < until atdrop rdrop
   then clr-line ;
: del-line  row@ rows < if
      0 row@ cr->pos >t  t@ cols + >a  last-ch >r
      begin c@a+ c!t+  a@ r@ > until atdrop rdrop
   then row@  rows 1- row!  clr-line  row! ;
: ed-prev-word rc->pos 1- >t begin
      t@ ed-blk < if t> pos->rc exit then
      c@t- 33 < if t> 1+ pos->rc exit then
 again ;
: ed-next-word rc->pos 1+ >t begin
      t@ last-ch > if t> pos->rc exit then
      c@t+ 33 < if t> 1- pos->rc exit then
 again ;
: next-pg ed-save  blk@ 1+ ed-goto ;
: prev-pg ed-save  blk@ 1- 0 max ed-goto ;
: rl blk@ ed-goto ;
: q quit! ;
: wq ed-save quit! ;
: do-cmd ->cmd ':' emit clr-eol pad accept
   pad outer show!
   ->cmd clr-eol 0 pad ! ;

( switch: case-table process )
: case   ( ch-- )  v, find drop v, ;   ( case-table entry - single word )
: case!  ( ch-- )  v, here v, compile state ! ;   ( case-table entry - code )
: switch ( tbl-- ) >t begin
      t@ @ if0 tdrop exit then
      t@+c @ a@ = if t> @ >r exit then
      t@ cell+ t! again ;

(( VI-like commands ))
vhere const ed-ctrl-cases
key-left    case   mv-left
key-right   case   mv-right
key-up      case   mv-up
key-down    case   mv-down
key-home    case!  0 col! ;
key-ins     case!  ins? if ->norm exit then ->ins ;
key-del     case   del-ch
key-pgup    case   prev-pg
key-pgdn    case   next-pg
key-chome   case!  0 dup row! col! ;
key-cend    case!  rows 1- row! 0 col! ;
key-f1      case!  define  replace-char! ;
key-f2      case!  compile replace-char! ;
key-f3      case!  interp  replace-char! ;
key-f4      case!  comment replace-char! ;
 3          case   ->norm
 9          case!  0 8 mv ;
13          case!  mv-down 0 col! ;
27          case   ->norm
0 v, 0 v, (( end ))

vhere const ed-cases
'j'  case   mv-down
'k'  case   mv-up
'h'  case   mv-left
'l'  case   mv-right
32   case   mv-right
'q'  case!  0 8 mv ;
'Q'  case!  0 8 negate mv ;
'1'  case!  define  replace-char! ;
'2'  case!  compile replace-char! ;
'3'  case!  interp  replace-char! ;
'4'  case!  comment replace-char! ;
'_'  case!  0 col! ;
':'  case!  do-cmd ;
'b'  case   ins-bl
'B'  case   ins-bl2
'x'  case   del-ch
'X'  case!  mv-left del-ch ;
'C'  case   clr-line
'r'  case!  red '?' emit key a! replace-char ;
'R'  case   ->repl
'i'  case   ->ins
'O'  case   ins-line 
'o'  case!  mv-down ins-line ;
'D'  case   del-line
'w'  case   ed-next-word
'W'  case   ed-prev-word
'g'  case!  rows 0  row! 0 col! ;
'G'  case!  rows 1- row! 0 col! ;
'+'  case   next-pg
'-'  case   prev-pg
0 v, 0 v, (( end ))

: process-key ( --, key is in a )
   a@ 32 < a@ 127 > or if ed-ctrl-cases switch exit then
   ins?  if insert-char exit then
   repl? if replace-char exit then
   ed-cases switch ;
: ed-loop begin show vkey >a process-key adrop quit? until ;
: ed-init cls 0 mode! 0 dup row! col! blk@ ed-goto ;
: ed  ( -- ) ed-init ed-loop ->cmd interp state ! ;
: edit ( n-- )  blk! ed ;

( This dump is from Peter Jakacki )
: a-emit ( b-- ) dup $20 < over $7e > or if drop '.' then emit ;
: .ascii ( -- ) a@ $10 - $10 for dup c@ a-emit 1+ next drop ;
: dump ( f n-- ) swap >a 0 >t for
      t@ if0 cr a@ .hex ':' emit space then
      c@a+ .hex space
      t@+ $0f = if 3 spaces .ascii 0 t! then
   next atdrop ;
   
( fgl: forget the last word )
: fgl last dup de-sz + (la) ! de>xt (ha) ! ;  

cell var t0
cell var t1
cell var t2
: marker here t0 ! last t1 ! vhere t2 ! ;
: forget t0 @ (ha) ! t1 @ (la) ! t2 @ (vha) ! ;

: .version ." cf v" version <# # # #. # # #. #s #> ztype ;

(( fixed point ))
cell var t0
: fbase! t0 ! ;
: fbase t0 @ ;
cell var t0
: fprec t0 @ ;
: fprec! t0 ! 1 fprec for 10 * next fbase! ;
2 fprec!
: f. fbase /mod (.) '.' emit abs fprec .nw ;
: f* * fbase / ;
: f/ >a fbase * a> / ;
: f+ + ;
: f- - ;

( some benchmarks )
: mil 1000 dup * * ;
: elapsed timer swap - . ." usec" ;
: bm timer swap for next elapsed ;
: fib 1- dup 2 < if drop 1 exit then dup fib swap 1- fib + ;
: fib-bm timer swap fib . elapsed ;
: bb 1000 mil bm ;

(( compile then execute ))
cell var there
: [[ here there ! compile state ! ;
: ]] (exit) , there @ (ha) ! interp state ! here execute ;
immediate

( Move the source to the disk area )
: vi z" vi boot.fth" system ;
: lg z" lazygit" system ;
: ll z" ls -l" system ;

marker
green .version white ."  - Chris Curl " cr
yellow ."  Memory: " white mem-sz . ." bytes" cr
yellow ."    Code: " white here . ." opcodes used" cr
yellow ."    Dict: " white dict-end last - de-sz / . ." words defined" cr
