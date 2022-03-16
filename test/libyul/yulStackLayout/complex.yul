{
    function f(a, b) -> c {
        for { let x := 42 } lt(x, a) {
            x := add(x, 1)
            if calldataload(x)
            {
                sstore(0, x)
                c := 0x21
                leave
                sstore(0x01, 0x0101)
            }
            sstore(0xFF, 0xFFFF)
        }
        {
            switch mload(x)
            case 0 {
                sstore(a, b)
                break
                sstore(a, b)
            }
            case 1 {
                sstore(0x04, x)
                leave
                sstore(a, 0x0505)
            }
            case 2 {
                sstore(x, 0x06)
                c := 42
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
        if sload(0x0D) {
            c := 0x424242
        }
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
// [ ]\l\
// [ RET[f] 0x02 0x01 ]\l\
// f\l\
// [ TMP[f, 0] ]\l\
// [ TMP[f, 0] ]\l\
// pop\l\
// [ ]\l\
// [ ]\l\
// "];
// Block0Exit [label="MainExit"];
// Block0 -> Block0Exit;
//
// FunctionEntry_f [label="function f(a, b) -> c\l\
// [ RET b a ]"];
// FunctionEntry_f -> Block1;
// Block1 [label="\
// [ c RET a b ]\l\
// [ c RET a b 0x2a ]\l\
// Assignment(x)\l\
// [ c RET a b x ]\l\
// [ c RET a b x ]\l\
// "];
// Block1 -> Block1Exit [arrowhead=none];
// Block1Exit [label="Jump" shape=oval];
// Block1Exit -> Block2;
//
// Block2 [label="\
// [ c RET a b x ]\l\
// [ c RET a b x a x ]\l\
// lt\l\
// [ c RET a b x TMP[lt, 0] ]\l\
// [ c RET a b x TMP[lt, 0] ]\l\
// "];
// Block2 -> Block2Exit;
// Block2Exit [label="{ TMP[lt, 0]| { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block2Exit:0 -> Block3;
// Block2Exit:1 -> Block4;
//
// Block3 [label="\
// [ c RET JUNK JUNK JUNK ]\l\
// [ c RET 0x0c0c 0x0c ]\l\
// sstore\l\
// [ c RET ]\l\
// [ c RET 0x0d ]\l\
// sload\l\
// [ c RET TMP[sload, 0] ]\l\
// [ c RET TMP[sload, 0] ]\l\
// "];
// Block3 -> Block3Exit;
// Block3Exit [label="{ TMP[sload, 0]| { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block3Exit:0 -> Block5;
// Block3Exit:1 -> Block6;
//
// Block4 [label="\
// [ c RET a b x ]\l\
// [ c RET a b x x ]\l\
// mload\l\
// [ c RET a b x TMP[mload, 0] ]\l\
// [ c RET a b x TMP[mload, 0] ]\l\
// Assignment(GHOST[0])\l\
// [ c RET a b x GHOST[0] ]\l\
// [ c RET a b x GHOST[0] GHOST[0] 0x00 ]\l\
// eq\l\
// [ c RET a b x GHOST[0] TMP[eq, 0] ]\l\
// [ c RET a b x GHOST[0] TMP[eq, 0] ]\l\
// "];
// Block4 -> Block4Exit;
// Block4Exit [label="{ TMP[eq, 0]| { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block4Exit:0 -> Block7;
// Block4Exit:1 -> Block8;
//
// Block5 [label="\
// [ c RET ]\l\
// [ c RET ]\l\
// "];
// Block5Exit [label="FunctionReturn[f]"];
// Block5 -> Block5Exit;
//
// Block6 [label="\
// [ JUNK RET ]\l\
// [ RET 0x424242 ]\l\
// Assignment(c)\l\
// [ RET c ]\l\
// [ c RET ]\l\
// "];
// Block6 -> Block6Exit [arrowhead=none];
// Block6Exit [label="Jump" shape=oval];
// Block6Exit -> Block5;
//
// Block7 [label="\
// [ c RET a b x GHOST[0] ]\l\
// [ c RET a b x GHOST[0] GHOST[0] 0x01 ]\l\
// eq\l\
// [ c RET a b x GHOST[0] TMP[eq, 0] ]\l\
// [ c RET a b x GHOST[0] TMP[eq, 0] ]\l\
// "];
// Block7 -> Block7Exit;
// Block7Exit [label="{ TMP[eq, 0]| { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block7Exit:0 -> Block9;
// Block7Exit:1 -> Block10;
//
// Block8 [label="\
// [ c RET a b JUNK JUNK ]\l\
// [ c RET b a ]\l\
// sstore\l\
// [ c RET ]\l\
// [ c RET ]\l\
// "];
// Block8 -> Block8Exit [arrowhead=none];
// Block8Exit [label="Jump" shape=oval];
// Block8Exit -> Block3;
//
// Block9 [label="\
// [ c RET a b x GHOST[0] ]\l\
// [ c RET a b x GHOST[0] GHOST[0] 0x02 ]\l\
// eq\l\
// [ c RET a b x GHOST[0] TMP[eq, 0] ]\l\
// [ c RET a b x GHOST[0] TMP[eq, 0] ]\l\
// "];
// Block9 -> Block9Exit;
// Block9Exit [label="{ TMP[eq, 0]| { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block9Exit:0 -> Block11;
// Block9Exit:1 -> Block12;
//
// Block10 [label="\
// [ c RET JUNK JUNK x JUNK ]\l\
// [ c RET x 0x04 ]\l\
// sstore\l\
// [ c RET ]\l\
// [ c RET ]\l\
// "];
// Block10Exit [label="FunctionReturn[f]"];
// Block10 -> Block10Exit;
//
// Block11 [label="\
// [ c RET a b x GHOST[0] ]\l\
// [ c RET a b x GHOST[0] 0x03 ]\l\
// eq\l\
// [ c RET a b x TMP[eq, 0] ]\l\
// [ c RET a b x TMP[eq, 0] ]\l\
// "];
// Block11 -> Block11Exit;
// Block11Exit [label="{ TMP[eq, 0]| { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block11Exit:0 -> Block13;
// Block11Exit:1 -> Block14;
//
// Block12 [label="\
// [ JUNK JUNK JUNK JUNK x JUNK ]\l\
// [ JUNK JUNK JUNK JUNK JUNK JUNK 0x06 x ]\l\
// sstore\l\
// [ JUNK JUNK JUNK JUNK JUNK JUNK ]\l\
// [ JUNK JUNK JUNK JUNK JUNK JUNK 0x2a ]\l\
// Assignment(c)\l\
// [ JUNK JUNK JUNK JUNK JUNK JUNK c ]\l\
// [ JUNK JUNK JUNK JUNK JUNK JUNK 0x00 0x00 ]\l\
// revert\l\
// [ JUNK JUNK JUNK JUNK JUNK JUNK ]\l\
// [ JUNK JUNK JUNK JUNK JUNK JUNK ]\l\
// "];
// Block12Exit [label="Terminated"];
// Block12 -> Block12Exit;
//
// Block13 [label="\
// [ c RET a b x ]\l\
// [ c RET a b x b ]\l\
// mload\l\
// [ c RET a b x TMP[mload, 0] ]\l\
// [ c RET a b x TMP[mload, 0] ]\l\
// "];
// Block13 -> Block13Exit;
// Block13Exit [label="{ TMP[mload, 0]| { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block13Exit:0 -> Block15;
// Block13Exit:1 -> Block16;
//
// Block14 [label="\
// [ c RET a b x ]\l\
// [ c RET a b 0x01 x 0x0808 0x08 ]\l\
// sstore\l\
// [ c RET a b 0x01 x ]\l\
// [ c RET a b 0x01 x ]\l\
// "];
// Block14 -> Block14Exit [arrowhead=none];
// Block14Exit [label="Jump" shape=oval];
// Block14Exit -> Block17;
//
// Block15 [label="\
// [ c RET a b x ]\l\
// [ c RET a b 0x01 x 0x0a0a 0x0a ]\l\
// sstore\l\
// [ c RET a b 0x01 x ]\l\
// [ c RET a b 0x01 x ]\l\
// "];
// Block15 -> Block15Exit [arrowhead=none];
// Block15Exit [label="Jump" shape=oval];
// Block15Exit -> Block17;
//
// Block16 [label="\
// [ JUNK JUNK JUNK JUNK JUNK ]\l\
// [ JUNK JUNK JUNK JUNK JUNK 0x00 0x00 ]\l\
// return\l\
// [ JUNK JUNK JUNK JUNK JUNK ]\l\
// [ JUNK JUNK JUNK JUNK JUNK ]\l\
// "];
// Block16Exit [label="Terminated"];
// Block16 -> Block16Exit;
//
// Block17 [label="\
// [ c RET a b 0x01 x ]\l\
// [ c RET a b 0x01 x 0x0b0b 0x0b ]\l\
// sstore\l\
// [ c RET a b 0x01 x ]\l\
// [ c RET a b 0x01 x ]\l\
// "];
// Block17 -> Block17Exit [arrowhead=none];
// Block17Exit [label="Jump" shape=oval];
// Block17Exit -> Block18;
//
// Block18 [label="\
// [ c RET a b 0x01 x ]\l\
// [ c RET a b 0x01 x ]\l\
// add\l\
// [ c RET a b TMP[add, 0] ]\l\
// [ c RET a b TMP[add, 0] ]\l\
// Assignment(x)\l\
// [ c RET a b x ]\l\
// [ c RET x b a x ]\l\
// calldataload\l\
// [ c RET x b a TMP[calldataload, 0] ]\l\
// [ c RET x b a TMP[calldataload, 0] ]\l\
// "];
// Block18 -> Block18Exit;
// Block18Exit [label="{ TMP[calldataload, 0]| { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block18Exit:0 -> Block19;
// Block18Exit:1 -> Block20;
//
// Block19 [label="\
// [ c RET x b a ]\l\
// [ c RET x b a 0xffff 0xff ]\l\
// sstore\l\
// [ c RET x b a ]\l\
// [ c RET x b a ]\l\
// "];
// Block19 -> Block19Exit [arrowhead=none];
// Block19Exit [label="BackwardsJump" shape=oval];
// Block19Exit -> Block2;
//
// Block20 [label="\
// [ JUNK RET x JUNK JUNK ]\l\
// [ RET x 0x00 ]\l\
// sstore\l\
// [ RET ]\l\
// [ RET 0x21 ]\l\
// Assignment(c)\l\
// [ RET c ]\l\
// [ c RET ]\l\
// "];
// Block20Exit [label="FunctionReturn[f]"];
// Block20 -> Block20Exit;
//
// }
