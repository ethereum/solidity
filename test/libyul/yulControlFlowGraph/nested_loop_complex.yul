{
    for { let x := 0 } lt(x, 0x0101) {
        sstore(x, 0x0202)
        for { let y := 0 } lt(y, 0x0303) { y := add(y, 0x0404) } {
            sstore(y, 0x0505)
        }
        x := add(x, 0x0202)
    }
    {
        sstore(0x0606, 0x0606)
        if sload(0x0707) { continue }
        sstore(0x0808, 0x0808)
        if sload(0x0909) { break }
        sstore(0x0A0A, 0x0B0B)
        for { let z := 0 } lt(z, 0x0C0C) { z := add(z, 1) } {
            sstore(0x0D0D, 0x0D0D)
            if sload(0x0E0E) {
                continue
            }
            sstore(0x0F0F, 0x0F0F)
            if sload(0x1010) {
                break
            }
            sstore(0x1111, 0x1111)
        }
        sstore(0x1212, 0x1212)
    }
}
// ----
// digraph CFG {
// nodesep=0.7;
// node[shape=box];
//
// Entry [label="Entry"];
// Entry -> Block0;
// Block0 [label="\
// Assignment(x): [ 0x00 ] => [ x ]\l\
// "];
// Block0 -> Block0Exit [arrowhead=none];
// Block0Exit [label="Jump" shape=oval];
// Block0Exit -> Block1;
//
// Block1 [label="\
// lt: [ 0x0101 x ] => [ TMP[lt, 0] ]\l\
// "];
// Block1 -> Block1Exit;
// Block1Exit [label="{ TMP[lt, 0]| { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block1Exit:0 -> Block2;
// Block1Exit:1 -> Block3;
//
// Block2 [label="\
// "];
// Block2Exit [label="MainExit"];
// Block2 -> Block2Exit;
//
// Block3 [label="\
// sstore: [ 0x0606 0x0606 ] => [ ]\l\
// sload: [ 0x0707 ] => [ TMP[sload, 0] ]\l\
// "];
// Block3 -> Block3Exit;
// Block3Exit [label="{ TMP[sload, 0]| { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block3Exit:0 -> Block4;
// Block3Exit:1 -> Block5;
//
// Block4 [label="\
// sstore: [ 0x0808 0x0808 ] => [ ]\l\
// sload: [ 0x0909 ] => [ TMP[sload, 0] ]\l\
// "];
// Block4 -> Block4Exit;
// Block4Exit [label="{ TMP[sload, 0]| { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block4Exit:0 -> Block6;
// Block4Exit:1 -> Block7;
//
// Block5 [label="\
// "];
// Block5 -> Block5Exit [arrowhead=none];
// Block5Exit [label="Jump" shape=oval];
// Block5Exit -> Block8;
//
// Block6 [label="\
// sstore: [ 0x0b0b 0x0a0a ] => [ ]\l\
// Assignment(z): [ 0x00 ] => [ z ]\l\
// "];
// Block6 -> Block6Exit [arrowhead=none];
// Block6Exit [label="Jump" shape=oval];
// Block6Exit -> Block9;
//
// Block7 [label="\
// "];
// Block7 -> Block7Exit [arrowhead=none];
// Block7Exit [label="Jump" shape=oval];
// Block7Exit -> Block2;
//
// Block8 [label="\
// sstore: [ 0x0202 x ] => [ ]\l\
// Assignment(y): [ 0x00 ] => [ y ]\l\
// "];
// Block8 -> Block8Exit [arrowhead=none];
// Block8Exit [label="Jump" shape=oval];
// Block8Exit -> Block10;
//
// Block9 [label="\
// lt: [ 0x0c0c z ] => [ TMP[lt, 0] ]\l\
// "];
// Block9 -> Block9Exit;
// Block9Exit [label="{ TMP[lt, 0]| { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block9Exit:0 -> Block11;
// Block9Exit:1 -> Block12;
//
// Block10 [label="\
// lt: [ 0x0303 y ] => [ TMP[lt, 0] ]\l\
// "];
// Block10 -> Block10Exit;
// Block10Exit [label="{ TMP[lt, 0]| { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block10Exit:0 -> Block13;
// Block10Exit:1 -> Block14;
//
// Block11 [label="\
// sstore: [ 0x1212 0x1212 ] => [ ]\l\
// "];
// Block11 -> Block11Exit [arrowhead=none];
// Block11Exit [label="Jump" shape=oval];
// Block11Exit -> Block8;
//
// Block12 [label="\
// sstore: [ 0x0d0d 0x0d0d ] => [ ]\l\
// sload: [ 0x0e0e ] => [ TMP[sload, 0] ]\l\
// "];
// Block12 -> Block12Exit;
// Block12Exit [label="{ TMP[sload, 0]| { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block12Exit:0 -> Block15;
// Block12Exit:1 -> Block16;
//
// Block13 [label="\
// add: [ 0x0202 x ] => [ TMP[add, 0] ]\l\
// Assignment(x): [ TMP[add, 0] ] => [ x ]\l\
// "];
// Block13 -> Block13Exit [arrowhead=none];
// Block13Exit [label="BackwardsJump" shape=oval];
// Block13Exit -> Block1;
//
// Block14 [label="\
// sstore: [ 0x0505 y ] => [ ]\l\
// "];
// Block14 -> Block14Exit [arrowhead=none];
// Block14Exit [label="Jump" shape=oval];
// Block14Exit -> Block17;
//
// Block15 [label="\
// sstore: [ 0x0f0f 0x0f0f ] => [ ]\l\
// sload: [ 0x1010 ] => [ TMP[sload, 0] ]\l\
// "];
// Block15 -> Block15Exit;
// Block15Exit [label="{ TMP[sload, 0]| { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block15Exit:0 -> Block18;
// Block15Exit:1 -> Block19;
//
// Block16 [label="\
// "];
// Block16 -> Block16Exit [arrowhead=none];
// Block16Exit [label="Jump" shape=oval];
// Block16Exit -> Block20;
//
// Block17 [label="\
// add: [ 0x0404 y ] => [ TMP[add, 0] ]\l\
// Assignment(y): [ TMP[add, 0] ] => [ y ]\l\
// "];
// Block17 -> Block17Exit [arrowhead=none];
// Block17Exit [label="BackwardsJump" shape=oval];
// Block17Exit -> Block10;
//
// Block18 [label="\
// sstore: [ 0x1111 0x1111 ] => [ ]\l\
// "];
// Block18 -> Block18Exit [arrowhead=none];
// Block18Exit [label="Jump" shape=oval];
// Block18Exit -> Block20;
//
// Block19 [label="\
// "];
// Block19 -> Block19Exit [arrowhead=none];
// Block19Exit [label="Jump" shape=oval];
// Block19Exit -> Block11;
//
// Block20 [label="\
// add: [ 0x01 z ] => [ TMP[add, 0] ]\l\
// Assignment(z): [ TMP[add, 0] ] => [ z ]\l\
// "];
// Block20 -> Block20Exit [arrowhead=none];
// Block20Exit [label="BackwardsJump" shape=oval];
// Block20Exit -> Block9;
//
// }
