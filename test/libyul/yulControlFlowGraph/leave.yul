{
    function f(a, b) -> c {
        sstore(0x01, 0x0101)
        if lt(a,b) {
            sstore(0x02, 0x0202)
            leave
            sstore(0x03, 0x0303)
        }
        sstore(0x04, 0x0404)
    }

    pop(f(0,1))
}
// ----
// digraph CFG {
// nodesep=0.7;
// node[shape=box];
//
// Entry [label="Entry"];
// Entry -> Block0;
// Block0 [label="\
// f: [ RET[f] 0x01 0x00 ] => [ TMP[f, 0] ]\l\
// pop: [ TMP[f, 0] ] => [ ]\l\
// "];
// Block0Exit [label="MainExit"];
// Block0 -> Block0Exit;
//
// FunctionEntry_f_1 [label="function f(a, b) -> c"];
// FunctionEntry_f_1 -> Block1;
// Block1 [label="\
// sstore: [ 0x0101 0x01 ] => [ ]\l\
// lt: [ b a ] => [ TMP[lt, 0] ]\l\
// "];
// Block1 -> Block1Exit;
// Block1Exit [label="{ TMP[lt, 0]| { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block1Exit:0 -> Block2;
// Block1Exit:1 -> Block3;
//
// Block2 [label="\
// sstore: [ 0x0404 0x04 ] => [ ]\l\
// "];
// Block2Exit [label="FunctionReturn[f]"];
// Block2 -> Block2Exit;
//
// Block3 [label="\
// sstore: [ 0x0202 0x02 ] => [ ]\l\
// "];
// Block3Exit [label="FunctionReturn[f]"];
// Block3 -> Block3Exit;
//
// }
