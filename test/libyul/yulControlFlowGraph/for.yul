{
    sstore(0x01, 0x0101)
    for { sstore(0x02, 0x0202) } sload(0x03) { sstore(0x04, 0x0404) } {
        sstore(0x05, 0x0505)
    }
    sstore(0x06, 0x0506)
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
// sstore: [ 0x0202 0x02 ] => [ ]\l\
// "];
// Block0 -> Block0Exit [arrowhead=none];
// Block0Exit [label="Jump" shape=oval];
// Block0Exit -> Block1;
//
// Block1 [label="\
// sload: [ 0x03 ] => [ TMP[sload, 0] ]\l\
// "];
// Block1 -> Block1Exit;
// Block1Exit [label="{ TMP[sload, 0]| { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block1Exit:0 -> Block2;
// Block1Exit:1 -> Block3;
//
// Block2 [label="\
// sstore: [ 0x0506 0x06 ] => [ ]\l\
// "];
// Block2Exit [label="MainExit"];
// Block2 -> Block2Exit;
//
// Block3 [label="\
// sstore: [ 0x0505 0x05 ] => [ ]\l\
// "];
// Block3 -> Block3Exit [arrowhead=none];
// Block3Exit [label="Jump" shape=oval];
// Block3Exit -> Block4;
//
// Block4 [label="\
// sstore: [ 0x0404 0x04 ] => [ ]\l\
// "];
// Block4 -> Block4Exit [arrowhead=none];
// Block4Exit [label="BackwardsJump" shape=oval];
// Block4Exit -> Block1;
//
// }
