{
    sstore(0x01, 0x0101)
    if calldataload(0) {
        sstore(0x02, 0x0202)
    }
    sstore(0x03, 0x003)
}
// ----
// digraph CFG {
// nodesep=0.7;
// node[shape=box];
//
// Entry [label="Entry"];
// Entry -> Block0;
// Block0 [label="\
// sstore: [ 0x0101 0x01 ] => [ ]\l\
// calldataload: [ 0x00 ] => [ TMP[calldataload, 0] ]\l\
// "];
// Block0 -> Block0Exit;
// Block0Exit [label="{ TMP[calldataload, 0]| { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block0Exit:0 -> Block1;
// Block0Exit:1 -> Block2;
//
// Block1 [label="\
// sstore: [ 0x03 0x03 ] => [ ]\l\
// "];
// Block1Exit [label="MainExit"];
// Block1 -> Block1Exit;
//
// Block2 [label="\
// sstore: [ 0x0202 0x02 ] => [ ]\l\
// "];
// Block2 -> Block2Exit [arrowhead=none];
// Block2Exit [label="Jump" shape=oval];
// Block2Exit -> Block1;
//
// }
