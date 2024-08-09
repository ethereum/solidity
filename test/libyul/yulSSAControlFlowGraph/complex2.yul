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
        c:=27
    }
    sstore(0x1,0x1)
    pop(f(1,2))
    let z:= add(5,sload(0))
    let w := f(z,sload(4))
    sstore(z,w)
    let x := f(w,sload(5))
    sstore(0x1,x)
}
// ----
// digraph SSACFG {
// nodesep=0.7;
// node[shape=box];
//
// Entry [label="Entry"];
// Entry -> Block0;
// Block0 [label="\
// Block 0\nsstore(1, 1)\l\
// v78 := f(2, 1)\l\
// pop(v78)\l\
// v79 := sload(0)\l\
// v80 := add(v79, 5)\l\
// v81 := sload(4)\l\
// v82 := f(v81, v80)\l\
// sstore(v82, v80)\l\
// v83 := sload(5)\l\
// v84 := f(v83, v82)\l\
// sstore(v84, 1)\l\
// "];
// Block0Exit [label="MainExit"];
// Block0 -> Block0Exit;
// FunctionEntry_f_1 [label="function f:
//  c := f(v0, v1)"];
// FunctionEntry_f_1 -> Block1;
// Block1 [label="\
// Block 1\n"];
// Block1 -> Block1Exit [arrowhead=none];
// Block1Exit [label="Jump" shape=oval];
// Block1Exit -> Block2;
// Block2 [label="\
// Block 2\nv4 := φ(\l\
// 	Block 1 => 42,\l\
// 	Block 22 => v43\l\
// )\l\
// v5 := lt(v0, v4)\l\
// "];
// Block2 -> Block2Exit;
// Block2Exit [label="{ If v5| { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block2Exit:0 -> Block5;
// Block2Exit:1 -> Block3;
// Block3 [label="\
// Block 3\nv6 := mload(v4)\l\
// v7 := eq(0, v6)\l\
// "];
// Block3 -> Block3Exit;
// Block3Exit [label="{ If v7| { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block3Exit:0 -> Block8;
// Block3Exit:1 -> Block7;
// Block5 [label="\
// Block 5\nsstore(3084, 12)\l\
// "];
// Block5Exit [label="FunctionReturn[27]"];
// Block5 -> Block5Exit;
// Block7 [label="\
// Block 7\nsstore(514, 2)\l\
// "];
// Block7 -> Block7Exit [arrowhead=none];
// Block7Exit [label="Jump" shape=oval];
// Block7Exit -> Block5;
// Block8 [label="\
// Block 8\nv13 := eq(1, v6)\l\
// "];
// Block8 -> Block8Exit;
// Block8Exit [label="{ If v13| { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block8Exit:0 -> Block11;
// Block8Exit:1 -> Block10;
// Block10 [label="\
// Block 10\nsstore(1028, 4)\l\
// "];
// Block10Exit [label="FunctionReturn[v17]"];
// Block10 -> Block10Exit;
// Block11 [label="\
// Block 11\nv20 := eq(2, v6)\l\
// "];
// Block11 -> Block11Exit;
// Block11Exit [label="{ If v20| { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block11Exit:0 -> Block14;
// Block11Exit:1 -> Block13;
// Block13 [label="\
// Block 13\nsstore(1542, 6)\l\
// revert(0, 0)\l\
// "];
// Block13Exit [label="Terminated"];
// Block13 -> Block13Exit;
// Block14 [label="\
// Block 14\nv25 := eq(3, v6)\l\
// "];
// Block14 -> Block14Exit;
// Block14Exit [label="{ If v25| { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block14Exit:0 -> Block17;
// Block14Exit:1 -> Block16;
// Block16 [label="\
// Block 16\nsstore(2056, 8)\l\
// "];
// Block16 -> Block16Exit [arrowhead=none];
// Block16Exit [label="Jump" shape=oval];
// Block16Exit -> Block6;
// Block17 [label="\
// Block 17\nv29 := mload(v1)\l\
// "];
// Block17 -> Block17Exit;
// Block17Exit [label="{ If v29| { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block17Exit:0 -> Block19;
// Block17Exit:1 -> Block18;
// Block6 [label="\
// Block 6\nsstore(2827, 11)\l\
// "];
// Block6 -> Block6Exit [arrowhead=none];
// Block6Exit [label="Jump" shape=oval];
// Block6Exit -> Block4;
// Block18 [label="\
// Block 18\nreturn(0, 0)\l\
// "];
// Block18Exit [label="Terminated"];
// Block18 -> Block18Exit;
// Block19 [label="\
// Block 19\nsstore(2570, 10)\l\
// "];
// Block19 -> Block19Exit [arrowhead=none];
// Block19Exit [label="Jump" shape=oval];
// Block19Exit -> Block6;
// Block4 [label="\
// Block 4\nv43 := add(1, v4)\l\
// v44 := calldataload(v43)\l\
// "];
// Block4 -> Block4Exit;
// Block4Exit [label="{ If v44| { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block4Exit:0 -> Block22;
// Block4Exit:1 -> Block21;
// Block21 [label="\
// Block 21\nsstore(v43, 0)\l\
// "];
// Block21Exit [label="FunctionReturn[v45]"];
// Block21 -> Block21Exit;
// Block22 [label="\
// Block 22\nsstore(65535, 255)\l\
// "];
// Block22 -> Block22Exit [arrowhead=none];
// Block22Exit [label="Jump" shape=oval];
// Block22Exit -> Block2;
// }
