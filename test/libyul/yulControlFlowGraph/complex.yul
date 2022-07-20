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
// "];
// Block4 -> Block4Exit;
// Block4Exit [label="{ TMP[mload, 0]| { <Default> Default | <0> 0 | <1> 1 | <2> 2 | <3> 3 }}" shape=Mrecord];
// Block4Exit:Default -> Block5;
// Block4Exit:0 -> Block6;
// Block4Exit:1 -> Block7;
// Block4Exit:2 -> Block8;
// Block4Exit:3 -> Block9;
//
// Block5 [label="\
// mload: [ b ] => [ TMP[mload, 0] ]\l\
// "];
// Block5 -> Block5Exit;
// Block5Exit [label="{ TMP[mload, 0]| { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block5Exit:0 -> Block10;
// Block5Exit:1 -> Block11;
//
// Block6 [label="\
// sstore: [ 0x0202 0x02 ] => [ ]\l\
// "];
// Block6 -> Block6Exit [arrowhead=none];
// Block6Exit [label="Jump" shape=oval];
// Block6Exit -> Block3;
//
// Block7 [label="\
// sstore: [ 0x0404 0x04 ] => [ ]\l\
// "];
// Block7Exit [label="FunctionReturn[f]"];
// Block7 -> Block7Exit;
//
// Block8 [label="\
// sstore: [ 0x0606 0x06 ] => [ ]\l\
// revert: [ 0x00 0x00 ] => [ ]\l\
// "];
// Block8Exit [label="Terminated"];
// Block8 -> Block8Exit;
//
// Block9 [label="\
// sstore: [ 0x0808 0x08 ] => [ ]\l\
// "];
// Block9 -> Block9Exit [arrowhead=none];
// Block9Exit [label="Jump" shape=oval];
// Block9Exit -> Block12;
//
// Block10 [label="\
// sstore: [ 0x0a0a 0x0a ] => [ ]\l\
// "];
// Block10 -> Block10Exit [arrowhead=none];
// Block10Exit [label="Jump" shape=oval];
// Block10Exit -> Block12;
//
// Block11 [label="\
// return: [ 0x00 0x00 ] => [ ]\l\
// "];
// Block11Exit [label="Terminated"];
// Block11 -> Block11Exit;
//
// Block12 [label="\
// sstore: [ 0x0b0b 0x0b ] => [ ]\l\
// "];
// Block12 -> Block12Exit [arrowhead=none];
// Block12Exit [label="Jump" shape=oval];
// Block12Exit -> Block13;
//
// Block13 [label="\
// add: [ 0x01 x ] => [ TMP[add, 0] ]\l\
// Assignment(x): [ TMP[add, 0] ] => [ x ]\l\
// calldataload: [ x ] => [ TMP[calldataload, 0] ]\l\
// "];
// Block13 -> Block13Exit;
// Block13Exit [label="{ TMP[calldataload, 0]| { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block13Exit:0 -> Block14;
// Block13Exit:1 -> Block15;
//
// Block14 [label="\
// sstore: [ 0xffff 0xff ] => [ ]\l\
// "];
// Block14 -> Block14Exit [arrowhead=none];
// Block14Exit [label="BackwardsJump" shape=oval];
// Block14Exit -> Block2;
//
// Block15 [label="\
// sstore: [ x 0x00 ] => [ ]\l\
// "];
// Block15Exit [label="FunctionReturn[f]"];
// Block15 -> Block15Exit;
//
// }
