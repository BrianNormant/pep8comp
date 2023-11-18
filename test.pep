BR start

str:   .ASCII "Hello UwU"

start: LDA 0,i
       STA var,d
       DECO var,d
       STOP

var:   .BLOCK 45
