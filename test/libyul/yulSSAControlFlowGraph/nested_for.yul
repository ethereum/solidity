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
// Block 0; (0, max 24)\nLiveIn: \l\
// LiveOut: \l\n"];
// Block0_0 -> Block0_0Exit [arrowhead=none];
// Block0_0Exit [label="Jump" shape=oval];
// Block0_0Exit -> Block0_1 [style="solid"];
// Block0_1 [label="\
// Block 1; (1, max 24)\nLiveIn: v2\l\
// LiveOut: v2\l\nv2 := φ(\l\
// 	Block 0 => 0,\l\
// 	Block 3 => v36\l\
// )\l\
// v3 := lt(3, v2)\l\
// "];
// Block0_1 -> Block0_1Exit;
// Block0_1Exit [label="{ If v3 | { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block0_1Exit:0 -> Block0_4 [style="solid"];
// Block0_1Exit:1 -> Block0_2 [style="solid"];
// Block0_2 [label="\
// Block 2; (2, max 23)\nLiveIn: v2\l\
// LiveOut: v2\l\n"];
// Block0_2 -> Block0_2Exit [arrowhead=none];
// Block0_2Exit [label="Jump" shape=oval];
// Block0_2Exit -> Block0_5 [style="solid"];
// Block0_4 [label="\
// Block 4; (24, max 24)\nLiveIn: \l\
// LiveOut: \l\n"];
// Block0_4Exit [label="MainExit"];
// Block0_4 -> Block0_4Exit;
// Block0_5 [label="\
// Block 5; (3, max 23)\nLiveIn: v2,v4\l\
// LiveOut: v2,v4\l\nv4 := φ(\l\
// 	Block 2 => 0,\l\
// 	Block 7 => v35\l\
// )\l\
// v5 := lt(3, v4)\l\
// "];
// Block0_5 -> Block0_5Exit;
// Block0_5Exit [label="{ If v5 | { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block0_5Exit:0 -> Block0_8 [style="solid"];
// Block0_5Exit:1 -> Block0_6 [style="solid"];
// Block0_6 [label="\
// Block 6; (4, max 21)\nLiveIn: v2,v4\l\
// LiveOut: v2,v4\l\n"];
// Block0_6 -> Block0_6Exit [arrowhead=none];
// Block0_6Exit [label="Jump" shape=oval];
// Block0_6Exit -> Block0_9 [style="solid"];
// Block0_8 [label="\
// Block 8; (22, max 23)\nLiveIn: v2\l\
// LiveOut: v2\l\n"];
// Block0_8 -> Block0_8Exit [arrowhead=none];
// Block0_8Exit [label="Jump" shape=oval];
// Block0_8Exit -> Block0_3 [style="solid"];
// Block0_9 [label="\
// Block 9; (5, max 21)\nLiveIn: v2,v4,v6\l\
// LiveOut: v2,v4,v6\l\nv6 := φ(\l\
// 	Block 6 => 0,\l\
// 	Block 11 => v31\l\
// )\l\
// v7 := lt(3, v6)\l\
// "];
// Block0_9 -> Block0_9Exit;
// Block0_9Exit [label="{ If v7 | { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block0_9Exit:0 -> Block0_12 [style="solid"];
// Block0_9Exit:1 -> Block0_10 [style="solid"];
// Block0_3 [label="\
// Block 3; (23, max 23)\nLiveIn: v2\l\
// LiveOut: v36\l\nv36 := add(1, v2)\l\
// "];
// Block0_3 -> Block0_3Exit [arrowhead=none];
// Block0_3Exit [label="Jump" shape=oval];
// Block0_3Exit -> Block0_1 [style="dashed"];
// Block0_10 [label="\
// Block 10; (6, max 19)\nLiveIn: v2,v4,v6\l\
// LiveOut: v2,v4,v6\l\n"];
// Block0_10 -> Block0_10Exit;
// Block0_10Exit [label="{ If 0 | { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block0_10Exit:0 -> Block0_14 [style="solid"];
// Block0_10Exit:1 -> Block0_13 [style="solid"];
// Block0_12 [label="\
// Block 12; (20, max 21)\nLiveIn: v2,v4\l\
// LiveOut: v2,v4\l\n"];
// Block0_12 -> Block0_12Exit [arrowhead=none];
// Block0_12Exit [label="Jump" shape=oval];
// Block0_12Exit -> Block0_7 [style="solid"];
// Block0_13 [label="\
// Block 13; (7, max 19)\nLiveIn: v2,v4,v6\l\
// LiveOut: v2,v4,v6\l\n"];
// Block0_13 -> Block0_13Exit [arrowhead=none];
// Block0_13Exit [label="Jump" shape=oval];
// Block0_13Exit -> Block0_15 [style="solid"];
// Block0_14 [label="\
// Block 14; (12, max 19)\nLiveIn: v2,v4,v6\l\
// LiveOut: v2,v4,v6\l\n"];
// Block0_14 -> Block0_14Exit;
// Block0_14Exit [label="{ If 1 | { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block0_14Exit:0 -> Block0_20 [style="solid"];
// Block0_14Exit:1 -> Block0_19 [style="solid"];
// Block0_7 [label="\
// Block 7; (21, max 21)\nLiveIn: v2,v4\l\
// LiveOut: v2,v35\l\nv35 := add(1, v4)\l\
// "];
// Block0_7 -> Block0_7Exit [arrowhead=none];
// Block0_7Exit [label="Jump" shape=oval];
// Block0_7Exit -> Block0_5 [style="dashed"];
// Block0_15 [label="\
// Block 15; (8, max 19)\nLiveIn: v2,v4,v6,v8\l\
// LiveOut: v2,v4,v6,v8\l\nv8 := φ(\l\
// 	Block 13 => 0,\l\
// 	Block 17 => v16\l\
// )\l\
// v9 := lt(3, v8)\l\
// "];
// Block0_15 -> Block0_15Exit;
// Block0_15Exit [label="{ If v9 | { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block0_15Exit:0 -> Block0_18 [style="solid"];
// Block0_15Exit:1 -> Block0_16 [style="solid"];
// Block0_19 [label="\
// Block 19; (13, max 19)\nLiveIn: v2,v4,v6\l\
// LiveOut: v2,v4,v6\l\n"];
// Block0_19 -> Block0_19Exit [arrowhead=none];
// Block0_19Exit [label="Jump" shape=oval];
// Block0_19Exit -> Block0_21 [style="solid"];
// Block0_20 [label="\
// Block 20; (18, max 19)\nLiveIn: v2,v4,v6\l\
// LiveOut: v2,v4,v6\l\n"];
// Block0_20 -> Block0_20Exit [arrowhead=none];
// Block0_20Exit [label="Jump" shape=oval];
// Block0_20Exit -> Block0_11 [style="solid"];
// Block0_16 [label="\
// Block 16; (9, max 10)\nLiveIn: v2,v4,v6,v8\l\
// LiveOut: v2,v4,v6,v8\l\nv13 := add(v4, v2)\l\
// v14 := add(v6, v13)\l\
// sstore(v14, v8)\l\
// "];
// Block0_16 -> Block0_16Exit [arrowhead=none];
// Block0_16Exit [label="Jump" shape=oval];
// Block0_16Exit -> Block0_17 [style="solid"];
// Block0_18 [label="\
// Block 18; (11, max 19)\nLiveIn: v2,v4,v6\l\
// LiveOut: v2,v4,v6\l\n"];
// Block0_18 -> Block0_18Exit [arrowhead=none];
// Block0_18Exit [label="Jump" shape=oval];
// Block0_18Exit -> Block0_14 [style="solid"];
// Block0_21 [label="\
// Block 21; (14, max 19)\nLiveIn: v2,v4,v6,v19\l\
// LiveOut: v2,v4,v6,v19\l\nv19 := φ(\l\
// 	Block 19 => 0,\l\
// 	Block 23 => v26\l\
// )\l\
// v20 := lt(3, v19)\l\
// "];
// Block0_21 -> Block0_21Exit;
// Block0_21Exit [label="{ If v20 | { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block0_21Exit:0 -> Block0_24 [style="solid"];
// Block0_21Exit:1 -> Block0_22 [style="solid"];
// Block0_11 [label="\
// Block 11; (19, max 19)\nLiveIn: v2,v4,v6\l\
// LiveOut: v2,v4,v31\l\nv31 := add(1, v6)\l\
// "];
// Block0_11 -> Block0_11Exit [arrowhead=none];
// Block0_11Exit [label="Jump" shape=oval];
// Block0_11Exit -> Block0_9 [style="dashed"];
// Block0_17 [label="\
// Block 17; (10, max 10)\nLiveIn: v2,v4,v6,v8\l\
// LiveOut: v2,v4,v6,v16\l\nv16 := add(1, v8)\l\
// "];
// Block0_17 -> Block0_17Exit [arrowhead=none];
// Block0_17Exit [label="Jump" shape=oval];
// Block0_17Exit -> Block0_15 [style="dashed"];
// Block0_22 [label="\
// Block 22; (15, max 16)\nLiveIn: v2,v4,v6,v19\l\
// LiveOut: v2,v4,v6,v19\l\nv24 := add(v4, v2)\l\
// v25 := add(v6, v24)\l\
// sstore(v25, v19)\l\
// "];
// Block0_22 -> Block0_22Exit [arrowhead=none];
// Block0_22Exit [label="Jump" shape=oval];
// Block0_22Exit -> Block0_23 [style="solid"];
// Block0_24 [label="\
// Block 24; (17, max 19)\nLiveIn: v2,v4,v6\l\
// LiveOut: v2,v4,v6\l\n"];
// Block0_24 -> Block0_24Exit [arrowhead=none];
// Block0_24Exit [label="Jump" shape=oval];
// Block0_24Exit -> Block0_20 [style="solid"];
// Block0_23 [label="\
// Block 23; (16, max 16)\nLiveIn: v2,v4,v6,v19\l\
// LiveOut: v2,v4,v6,v26\l\nv26 := add(1, v19)\l\
// "];
// Block0_23 -> Block0_23Exit [arrowhead=none];
// Block0_23Exit [label="Jump" shape=oval];
// Block0_23Exit -> Block0_21 [style="dashed"];
// }
