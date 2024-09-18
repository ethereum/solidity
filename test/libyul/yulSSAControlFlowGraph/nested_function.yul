{
    function f(a, b) -> r {
        let x := add(a,b)
        r := sub(x,a)
    }
    function g() {
        function z() -> r {
            function w() -> rw1 {
                rw1 := 0x01
            }
            r := w()
        }
        function v() -> r {
            function w() -> rw2 {
                rw2 := 0x11
            }
            r := w()
        }
        sstore(z(), f(v(),2))
    }
    function cycle1() -> r {
        if mload(3) {
            r := cycle2()
        }
    }
    function cycle2() -> r {
        if mload(4) {
            r := cycle1()
        }
    }
}
// ----
// digraph SSACFG {
// nodesep=0.7;
// graph[fontname="DejaVu Sans"]
// node[shape=box,fontname="DejaVu Sans"];
//
// Entry0 [label="Entry"];
// Entry0 -> Block0_0;
// Block0_0 [label="\
// Block 0; (0, max 0)\nLiveIn: \l\
// LiveOut: \l\n"];
// Block0_0Exit [label="MainExit"];
// Block0_0 -> Block0_0Exit;
// FunctionEntry_f_0 [label="function f:
//  r := f(v0, v1)"];
// FunctionEntry_f_0 -> Block1_0;
// Block1_0 [label="\
// Block 0; (0, max 0)\nLiveIn: v0,v1\l\
// LiveOut: v4\l\nv3 := add(v1, v0)\l\
// v4 := sub(v0, v3)\l\
// "];
// Block1_0Exit [label="FunctionReturn[v4]"];
// Block1_0 -> Block1_0Exit;
// FunctionEntry_g_0 [label="function g:
//  g()"];
// FunctionEntry_g_0 -> Block2_0;
// Block2_0 [label="\
// Block 0; (0, max 0)\nLiveIn: \l\
// LiveOut: \l\nv1 := v()\l\
// v2 := f(2, v1)\l\
// v3 := z()\l\
// sstore(v2, v3)\l\
// "];
// Block2_0Exit [label="FunctionReturn[]"];
// Block2_0 -> Block2_0Exit;
// FunctionEntry_z_0 [label="function z:
//  r := z()"];
// FunctionEntry_z_0 -> Block3_0;
// Block3_0 [label="\
// Block 0; (0, max 0)\nLiveIn: \l\
// LiveOut: v1\l\nv1 := w()\l\
// "];
// Block3_0Exit [label="FunctionReturn[v1]"];
// Block3_0 -> Block3_0Exit;
// FunctionEntry_w_0 [label="function w:
//  rw1 := w()"];
// FunctionEntry_w_0 -> Block4_0;
// Block4_0 [label="\
// Block 0; (0, max 0)\nLiveIn: \l\
// LiveOut: \l\n"];
// Block4_0Exit [label="FunctionReturn[1]"];
// Block4_0 -> Block4_0Exit;
// FunctionEntry_v_0 [label="function v:
//  r := v()"];
// FunctionEntry_v_0 -> Block5_0;
// Block5_0 [label="\
// Block 0; (0, max 0)\nLiveIn: \l\
// LiveOut: v1\l\nv1 := w()\l\
// "];
// Block5_0Exit [label="FunctionReturn[v1]"];
// Block5_0 -> Block5_0Exit;
// FunctionEntry_w_0 [label="function w:
//  rw2 := w()"];
// FunctionEntry_w_0 -> Block6_0;
// Block6_0 [label="\
// Block 0; (0, max 0)\nLiveIn: \l\
// LiveOut: \l\n"];
// Block6_0Exit [label="FunctionReturn[17]"];
// Block6_0 -> Block6_0Exit;
// FunctionEntry_cycle1_0 [label="function cycle1:
//  r := cycle1()"];
// FunctionEntry_cycle1_0 -> Block7_0;
// Block7_0 [label="\
// Block 0; (0, max 2)\nLiveIn: \l\
// LiveOut: \l\nv2 := mload(3)\l\
// "];
// Block7_0 -> Block7_0Exit;
// Block7_0Exit [label="{ If v2 | { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block7_0Exit:0 -> Block7_2 [style="solid"];
// Block7_0Exit:1 -> Block7_1 [style="solid"];
// Block7_1 [label="\
// Block 1; (1, max 2)\nLiveIn: \l\
// LiveOut: v3\l\nv3 := cycle2()\l\
// "];
// Block7_1 -> Block7_1Exit [arrowhead=none];
// Block7_1Exit [label="Jump" shape=oval];
// Block7_1Exit -> Block7_2 [style="solid"];
// Block7_2 [label="\
// Block 2; (2, max 2)\nLiveIn: v4\l\
// LiveOut: v4\l\nv4 := φ(\l\
// 	Block 0 => 0,\l\
// 	Block 1 => v3\l\
// )\l\
// "];
// Block7_2Exit [label="FunctionReturn[v4]"];
// Block7_2 -> Block7_2Exit;
// FunctionEntry_cycle2_0 [label="function cycle2:
//  r := cycle2()"];
// FunctionEntry_cycle2_0 -> Block8_0;
// Block8_0 [label="\
// Block 0; (0, max 2)\nLiveIn: \l\
// LiveOut: \l\nv2 := mload(4)\l\
// "];
// Block8_0 -> Block8_0Exit;
// Block8_0Exit [label="{ If v2 | { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block8_0Exit:0 -> Block8_2 [style="solid"];
// Block8_0Exit:1 -> Block8_1 [style="solid"];
// Block8_1 [label="\
// Block 1; (1, max 2)\nLiveIn: \l\
// LiveOut: v3\l\nv3 := cycle1()\l\
// "];
// Block8_1 -> Block8_1Exit [arrowhead=none];
// Block8_1Exit [label="Jump" shape=oval];
// Block8_1Exit -> Block8_2 [style="solid"];
// Block8_2 [label="\
// Block 2; (2, max 2)\nLiveIn: v4\l\
// LiveOut: v4\l\nv4 := φ(\l\
// 	Block 0 => 0,\l\
// 	Block 1 => v3\l\
// )\l\
// "];
// Block8_2Exit [label="FunctionReturn[v4]"];
// Block8_2 -> Block8_2Exit;
// }
