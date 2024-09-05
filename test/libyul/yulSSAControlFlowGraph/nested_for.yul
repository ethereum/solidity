{
    for { let i := 0 } lt(i, 3) { i := add(i, 1) } {
        for { let j := 0 } lt(j, 3) { j := add(j, 1) } {
            for { let k := 0 } lt(k, 3) { k := add(k, 1) } {
                if 0 {
                    for { let l := 0 } lt(l, 3) { l := add(l, 1) } {
                        sstore(l, add(add(i,j),k))
                    }
                }
                if 1 {
                    for { let l := 0 } lt(l, 3) { l := add(l, 1) } {
                        sstore(l, add(add(i,j),k))
                    }
                }
            }
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
// Block 0; (0, max 12)\nLiveIn: v0,v8,v9,v13,v15,v16\l\
// LiveOut: v0,v8,v9,v13,v15,v16\l\n"];
// Block0_0 -> Block0_0Exit [arrowhead=none];
// Block0_0Exit [label="Jump" shape=oval];
// Block0_0Exit -> Block0_1 [style="solid"];
// Block0_1 [label="\
// Block 1; (1, max 12)\nLiveIn: v0,v2,v8,v9,v13,v15\l\
// LiveOut: v0,v2,v8,v9,v13,v15\l\nv2 := φ(\l\
// 	Block 0 => 0,\l\
// 	Block 3 => v16\l\
// )\l\
// v3 := lt(3, v2)\l\
// "];
// Block0_1 -> Block0_1Exit;
// Block0_1Exit [label="{ If v3 | { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block0_1Exit:0 -> Block0_4 [style="solid"];
// Block0_1Exit:1 -> Block0_2 [style="solid"];
// Block0_2 [label="\
// Block 2; (2, max 11)\nLiveIn: v0,v2,v8,v9,v13,v15\l\
// LiveOut: v0,v2,v8,v9,v13,v15\l\n"];
// Block0_2 -> Block0_2Exit [arrowhead=none];
// Block0_2Exit [label="Jump" shape=oval];
// Block0_2Exit -> Block0_5 [style="solid"];
// Block0_4 [label="\
// Block 4; (12, max 12)\nLiveIn: \l\
// LiveOut: \l\n"];
// Block0_4Exit [label="MainExit"];
// Block0_4 -> Block0_4Exit;
// Block0_5 [label="\
// Block 5; (3, max 11)\nLiveIn: v0,v2,v4,v8,v9,v13,v14\l\
// LiveOut: v0,v2,v4,v8,v9,v13,v14\l\nv4 := φ(\l\
// 	Block 2 => 0,\l\
// 	Block 7 => v15\l\
// )\l\
// v14 := φ(\l\
// 	Block 2 => v2,\l\
// 	Block 7 => v9\l\
// )\l\
// v5 := lt(3, v4)\l\
// "];
// Block0_5 -> Block0_5Exit;
// Block0_5Exit [label="{ If v5 | { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block0_5Exit:0 -> Block0_8 [style="solid"];
// Block0_5Exit:1 -> Block0_6 [style="solid"];
// Block0_6 [label="\
// Block 6; (4, max 9)\nLiveIn: v0,v2,v4,v8,v9,v13,v14\l\
// LiveOut: v0,v2,v4,v8,v9,v13,v14\l\n"];
// Block0_6 -> Block0_6Exit [arrowhead=none];
// Block0_6Exit [label="Jump" shape=oval];
// Block0_6Exit -> Block0_9 [style="solid"];
// Block0_8 [label="\
// Block 8; (10, max 11)\nLiveIn: v0,v14\l\
// LiveOut: v0,v14\l\n"];
// Block0_8 -> Block0_8Exit [arrowhead=none];
// Block0_8Exit [label="Jump" shape=oval];
// Block0_8Exit -> Block0_3 [style="solid"];
// Block0_9 [label="\
// Block 9; (5, max 9)\nLiveIn: v0,v2,v4,v6,v8,v9,v14\l\
// LiveOut: v0,v2,v4,v6,v8,v9,v14\l\nv6 := φ(\l\
// 	Block 6 => 0,\l\
// 	Block 11 => v13\l\
// )\l\
// v7 := lt(3, v6)\l\
// "];
// Block0_9 -> Block0_9Exit;
// Block0_9Exit [label="{ If v7 | { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block0_9Exit:0 -> Block0_12 [style="solid"];
// Block0_9Exit:1 -> Block0_10 [style="solid"];
// Block0_3 [label="\
// Block 3; (11, max 11)\nLiveIn: v0,v14\l\
// LiveOut: v0,v16\l\nv16 := add(1, v14)\l\
// "];
// Block0_3 -> Block0_3Exit [arrowhead=none];
// Block0_3Exit [label="Jump" shape=oval];
// Block0_3Exit -> Block0_1 [style="dashed"];
// Block0_10 [label="\
// Block 10; (6, max 7)\nLiveIn: v0,v4,v6,v14\l\
// LiveOut: v0,v6\l\nv10 := add(v4, v14)\l\
// v11 := add(v6, v10)\l\
// sstore(v11, 0)\l\
// "];
// Block0_10 -> Block0_10Exit [arrowhead=none];
// Block0_10Exit [label="Jump" shape=oval];
// Block0_10Exit -> Block0_11 [style="solid"];
// Block0_12 [label="\
// Block 12; (8, max 9)\nLiveIn: v0,v2,v8,v9\l\
// LiveOut: v0,v2,v8,v9\l\n"];
// Block0_12 -> Block0_12Exit [arrowhead=none];
// Block0_12Exit [label="Jump" shape=oval];
// Block0_12Exit -> Block0_7 [style="solid"];
// Block0_11 [label="\
// Block 11; (7, max 7)\nLiveIn: v0,v6\l\
// LiveOut: v0,v13\l\nv13 := add(1, v6)\l\
// "];
// Block0_11 -> Block0_11Exit [arrowhead=none];
// Block0_11Exit [label="Jump" shape=oval];
// Block0_11Exit -> Block0_9 [style="dashed"];
// Block0_7 [label="\
// Block 7; (9, max 9)\nLiveIn: v0,v2,v8,v9\l\
// LiveOut: v0,v2,v9,v15\l\nv15 := add(1, v8)\l\
// "];
// Block0_7 -> Block0_7Exit [arrowhead=none];
// Block0_7Exit [label="Jump" shape=oval];
// Block0_7Exit -> Block0_5 [style="dashed"];
// }
