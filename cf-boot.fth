( these are created later as -last- and -here- )
( they are used later for rebooting )
(h) @   (l) @ 

: last (l) @ ;
: here (h) @ ;
: inline    ( -- ) $40 last cell 1+ + c! ;
: immediate ( -- ) $80 last cell 1+ + c! ;
: cells  ( n--n' ) cell * ; inline
: ->code ( off--addr ) cells mem + ;
: code@  ( off--dw )  ->code @ ;
: code!  ( dw off-- ) ->code ! ;
: , ( dw-- ) here dup 1 + (h) ! code! ;

: bye      ( -- ) 999 state ! ;
: (exit)   ( --n )  0 ; inline
: (lit)    ( --n )  1 ; inline
: (jmp)    ( --n )  2 ; inline
: (jmpz)   ( --n )  3 ; inline
: (jmpnz)  ( --n )  4 ; inline
: (njmpz)  ( --n )  5 ; inline
: (njmpnz) ( --n )  6 ; inline
: (ztype)  ( --n ) 43 ; inline

: if   (jmpz)   , here 0 , ; immediate
: -if  (njmpz)  , here 0 , ; immediate
: if0  (jmpnz)  , here 0 , ; immediate
: -if0 (njmpnz) , here 0 , ; immediate
: then here swap code!     ; immediate

: begin here ; immediate
: again (jmp)     , , ; immediate
: while (jmpnz)   , , ; immediate
: -while (njmpnz) , , ; immediate
: until (jmpz)    , , ; immediate

( val and (val) define a very efficient variable mechanism )
( Usage:  val a@   (val) (a)   : a! (xx) ! ; )
: const ( n-- ) add-word (lit) , , (exit) , ;
:  val  ( -- ) 0 const ;
: (val) ( -- ) here 2 - ->code const ;

( the original here and last - used by 'rb' )
const -last-   const -here-

mem mem-sz + const dict-end
32 ->code const (vh)
64 1024 * ->code const vars
vars (vh) !
: vhere ( --a ) (vh) @ ;
: allot ( n-- ) (vh) +! ;
: var   ( n-- ) vhere const allot ;

( TSTK is a stack for 3 locals - x,y,z )
: +L1 ( x -- )    +L x! ;
: +L2 ( x y-- )   +L y! x! ;
: +L3 ( x y z-- ) +L z! y! x! ;

: x++ ( -- )  x@ 1+ x! ;   : x@+  ( --n ) x@ x++ ;
: x-- ( -- )  x@ 1- x! ;   : x@-  ( --n ) x@ x-- ;
: c@x ( --b ) x@ c@ ;      : c@x+ ( --b ) x@+ c@ ;  : c@x- ( --b ) x@- c@ ;
: c!x ( b-- ) x@ c! ;      : c!x+ ( b-- ) x@+ c! ;  : c!x- ( b-- ) x@- c! ;

: y++ ( -- )  y@ 1+ y! ;   : y@+  ( --n ) y@ y++ ;
: y-- ( -- )  y@ 1- y! ;   : y@-  ( --n ) y@ y-- ;
: c@y ( --b ) y@ c@ ;      : c@y+ ( --b ) y@+ c@ ;  : c@y- ( --b ) y@- c@ ;
: c!y ( b-- ) y@ c! ;      : c!y+ ( b-- ) y@+ c! ;  : c!y- ( b-- ) y@- c! ;

: z++ ( -- )  z@ 1+ z! ;   : z@+  ( --n ) z@ z++ ;

( Strings )
: compiling? ( --n ) state @ 1 = ;
: t3 ( --a ) +L vhere dup z! x! 1 >in +!
    begin
        >in @ c@ y! 1 >in +!
        y@ 0 = y@ '"' = or
        if  0 c!x+  z@
            compiling? if (lit) , , x@ (vh) ! then
            -L exit
        then
        y@ c!x+
    again ;

: z" ( "string"--addr ) t3 ; immediate
: ." ( "string"-- ) t3 compiling? if (ztype) , exit then ztype ; immediate

( Files )
: fopen-r   ( nm--fh ) z" rb" fopen ;
: fopen-w   ( nm--fh ) z" wb" fopen ;
: ->file    ( fh-- )   output-fp ! ;
: ->stdout  ( -- )     0 ->file ;
: ->stdout! ( -- )     output-fp @ fclose ->stdout ;

( reboot )
: t4 50000 ;
: t5 vars t4 + ;
: rb ( -- )
    z" boot.fth" fopen-r -if0 drop ." -nf-" exit then
    z! t5 x! t4 for 0 c!x+ next
    t5 t4 z@ fread drop z@ fclose
    -here- (h) !  -last- (l) ! 
    t5 >in ! ;
: vi z" vi boot.fth" system ;

( More core words )
: [ ( -- ) 0 state ! ; immediate  ( 0 = INTERPRET )
: ] ( -- ) 1 state ! ;            ( 1 = COMPILE )
: rdrop ( -- ) r> drop ; inline
: tuck  ( a b--b a b )   swap over ; inline
: nip   ( a b--b )       swap drop ; inline
: ?dup ( n--n n|0 )  -if dup then ;
: 2dup  ( a b--a b a b ) over over ; inline
: 2drop ( a b-- )        drop drop ; inline
: -rot ( a b c--c a b )  swap >r swap r> ;
: 0< ( n--f ) 0 <    ; inline
: <= ( a b--f ) > 0= ;
: >= ( a b--f ) < 0= ;
: type ( a n-- ) for dup c@ emit 1+ next drop ;
: btwi ( n l h--f ) >r over <= swap r> <= and ;
: negate ( n--n' ) 0 swap - ;
: abs ( n--n' ) dup 0< if negate then ;
: cr  ( -- )     13 emit 10 emit ;
: tab ( -- )      9 emit ;
: space  ( -- )  32 emit ;
: spaces ( n-- ) for space next ;
: /   ( a b--q ) /mod nip  ;
: mod ( a b--r ) /mod drop ;
: */  ( n m q--n' ) >r * r> / ;
: min ( a b-a|b ) over over > if swap then drop ;
: max ( a b-a|b ) over over < if swap then drop ;
: unloop  ( -- ) (lsp) @ 3 - 0 max (lsp) ! ;
: execute ( xt-- ) ?dup if >r then ;
: decimal  ( -- )  #10 base ! ;
: hex      ( -- )  $10 base ! ;
: binary   ( -- )  %10 base ! ;

   1 var (neg)
  65 var buf
cell var (buf)
: ?neg ( n--n' ) dup 0< dup (neg) c! if negate then ;
: hold ( c-- )   -1 (buf) +! (buf) @ c! ;
: #.   ( -- )    '.' hold ;
: #n   ( n-- )   '0' + dup '9' > if 7 + then hold ;
: #    ( n--m )  base @ /mod swap #n ;
: #s   ( n--0 )  # -if #s exit then ;
: <#   ( n--n' ) ?neg buf 65 + (buf) ! 0 hold ;
: #>   ( n--a )  drop (neg) @ if '-' hold then (buf) @ ;
: (.)  ( n-- )   <# #s #> ztype ;
: .    ( n-- )   (.) space ;

: 0sp 0 (sp) ! ;
: depth ( --n ) (sp) @ 1- ;
: .s '(' emit space depth ?dup if
        stk swap for cell + dup @ . next drop
    then ')' emit ;

: .word ( de-- ) cell + 3 + ztype ;
: words ( -- ) +L last x! 0 y! 1 z! begin
        x@ dict-end < if0 '(' emit z@ . ." words)" -L exit then
        x@ .word tab z++
        x@ cell + 2 + c@ 7 > if y++ then 
        y@+ 12 > if cr 0 y! then
        x@ dup cell + c@ + x!
    again ;

: words-n ( n-- ) +L last x! 0 y! for
        x@ .word tab
        y@+ 12 > if cr 0 y! then
		x@ dup cell + c@ + x!
    next -L ;

cell var t4   cell var t5
: [[ here t4 !  vhere t5 !  1 state ! ;
: ]] (exit) , 0 state ! t4 @ dup >r (h) ! t5 @ (vh) ! ; immediate

( Strings / Memory )
: pad    ( --a ) vhere $100 + ;
: fill   ( a num ch-- ) -rot for 2dup c! 1+ next 2drop ;
: cmove  ( f t n-- )  +L3  z@ if  z@ for c@x+ c!y+ next then -L ;
: cmove> ( f t n-- )  +L3  y@ z@ + 1- y!  x@ z@ + 1- x!  z@ for c@x- c!y- next -L ;
: s-len  ( str--len ) +L1 0 begin c@x+ if0 -L exit then 1+ again ;
: s-end  ( str--end ) dup s-len + ;   \ end: address of the null
: s-cpy  ( dst src--dst ) 2dup s-len 1+ cmove ;
: s-cat  ( dst src--dst ) over s-end  over s-len 1+  cmove ;
: s-catc ( dst ch--dst )  over s-end  +L1  c!x+  0 c!x+  -L ;
: s-catn ( dst num--dst ) <# #s #> s-cat ;
: s-eqn  ( s1 s2 n--f ) +L3 z@ for
	   c@x+ c@y+ = if0 -L 0 unloop exit then
	next -L 1 ;
: s-eq   ( s1 s2--f ) dup s-len 1+ s-eqn ;
  
( Disk: 32 blocks, 32K bytes each )
mem 14 1024 1024 * * + const disk
32 var fn
val blk@   (val) t0
: #blks     ( --n )   32 ;
: blk-sz    ( --n )   32768 ;
: blk!      ( n-- )   0 max #blks 1- min t0 ! ;
: blk-fn    ( --a )   fn z" block-" s-cpy blk@ <# # #s #> s-cat z" .fth" s-cat ;
: blk-addr  ( --a )   blk@ blk-sz * disk + ;
: blk-clr   ( -- )    blk-addr blk-sz 0 fill ;
: t2        ( fh-- )  >r  blk-clr  blk-addr blk-sz r@ fread drop  r> fclose ;
: blk-read  ( -- )    blk-fn fopen-r ?dup if0 ." -nf-" drop exit then t2 ;
: t1        ( fh-- )  >r  blk-addr blk-sz r@ fwrite drop  r> fclose ;
: blk-write ( -- )    blk-fn fopen-w ?dup if0 ." -err-" drop exit then t1 ;
: blk-nullt ( -- )    0 blk-addr blk-sz + 1- c! ;
: load      ( n-- )   blk! blk-read blk-nullt blk-addr outer ;
: load-next ( n-- )   blk! blk-read blk-nullt blk-addr >in ! ;

( *** App code - starts in block-001 *** )
1 load
