{
    function f(a, b) -> c {
        for { let x := 42 } lt(x, a) {
            x := add(x, 1)
            if calldataload(x)
            {
                sstore(0, x)
                leave
                sstore(0x01, 0x0101)
            }
            sstore(0xFF, 0xFFFF)
        }
        {
            switch mload(x)
            case 0 {
                sstore(0x02, 0x0202)
                break
                sstore(0x03, 0x0303)
            }
            case 1 {
                sstore(0x04, 0x0404)
                leave
                sstore(0x05, 0x0505)
            }
            case 2 {
                sstore(0x06, 0x0606)
                revert(0, 0)
                sstore(0x07, 0x0707)
            }
            case 3 {
                sstore(0x08, 0x0808)
            }
            default {
                if mload(b) {
                    return(0, 0)
                    sstore(0x09, 0x0909)
                }
                    sstore(0x0A, 0x0A0A)
            }
            sstore(0x0B, 0x0B0B)
        }
        sstore(0x0C, 0x0C0C)
    }
    pop(f(1,2))
}
// ----
// digraph CFG {
// nodesep=0.7;
// node[shape=box];
//
// Entry [label="Entry"];
// Entry -> Block0;
// Block0 [label="\
// f: [ RET[f] 0x02 0x01 ] => [ TMP[f, 0] ]\l\
// pop: [ TMP[f, 0] ] => [ ]\l\
// "];
// Block0Exit [label="MainExit"];
// Block0 -> Block0Exit;
//
// FunctionEntry_f_1 [label="function f(a, b) -> c"];
// FunctionEntry_f_1 -> Block1;
// Block1 [label="\
// Assignment(x): [ 0x2a ] => [ x ]\l\
// "];
// Block1 -> Block1Exit [arrowhead=none];
// Block1Exit [label="Jump" shape=oval];
// Block1Exit -> Block2;
//
// Block2 [label="\
// lt: [ a x ] => [ TMP[lt, 0] ]\l\
// "];
// Block2 -> Block2Exit;
// Block2Exit [label="{ TMP[lt, 0]| { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block2Exit:0 -> Block3;
// Block2Exit:1 -> Block4;
//
// Block3 [label="\
// sstore: [ 0x0c0c 0x0c ] => [ ]\l\
// "];
// Block3Exit [label="FunctionReturn[f]"];
// Block3 -> Block3Exit;
//
// Block4 [label="\
// mload: [ x ] => [ TMP[mload, 0] ]\l\
// Assignment(GHOST[0]): [ TMP[mload, 0] ] => [ GHOST[0] ]\l\
// eq: [ GHOST[0] 0x00 ] => [ TMP[eq, 0] ]\l\
// "];
// Block4 -> Block4Exit;
// Block4Exit [label="{ TMP[eq, 0]| { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block4Exit:0 -> Block5;
// Block4Exit:1 -> Block6;
//
// Block5 [label="\
// eq: [ GHOST[0] 0x01 ] => [ TMP[eq, 0] ]\l\
// "];
// Block5 -> Block5Exit;
// Block5Exit [label="{ TMP[eq, 0]| { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block5Exit:0 -> Block7;
// Block5Exit:1 -> Block8;
//
// Block6 [label="\
// sstore: [ 0x0202 0x02 ] => [ ]\l\
// "];
// Block6 -> Block6Exit [arrowhead=none];
// Block6Exit [label="Jump" shape=oval];
// Block6Exit -> Block3;
//
// Block7 [label="\
// eq: [ GHOST[0] 0x02 ] => [ TMP[eq, 0] ]\l\
// "];
// Block7 -> Block7Exit;
// Block7Exit [label="{ TMP[eq, 0]| { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block7Exit:0 -> Block9;
// Block7Exit:1 -> Block10;
//
// Block8 [label="\
// sstore: [ 0x0404 0x04 ] => [ ]\l\
// "];
// Block8Exit [label="FunctionReturn[f]"];
// Block8 -> Block8Exit;
//
// Block9 [label="\
// eq: [ GHOST[0] 0x03 ] => [ TMP[eq, 0] ]\l\
// "];
// Block9 -> Block9Exit;
// Block9Exit [label="{ TMP[eq, 0]| { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block9Exit:0 -> Block11;
// Block9Exit:1 -> Block12;
//
// Block10 [label="\
// sstore: [ 0x0606 0x06 ] => [ ]\l\
// revert: [ 0x00 0x00 ] => [ ]\l\
// "];
// Block10Exit [label="Terminated"];
// Block10 -> Block10Exit;
//
// Block11 [label="\
// mload: [ b ] => [ TMP[mload, 0] ]\l\
// "];
// Block11 -> Block11Exit;
// Block11Exit [label="{ TMP[mload, 0]| { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block11Exit:0 -> Block13;
// Block11Exit:1 -> Block14;
//
// Block12 [label="\
// sstore: [ 0x0808 0x08 ] => [ ]\l\
// "];
// Block12 -> Block12Exit [arrowhead=none];
// Block12Exit [label="Jump" shape=oval];
// Block12Exit -> Block15;
//
// Block13 [label="\
// sstore: [ 0x0a0a 0x0a ] => [ ]\l\
// "];
// Block13 -> Block13Exit [arrowhead=none];
// Block13Exit [label="Jump" shape=oval];
// Block13Exit -> Block15;
//
// Block14 [label="\
// return: [ 0x00 0x00 ] => [ ]\l\
// "];
// Block14Exit [label="Terminated"];
// Block14 -> Block14Exit;
//
// Block15 [label="\
// sstore: [ 0x0b0b 0x0b ] => [ ]\l\
// "];
// Block15 -> Block15Exit [arrowhead=none];
// Block15Exit [label="Jump" shape=oval];
// Block15Exit -> Block16;
//
// Block16 [label="\
// add: [ 0x01 x ] => [ TMP[add, 0] ]\l\
// Assignment(x): [ TMP[add, 0] ] => [ x ]\l\
// calldataload: [ x ] => [ TMP[calldataload, 0] ]\l\
// "];
// Block16 -> Block16Exit;
// Block16Exit [label="{ TMP[calldataload, 0]| { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block16Exit:0 -> Block17;
// Block16Exit:1 -> Block18;
//
// Block17 [label="\
// sstore: [ 0xffff 0xff ] => [ ]\l\
// "];
// Block17 -> Block17Exit [arrowhead=none];
// Block17Exit [label="BackwardsJump" shape=oval];
// Block17Exit -> Block2;
//
// Block18 [label="\
// sstore: [ x 0x00 ] => [ ]\l\
// "];
// Block18Exit [label="FunctionReturn[f]"];
// Block18 -> Block18Exit;
//
// }
