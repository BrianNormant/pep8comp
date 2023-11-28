_start:br start ; Comment
.ASCII "string with ;, may this not fuck";
.ASCII "string with \""; comment
.ASCII "Whow, case sEnSiTiVe with \\;"

ad:    .ADDRSS _start
.ASCII "Hello UwU" ; comment
; comment
start: LDA 0,i ; comement
       STA var,d ; comment
       DECO var,d
       STOP ; yet another comment

var:   .BLOCK 45 ; suprising! a comment

; Finaly a comment
