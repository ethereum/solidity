{
    function f(a, b) -> r {
        let x := add(a,b)
        r := sub(x,a)
    }
    function g() {
        sstore(0x01, 0x0101)
    }
    function h(x) {
        h(f(x, 0))
        g()
    }
    function i() -> v, w {
        v := 0x0202
        w := 0x0303
    }
    let x, y := i()
    h(x)
    h(y)
}
// ----
// digraph CFG {
// nodesep=0.7;
// node[shape=box];
//
// Entry [label="Entry"];
// Entry -> Block0;
// Block0 [label="\
// i: [ RET[i] ] => [ TMP[i, 0] TMP[i, 1] ]\l\
// Assignment(x, y): [ TMP[i, 0] TMP[i, 1] ] => [ x y ]\l\
// h: [ x ] => [ ]\l\
// "];
// Block0Exit [label="Terminated"];
// Block0 -> Block0Exit;
//
// FunctionEntry_f_1 [label="function f(a, b) -> r"];
// FunctionEntry_f_1 -> Block1;
// Block1 [label="\
// add: [ b a ] => [ TMP[add, 0] ]\l\
// Assignment(x): [ TMP[add, 0] ] => [ x ]\l\
// sub: [ a x ] => [ TMP[sub, 0] ]\l\
// Assignment(r): [ TMP[sub, 0] ] => [ r ]\l\
// "];
// Block1Exit [label="FunctionReturn[f]"];
// Block1 -> Block1Exit;
//
// FunctionEntry_g_2 [label="function g()"];
// FunctionEntry_g_2 -> Block2;
// Block2 [label="\
// sstore: [ 0x0101 0x01 ] => [ ]\l\
// "];
// Block2Exit [label="FunctionReturn[g]"];
// Block2 -> Block2Exit;
//
// FunctionEntry_h_3 [label="function h(x)"];
// FunctionEntry_h_3 -> Block3;
// Block3 [label="\
// f: [ RET[f] 0x00 x ] => [ TMP[f, 0] ]\l\
// h: [ TMP[f, 0] ] => [ ]\l\
// "];
// Block3Exit [label="Terminated"];
// Block3 -> Block3Exit;
//
// FunctionEntry_i_4 [label="function i() -> v, w"];
// FunctionEntry_i_4 -> Block4;
// Block4 [label="\
// Assignment(v): [ 0x0202 ] => [ v ]\l\
// Assignment(w): [ 0x0303 ] => [ w ]\l\
// "];
// Block4Exit [label="FunctionReturn[i]"];
// Block4 -> Block4Exit;
//
// }
