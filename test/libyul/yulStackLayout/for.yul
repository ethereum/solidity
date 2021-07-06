{
    let x := 0x01
    let y := 0x02
    sstore(0x01, x)
    for { sstore(0x02, 0x0202) } lt(x, 0x0303) { x := add(x,0x0404) } {
        sstore(0x05, 0x0505)
        y := sload(x)
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
// [ ]\l\
// [ 0x0404 0x0404 0x01 ]\l\
// Assignment(x)\l\
// [ 0x0404 0x0404 x ]\l\
// [ 0x0404 0x0404 x 0x02 ]\l\
// Assignment(y)\l\
// [ 0x0404 0x0404 x y ]\l\
// [ 0x0404 0x0404 x x 0x01 ]\l\
// sstore\l\
// [ 0x0404 0x0404 x ]\l\
// [ 0x0404 0x0404 x 0x0202 0x02 ]\l\
// sstore\l\
// [ 0x0404 0x0404 x ]\l\
// [ 0x0404 0x0404 x ]\l\
// "];
// Block0 -> Block0Exit [arrowhead=none];
// Block0Exit [label="Jump" shape=oval];
// Block0Exit -> Block1;
//
// Block1 [label="\
// [ 0x0404 0x0404 x ]\l\
// [ 0x0404 0x0404 x 0x0303 x ]\l\
// lt\l\
// [ 0x0404 0x0404 x TMP[lt, 0] ]\l\
// [ 0x0404 0x0404 x TMP[lt, 0] ]\l\
// "];
// Block1 -> Block1Exit;
// Block1Exit [label="{ TMP[lt, 0]| { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block1Exit:0 -> Block2;
// Block1Exit:1 -> Block3;
//
// Block2 [label="\
// [ JUNK JUNK JUNK ]\l\
// [ 0x0506 0x06 ]\l\
// sstore\l\
// [ ]\l\
// [ ]\l\
// "];
// Block2Exit [label="MainExit"];
// Block2 -> Block2Exit;
//
// Block3 [label="\
// [ 0x0404 0x0404 x ]\l\
// [ 0x0404 0x0404 x 0x0505 0x05 ]\l\
// sstore\l\
// [ 0x0404 0x0404 x ]\l\
// [ 0x0404 0x0404 x x ]\l\
// sload\l\
// [ 0x0404 0x0404 x TMP[sload, 0] ]\l\
// [ 0x0404 0x0404 x TMP[sload, 0] ]\l\
// Assignment(y)\l\
// [ 0x0404 0x0404 x y ]\l\
// [ 0x0404 0x0404 x ]\l\
// "];
// Block3 -> Block3Exit [arrowhead=none];
// Block3Exit [label="Jump" shape=oval];
// Block3Exit -> Block4;
//
// Block4 [label="\
// [ 0x0404 0x0404 x ]\l\
// [ 0x0404 0x0404 x ]\l\
// add\l\
// [ 0x0404 TMP[add, 0] ]\l\
// [ 0x0404 TMP[add, 0] ]\l\
// Assignment(x)\l\
// [ 0x0404 x ]\l\
// [ 0x0404 x ]\l\
// "];
// Block4 -> Block4Exit [arrowhead=none];
// Block4Exit [label="BackwardsJump" shape=oval];
// Block4Exit -> Block1;
//
// }
