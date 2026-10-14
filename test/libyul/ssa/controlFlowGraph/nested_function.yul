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
// Entry [label="Entry"];
// Entry -> Block0_0;
// Block0_0 [fillcolor="#FF746C", style=filled, label="\
// Block 0; (0, max 0)\nLiveIn: \l\
// LiveOut: \l\nUsed: \l\n"];
// Block0_0Exit [label="MainExit"];
// Block0_0 -> Block0_0Exit;
// FunctionEntry_f_0 [label="function f:
//  [1 returns] := f(v0, v1)"];
// FunctionEntry_f_0 -> Block1_0;
// Block1_0 [label="\
// Block 0; (0, max 0)\nLiveIn: v0[2], v1[1]\l\
// LiveOut: v4[1]\l\nUsed: v0[2], v1[1]\l\nv3 := add(v0, v1)\l\
// v4 := sub(v3, v0)\l\
// "];
// Block1_0Exit [label="FunctionReturn[v4]"];
// Block1_0 -> Block1_0Exit;
// FunctionEntry_g_0 [label="function g:
//  g()"];
// FunctionEntry_g_0 -> Block2_0;
// Block2_0 [label="\
// Block 0; (0, max 0)\nLiveIn: \l\
// LiveOut: \l\nUsed: \l\nv1 := v()\l\
// v2 := f(v1, 0x02)\l\
// v3 := z()\l\
// sstore(v3, v2)\l\
// "];
// Block2_0Exit [label="FunctionReturn[]"];
// Block2_0 -> Block2_0Exit;
// FunctionEntry_cycle1_0 [label="function cycle1:
//  [1 returns] := cycle1()"];
// FunctionEntry_cycle1_0 -> Block3_0;
// Block3_0 [label="\
// Block 0; (0, max 3)\nLiveIn: \l\
// LiveOut: \l\nUsed: \l\nv2 := mload(0x03)\l\
// "];
// Block3_0 -> Block3_0Exit;
// Block3_0Exit [label="{ If v2 | { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block3_0Exit:0 -> Block3_3 [style="solid"];
// Block3_0Exit:1 -> Block3_1 [style="solid"];
// Block3_1 [label="\
// Block 1; (1, max 2)\nLiveIn: \l\
// LiveOut: v3[1]\l\nUsed: \l\nv3 := cycle2()\l\
// "];
// Block3_1 -> Block3_1Exit [arrowhead=none];
// Block3_1Exit [label="Jump" shape=oval];
// Block3_1Exit -> Block3_2 [style="solid"];
// Block3_3 [label="\
// Block 3; (3, max 3)\nLiveIn: \l\
// LiveOut: \l\nUsed: \l\n"];
// Block3_3 -> Block3_3Exit [arrowhead=none];
// Block3_3Exit [label="Jump" shape=oval];
// Block3_3Exit -> Block3_2 [style="solid"];
// Block3_2 [label="\
// Block 2; (2, max 2)\nLiveIn: phi4[3]\l\
// LiveOut: phi4[1]\l\nUsed: phi4[2]\l\nphi4 := φ(\l\
// 	Block 1 => v3,\l\
// 	Block 3 => 0x00\l\
// )\l\
// "];
// Block3_2Exit [label="FunctionReturn[phi4]"];
// Block3_2 -> Block3_2Exit;
// FunctionEntry_cycle2_0 [label="function cycle2:
//  [1 returns] := cycle2()"];
// FunctionEntry_cycle2_0 -> Block4_0;
// Block4_0 [label="\
// Block 0; (0, max 3)\nLiveIn: \l\
// LiveOut: \l\nUsed: \l\nv2 := mload(0x04)\l\
// "];
// Block4_0 -> Block4_0Exit;
// Block4_0Exit [label="{ If v2 | { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block4_0Exit:0 -> Block4_3 [style="solid"];
// Block4_0Exit:1 -> Block4_1 [style="solid"];
// Block4_1 [label="\
// Block 1; (1, max 2)\nLiveIn: \l\
// LiveOut: v3[1]\l\nUsed: \l\nv3 := cycle1()\l\
// "];
// Block4_1 -> Block4_1Exit [arrowhead=none];
// Block4_1Exit [label="Jump" shape=oval];
// Block4_1Exit -> Block4_2 [style="solid"];
// Block4_3 [label="\
// Block 3; (3, max 3)\nLiveIn: \l\
// LiveOut: \l\nUsed: \l\n"];
// Block4_3 -> Block4_3Exit [arrowhead=none];
// Block4_3Exit [label="Jump" shape=oval];
// Block4_3Exit -> Block4_2 [style="solid"];
// Block4_2 [label="\
// Block 2; (2, max 2)\nLiveIn: phi4[3]\l\
// LiveOut: phi4[1]\l\nUsed: phi4[2]\l\nphi4 := φ(\l\
// 	Block 1 => v3,\l\
// 	Block 3 => 0x00\l\
// )\l\
// "];
// Block4_2Exit [label="FunctionReturn[phi4]"];
// Block4_2 -> Block4_2Exit;
// FunctionEntry_z_0 [label="function z:
//  [1 returns] := z()"];
// FunctionEntry_z_0 -> Block5_0;
// Block5_0 [label="\
// Block 0; (0, max 0)\nLiveIn: \l\
// LiveOut: v1[1]\l\nUsed: \l\nv1 := w()\l\
// "];
// Block5_0Exit [label="FunctionReturn[v1]"];
// Block5_0 -> Block5_0Exit;
// FunctionEntry_v_0 [label="function v:
//  [1 returns] := v()"];
// FunctionEntry_v_0 -> Block6_0;
// Block6_0 [label="\
// Block 0; (0, max 0)\nLiveIn: \l\
// LiveOut: v1[1]\l\nUsed: \l\nv1 := w()\l\
// "];
// Block6_0Exit [label="FunctionReturn[v1]"];
// Block6_0 -> Block6_0Exit;
// FunctionEntry_w_0 [label="function w:
//  [1 returns] := w()"];
// FunctionEntry_w_0 -> Block7_0;
// Block7_0 [label="\
// Block 0; (0, max 0)\nLiveIn: \l\
// LiveOut: \l\nUsed: \l\n"];
// Block7_0Exit [label="FunctionReturn[0x01]"];
// Block7_0 -> Block7_0Exit;
// FunctionEntry_w_0 [label="function w:
//  [1 returns] := w()"];
// FunctionEntry_w_0 -> Block8_0;
// Block8_0 [label="\
// Block 0; (0, max 0)\nLiveIn: \l\
// LiveOut: \l\nUsed: \l\n"];
// Block8_0Exit [label="FunctionReturn[0x11]"];
// Block8_0 -> Block8_0Exit;
// }
