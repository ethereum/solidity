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
// calldataload: [ 0x00 ] => [ TMP[calldataload, 0] ]\l\
// Assignment(x): [ TMP[calldataload, 0] ] => [ x ]\l\
// calldataload: [ 0x02 ] => [ TMP[calldataload, 0] ]\l\
// Assignment(y): [ TMP[calldataload, 0] ] => [ y ]\l\
// calldataload: [ 0x03 ] => [ TMP[calldataload, 0] ]\l\
// Assignment(x): [ TMP[calldataload, 0] ] => [ x ]\l\
// calldataload: [ 0x04 ] => [ TMP[calldataload, 0] ]\l\
// Assignment(y): [ TMP[calldataload, 0] ] => [ y ]\l\
// sstore: [ y x ] => [ ]\l\
// "];
// Block0Exit [label="MainExit"];
// Block0 -> Block0Exit;
//
// }
