{
    let x := calldataload(3)
    if mload(42) {
        x := calldataload(77)
    }
    let y := calldataload(x)
    sstore(y, 0)
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
// LiveOut: v1[1]\l\nUsed: \l\nv1 := calldataload(0x03)\l\
// v3 := mload(0x2a)\l\
// "];
// Block0_0 -> Block0_0Exit;
// Block0_0Exit [label="{ If v3 | { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block0_0Exit:0 -> Block0_3 [style="solid"];
// Block0_0Exit:1 -> Block0_1 [style="solid"];
// Block0_1 [label="\
// Block 1; (1, max 2)\nLiveIn: \l\
// LiveOut: v5[1]\l\nUsed: \l\nv5 := calldataload(0x4d)\l\
// "];
// Block0_1 -> Block0_1Exit [arrowhead=none];
// Block0_1Exit [label="Jump" shape=oval];
// Block0_1Exit -> Block0_2 [style="solid"];
// Block0_3 [label="\
// Block 3; (3, max 3)\nLiveIn: v1[1]\l\
// LiveOut: v1[1]\l\nUsed: \l\n"];
// Block0_3 -> Block0_3Exit [arrowhead=none];
// Block0_3Exit [label="Jump" shape=oval];
// Block0_3Exit -> Block0_2 [style="solid"];
// Block0_2 [label="\
// Block 2; (2, max 2)\nLiveIn: phi6[2]\l\
// LiveOut: \l\nUsed: phi6[2]\l\nphi6 := φ(\l\
// 	Block 1 => v5,\l\
// 	Block 3 => v1\l\
// )\l\
// v9 := calldataload(phi6)\l\
// sstore(v9, 0x00)\l\
// "];
// Block0_2Exit [label="MainExit"];
// Block0_2 -> Block0_2Exit;
// }
