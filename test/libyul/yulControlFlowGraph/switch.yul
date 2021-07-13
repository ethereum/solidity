{
    sstore(0, 0)
    switch sload(0)
    case 0 {
        sstore(0x01, 0x0101)
    }
    case 1 {
        sstore(0x02, 0x0101)
    }
    default {
        sstore(0x03, 0x0101)
    }
    sstore(0x04, 0x0101)
}
// ----
// digraph CFG {
// nodesep=0.7;
// node[shape=box];
//
// Entry [label="Entry"];
// Entry -> Block0;
// Block0 [label="\
// sstore: [ 0x00 0x00 ] => [ ]\l\
// sload: [ 0x00 ] => [ TMP[sload, 0] ]\l\
// Assignment(GHOST[0]): [ TMP[sload, 0] ] => [ GHOST[0] ]\l\
// eq: [ GHOST[0] 0x00 ] => [ TMP[eq, 0] ]\l\
// "];
// Block0 -> Block0Exit;
// Block0Exit [label="{ TMP[eq, 0]| { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block0Exit:0 -> Block1;
// Block0Exit:1 -> Block2;
//
// Block1 [label="\
// eq: [ GHOST[0] 0x01 ] => [ TMP[eq, 0] ]\l\
// "];
// Block1 -> Block1Exit;
// Block1Exit [label="{ TMP[eq, 0]| { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block1Exit:0 -> Block3;
// Block1Exit:1 -> Block4;
//
// Block2 [label="\
// sstore: [ 0x0101 0x01 ] => [ ]\l\
// "];
// Block2 -> Block2Exit [arrowhead=none];
// Block2Exit [label="Jump" shape=oval];
// Block2Exit -> Block5;
//
// Block3 [label="\
// sstore: [ 0x0101 0x03 ] => [ ]\l\
// "];
// Block3 -> Block3Exit [arrowhead=none];
// Block3Exit [label="Jump" shape=oval];
// Block3Exit -> Block5;
//
// Block4 [label="\
// sstore: [ 0x0101 0x02 ] => [ ]\l\
// "];
// Block4 -> Block4Exit [arrowhead=none];
// Block4Exit [label="Jump" shape=oval];
// Block4Exit -> Block5;
//
// Block5 [label="\
// sstore: [ 0x0101 0x04 ] => [ ]\l\
// "];
// Block5Exit [label="MainExit"];
// Block5 -> Block5Exit;
//
// }
