{
    let x := calldataload(3)

    switch sload(0)
    case 0 {
        x := calldataload(77)
    }
    case 1 {
        x := calldataload(88)
    }
    default {
        x := calldataload(99)
    }
    sstore(x, 0)
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
// Block 0\nv1 := calldataload(3)\l\
// v3 := sload(0)\l\
// v4 := eq(0, v3)\l\
// "];
// Block0_0 -> Block0_0Exit;
// Block0_0Exit [label="{ If v4| { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block0_0Exit:0 -> Block0_3;
// Block0_0Exit:1 -> Block0_2;
// Block0_2 [label="\
// Block 2\nv6 := calldataload(77)\l\
// "];
// Block0_2 -> Block0_2Exit [arrowhead=none];
// Block0_2Exit [label="Jump" shape=oval];
// Block0_2Exit -> Block0_1;
// Block0_3 [label="\
// Block 3\nv7 := eq(1, v3)\l\
// "];
// Block0_3 -> Block0_3Exit;
// Block0_3Exit [label="{ If v7| { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block0_3Exit:0 -> Block0_5;
// Block0_3Exit:1 -> Block0_4;
// Block0_1 [label="\
// Block 1\nv13 := φ(\l\
// 	Block 2 => v6,\l\
// 	Block 4 => v10,\l\
// 	Block 5 => v12\l\
// )\l\
// sstore(0, v13)\l\
// "];
// Block0_1Exit [label="MainExit"];
// Block0_1 -> Block0_1Exit;
// Block0_4 [label="\
// Block 4\nv10 := calldataload(88)\l\
// "];
// Block0_4 -> Block0_4Exit [arrowhead=none];
// Block0_4Exit [label="Jump" shape=oval];
// Block0_4Exit -> Block0_1;
// Block0_5 [label="\
// Block 5\nv12 := calldataload(99)\l\
// "];
// Block0_5 -> Block0_5Exit [arrowhead=none];
// Block0_5Exit [label="Jump" shape=oval];
// Block0_5Exit -> Block0_1;
// }
