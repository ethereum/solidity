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
// digraph SSACFG {
// nodesep=0.7;
// node[shape=box];
//
// Entry [label="Entry"];
// Entry -> Block0;
// Block0 [label="\
// Block 0\nv11, v12 := i()\l\
// h(v11)\l\
// "];
// Block0Exit [label="Terminated"];
// Block0 -> Block0Exit;
// FunctionEntry_f_1 [label="function f:
//  r := f(v0, v1)"];
// FunctionEntry_f_1 -> Block1;
// Block1 [label="\
// Block 1\nv3 := add(v1, v0)\l\
// v4 := sub(v0, v3)\l\
// "];
// Block1Exit [label="FunctionReturn[v4]"];
// Block1 -> Block1Exit;
// FunctionEntry_g_2 [label="function g:
//  g()"];
// FunctionEntry_g_2 -> Block2;
// Block2 [label="\
// Block 2\nsstore(257, 1)\l\
// "];
// Block2Exit [label="FunctionReturn[]"];
// Block2 -> Block2Exit;
// FunctionEntry_h_3 [label="function h:
//  h(v2)"];
// FunctionEntry_h_3 -> Block3;
// Block3 [label="\
// Block 3\nv8 := f(0, v2)\l\
// h(v8)\l\
// "];
// Block3Exit [label="Terminated"];
// Block3 -> Block3Exit;
// FunctionEntry_i_4 [label="function i:
//  v, w := i()"];
// FunctionEntry_i_4 -> Block4;
// Block4 [label="\
// Block 4\n"];
// Block4Exit [label="FunctionReturn[514, 771]"];
// Block4 -> Block4Exit;
// }
