{
    let x := calldataload(0)
    let y := calldataload(2)

    x := calldataload(3)
    y := calldataload(4)

    sstore(x,y)
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
// [ 0x00 ]\l\
// calldataload\l\
// [ TMP[calldataload, 0] ]\l\
// [ TMP[calldataload, 0] ]\l\
// Assignment(x)\l\
// [ x ]\l\
// [ 0x02 ]\l\
// calldataload\l\
// [ TMP[calldataload, 0] ]\l\
// [ TMP[calldataload, 0] ]\l\
// Assignment(y)\l\
// [ y ]\l\
// [ 0x03 ]\l\
// calldataload\l\
// [ TMP[calldataload, 0] ]\l\
// [ TMP[calldataload, 0] ]\l\
// Assignment(x)\l\
// [ x ]\l\
// [ x 0x04 ]\l\
// calldataload\l\
// [ x TMP[calldataload, 0] ]\l\
// [ x TMP[calldataload, 0] ]\l\
// Assignment(y)\l\
// [ x y ]\l\
// [ y x ]\l\
// sstore\l\
// [ ]\l\
// [ ]\l\
// "];
// Block0Exit [label="MainExit"];
// Block0 -> Block0Exit;
//
// }
