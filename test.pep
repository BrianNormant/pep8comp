BR start ; Comment
.ASCII "string with ;, may this not fuck";
.ASCII "string with \""; comment
.ASCII "string with \\;"
.ASCII "string with \\"
.ASCII "string with \\\\\\"
.ASCII "string with \a";

str:   .ASCII "Hello UwU" ; comment
; comment
start: LDA 0,i ; comement
       STA var,d ; comment
       DECO var,d
       STOP ; yet another comment

var:   .BLOCK 45 ; suprising! a comment

; Finaly a comment
