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
// [ ]\l\
// [ RET[i] ]\l\
// i\l\
// [ TMP[i, 0] TMP[i, 1] ]\l\
// [ TMP[i, 0] TMP[i, 1] ]\l\
// Assignment(x, y)\l\
// [ x y ]\l\
// [ x ]\l\
// h\l\
// [ ]\l\
// [ ]\l\
// "];
// Block0Exit [label="Terminated"];
// Block0 -> Block0Exit;
//
// FunctionEntry_f [label="function f(a, b) -> r\l\
// [ RET b a ]"];
// FunctionEntry_f -> Block1;
// Block1 [label="\
// [ RET a b ]\l\
// [ RET a b a ]\l\
// add\l\
// [ RET a TMP[add, 0] ]\l\
// [ RET a TMP[add, 0] ]\l\
// Assignment(x)\l\
// [ RET a x ]\l\
// [ RET a x ]\l\
// sub\l\
// [ RET TMP[sub, 0] ]\l\
// [ RET TMP[sub, 0] ]\l\
// Assignment(r)\l\
// [ RET r ]\l\
// [ r RET ]\l\
// "];
// Block1Exit [label="FunctionReturn[f]"];
// Block1 -> Block1Exit;
//
// FunctionEntry_g [label="function g()\l\
// [ RET ]"];
// FunctionEntry_g -> Block2;
// Block2 [label="\
// [ RET ]\l\
// [ RET 0x0101 0x01 ]\l\
// sstore\l\
// [ RET ]\l\
// [ RET ]\l\
// "];
// Block2Exit [label="FunctionReturn[g]"];
// Block2 -> Block2Exit;
//
// FunctionEntry_h [label="function h(x)\l\
// [ RET x ]"];
// FunctionEntry_h -> Block3;
// Block3 [label="\
// [ RET[f] 0x00 x ]\l\
// [ RET[f] 0x00 x ]\l\
// f\l\
// [ TMP[f, 0] ]\l\
// [ TMP[f, 0] ]\l\
// h\l\
// [ ]\l\
// [ ]\l\
// "];
// Block3Exit [label="Terminated"];
// Block3 -> Block3Exit;
//
// FunctionEntry_i [label="function i() -> v, w\l\
// [ RET ]"];
// FunctionEntry_i -> Block4;
// Block4 [label="\
// [ RET ]\l\
// [ RET 0x0202 ]\l\
// Assignment(v)\l\
// [ RET v ]\l\
// [ v RET 0x0303 ]\l\
// Assignment(w)\l\
// [ v RET w ]\l\
// [ v w RET ]\l\
// "];
// Block4Exit [label="FunctionReturn[i]"];
// Block4 -> Block4Exit;
//
// }
