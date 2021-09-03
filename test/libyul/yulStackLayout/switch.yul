{
    let x := 0x0101
    let y := 0x0202
    let z := 0x0303
    switch sload(x)
    case 0 {
        x := 0x42
    }
    case 1 {
        y := 0x42
    }
    default {
        sstore(z, z)
    }

    sstore(0x0404, y)
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
// [ 0x0101 ]\l\
// Assignment(x)\l\
// [ x ]\l\
// [ x 0x0202 ]\l\
// Assignment(y)\l\
// [ x y ]\l\
// [ y x 0x0303 ]\l\
// Assignment(z)\l\
// [ y x z ]\l\
// [ y z x ]\l\
// sload\l\
// [ y z TMP[sload, 0] ]\l\
// [ y z TMP[sload, 0] ]\l\
// Assignment(GHOST[0])\l\
// [ y z GHOST[0] ]\l\
// [ y z GHOST[0] GHOST[0] 0x00 ]\l\
// eq\l\
// [ y z GHOST[0] TMP[eq, 0] ]\l\
// [ y z GHOST[0] TMP[eq, 0] ]\l\
// "];
// Block0 -> Block0Exit;
// Block0Exit [label="{ TMP[eq, 0]| { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block0Exit:0 -> Block1;
// Block0Exit:1 -> Block2;
//
// Block1 [label="\
// [ y z GHOST[0] ]\l\
// [ y z GHOST[0] 0x01 ]\l\
// eq\l\
// [ y z TMP[eq, 0] ]\l\
// [ y z TMP[eq, 0] ]\l\
// "];
// Block1 -> Block1Exit;
// Block1Exit [label="{ TMP[eq, 0]| { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block1Exit:0 -> Block3;
// Block1Exit:1 -> Block4;
//
// Block2 [label="\
// [ y JUNK JUNK ]\l\
// [ y 0x42 ]\l\
// Assignment(x)\l\
// [ y x ]\l\
// [ y ]\l\
// "];
// Block2 -> Block2Exit [arrowhead=none];
// Block2Exit [label="Jump" shape=oval];
// Block2Exit -> Block5;
//
// Block3 [label="\
// [ y z ]\l\
// [ y z z ]\l\
// sstore\l\
// [ y ]\l\
// [ y ]\l\
// "];
// Block3 -> Block3Exit [arrowhead=none];
// Block3Exit [label="Jump" shape=oval];
// Block3Exit -> Block5;
//
// Block4 [label="\
// [ JUNK JUNK ]\l\
// [ 0x42 ]\l\
// Assignment(y)\l\
// [ y ]\l\
// [ y ]\l\
// "];
// Block4 -> Block4Exit [arrowhead=none];
// Block4Exit [label="Jump" shape=oval];
// Block4Exit -> Block5;
//
// Block5 [label="\
// [ y ]\l\
// [ y 0x0404 ]\l\
// sstore\l\
// [ ]\l\
// [ ]\l\
// "];
// Block5Exit [label="MainExit"];
// Block5 -> Block5Exit;
//
// }
