{
    let x
    if mload(42) {
        x := 5
    }
    sstore(x, x)
}
// ----
// digraph SSACFG {
// nodesep=0.7;
// graph[fontname="DejaVu Sans"]
// node[shape=box,fontname="DejaVu Sans"];
//
// Entry [label="Entry"];
// Entry -> Block0_0;
// Block0_0 [label="\
// Block 0; (0, max 3)\nLiveIn: \l\
// LiveOut: \l\nUsed: \l\nv2 := mload(0x2a)\l\
// "];
// Block0_0 -> Block0_0Exit;
// Block0_0Exit [label="{ If v2 | { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block0_0Exit:0 -> Block0_3 [style="solid"];
// Block0_0Exit:1 -> Block0_1 [style="solid"];
// Block0_1 [label="\
// Block 1; (1, max 2)\nLiveIn: \l\
// LiveOut: \l\nUsed: \l\n"];
// Block0_1 -> Block0_1Exit [arrowhead=none];
// Block0_1Exit [label="Jump" shape=oval];
// Block0_1Exit -> Block0_2 [style="solid"];
// Block0_3 [label="\
// Block 3; (3, max 3)\nLiveIn: \l\
// LiveOut: \l\nUsed: \l\n"];
// Block0_3 -> Block0_3Exit [arrowhead=none];
// Block0_3Exit [label="Jump" shape=oval];
// Block0_3Exit -> Block0_2 [style="solid"];
// Block0_2 [label="\
// Block 2; (2, max 2)\nLiveIn: phi4[3]\l\
// LiveOut: \l\nUsed: phi4[3]\l\nphi4 := φ(\l\
// 	Block 1 => 0x05,\l\
// 	Block 3 => 0x00\l\
// )\l\
// sstore(phi4, phi4)\l\
// "];
// Block0_2Exit [label="MainExit"];
// Block0_2 -> Block0_2Exit;
// }
