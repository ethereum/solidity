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
// node[shape=box];
//
// Entry [label="Entry"];
// Entry -> Block0;
// Block0 [label="\
// Block 0\nv1 := calldataload(3)\l\
// v3 := sload(0)\l\
// v4 := eq(0, v3)\l\
// "];
// Block0 -> Block0Exit;
// Block0Exit [label="{ If v4| { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block0Exit:0 -> Block3;
// Block0Exit:1 -> Block2;
// Block2 [label="\
// Block 2\nv6 := calldataload(77)\l\
// "];
// Block2 -> Block2Exit [arrowhead=none];
// Block2Exit [label="Jump" shape=oval];
// Block2Exit -> Block1;
// Block3 [label="\
// Block 3\nv7 := eq(1, v3)\l\
// "];
// Block3 -> Block3Exit;
// Block3Exit [label="{ If v7| { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block3Exit:0 -> Block5;
// Block3Exit:1 -> Block4;
// Block1 [label="\
// Block 1\nv13 := φ(\l\
// 	Block 2 => v6,\l\
// 	Block 4 => v10,\l\
// 	Block 5 => v12\l\
// )\l\
// sstore(0, v13)\l\
// "];
// Block1Exit [label="MainExit"];
// Block1 -> Block1Exit;
// Block4 [label="\
// Block 4\nv10 := calldataload(88)\l\
// "];
// Block4 -> Block4Exit [arrowhead=none];
// Block4Exit [label="Jump" shape=oval];
// Block4Exit -> Block1;
// Block5 [label="\
// Block 5\nv12 := calldataload(99)\l\
// "];
// Block5 -> Block5Exit [arrowhead=none];
// Block5Exit [label="Jump" shape=oval];
// Block5Exit -> Block1;
// }
