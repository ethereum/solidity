{
    sstore(0x01, 0x0101)
    for { sstore(0x02, 0x0202) } sload(0x03) { sstore(0x04, 0x0404) } {
        sstore(0x05, 0x0505)
        if sload(0x06) { sstore(0x07,0x0707) break }
        sstore(0x08, 0x0808)
        if sload(0x09) { sstore(0x0A,0x0A0A) continue }
        sstore(0x0B, 0x0B0B)
    }
    sstore(0x0C, 0x0C0C)
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
// sstore: [ 0x0c0c 0x0c ] => [ ]\l\
// "];
// Block2Exit [label="MainExit"];
// Block2 -> Block2Exit;
//
// Block3 [label="\
// sstore: [ 0x0505 0x05 ] => [ ]\l\
// sload: [ 0x06 ] => [ TMP[sload, 0] ]\l\
// "];
// Block3 -> Block3Exit;
// Block3Exit [label="{ TMP[sload, 0]| { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block3Exit:0 -> Block4;
// Block3Exit:1 -> Block5;
//
// Block4 [label="\
// sstore: [ 0x0808 0x08 ] => [ ]\l\
// sload: [ 0x09 ] => [ TMP[sload, 0] ]\l\
// "];
// Block4 -> Block4Exit;
// Block4Exit [label="{ TMP[sload, 0]| { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block4Exit:0 -> Block6;
// Block4Exit:1 -> Block7;
//
// Block5 [label="\
// sstore: [ 0x0707 0x07 ] => [ ]\l\
// "];
// Block5 -> Block5Exit [arrowhead=none];
// Block5Exit [label="Jump" shape=oval];
// Block5Exit -> Block2;
//
// Block6 [label="\
// sstore: [ 0x0b0b 0x0b ] => [ ]\l\
// "];
// Block6 -> Block6Exit [arrowhead=none];
// Block6Exit [label="Jump" shape=oval];
// Block6Exit -> Block8;
//
// Block7 [label="\
// sstore: [ 0x0a0a 0x0a ] => [ ]\l\
// "];
// Block7 -> Block7Exit [arrowhead=none];
// Block7Exit [label="Jump" shape=oval];
// Block7Exit -> Block8;
//
// Block8 [label="\
// sstore: [ 0x0404 0x04 ] => [ ]\l\
// "];
// Block8 -> Block8Exit [arrowhead=none];
// Block8Exit [label="BackwardsJump" shape=oval];
// Block8Exit -> Block1;
//
// }
