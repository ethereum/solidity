{
        let a_1 := 42
        let a_2 := 23
        let a_3 := 1
        let b := verbatim_10i_1o("test", a_1, a_2, a_3, 2, 3, 4, 5, 6, 7, 8)
        sstore(b,b)
}
// ----
// digraph CFG {
// nodesep=0.7;
// node[shape=box];
//
// Entry [label="Entry"];
// Entry -> Block0;
// Block0 [label="\
// Assignment(a_1): [ 0x2a ] => [ a_1 ]\l\
// Assignment(a_2): [ 0x17 ] => [ a_2 ]\l\
// Assignment(a_3): [ 0x01 ] => [ a_3 ]\l\
// verbatim_10i_1o: [ 0x08 0x07 0x06 0x05 0x04 0x03 0x02 a_3 a_2 a_1 ] => [ TMP[verbatim_10i_1o, 0] ]\l\
// Assignment(b): [ TMP[verbatim_10i_1o, 0] ] => [ b ]\l\
// sstore: [ b b ] => [ ]\l\
// "];
// Block0Exit [label="MainExit"];
// Block0 -> Block0Exit;
//
// }
