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
// Block 0\n"];
// Block0_0 -> Block0_0Exit [arrowhead=none];
// Block0_0Exit [label="Jump" shape=oval];
// Block0_0Exit -> Block0_1;
// Block0_1 [label="\
// Block 1\nv2 := φ(\l\
// 	Block 0 => 0,\l\
// 	Block 3 => v36\l\
// )\l\
// v3 := lt(3, v2)\l\
// "];
// Block0_1 -> Block0_1Exit;
// Block0_1Exit [label="{ If v3| { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block0_1Exit:0 -> Block0_4;
// Block0_1Exit:1 -> Block0_2;
// Block0_2 [label="\
// Block 2\n"];
// Block0_2 -> Block0_2Exit [arrowhead=none];
// Block0_2Exit [label="Jump" shape=oval];
// Block0_2Exit -> Block0_5;
// Block0_4 [label="\
// Block 4\n"];
// Block0_4Exit [label="MainExit"];
// Block0_4 -> Block0_4Exit;
// Block0_5 [label="\
// Block 5\nv4 := φ(\l\
// 	Block 2 => 0,\l\
// 	Block 7 => v35\l\
// )\l\
// v33 := φ(\l\
// 	Block 2 => v2,\l\
// 	Block 7 => v18\l\
// )\l\
// v5 := lt(3, v4)\l\
// "];
// Block0_5 -> Block0_5Exit;
// Block0_5Exit [label="{ If v5| { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block0_5Exit:0 -> Block0_8;
// Block0_5Exit:1 -> Block0_6;
// Block0_6 [label="\
// Block 6\n"];
// Block0_6 -> Block0_6Exit [arrowhead=none];
// Block0_6Exit [label="Jump" shape=oval];
// Block0_6Exit -> Block0_9;
// Block0_8 [label="\
// Block 8\n"];
// Block0_8 -> Block0_8Exit [arrowhead=none];
// Block0_8Exit [label="Jump" shape=oval];
// Block0_8Exit -> Block0_3;
// Block0_9 [label="\
// Block 9\nv6 := φ(\l\
// 	Block 6 => 0,\l\
// 	Block 11 => v31\l\
// )\l\
// v17 := φ(\l\
// 	Block 6 => v4,\l\
// 	Block 11 => v32\l\
// )\l\
// v18 := φ(\l\
// 	Block 6 => v33,\l\
// 	Block 11 => v34\l\
// )\l\
// v7 := lt(3, v6)\l\
// "];
// Block0_9 -> Block0_9Exit;
// Block0_9Exit [label="{ If v7| { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block0_9Exit:0 -> Block0_12;
// Block0_9Exit:1 -> Block0_10;
// Block0_3 [label="\
// Block 3\nv36 := add(1, v33)\l\
// "];
// Block0_3 -> Block0_3Exit [arrowhead=none];
// Block0_3Exit [label="Jump" shape=oval];
// Block0_3Exit -> Block0_1;
// Block0_10 [label="\
// Block 10\n"];
// Block0_10 -> Block0_10Exit;
// Block0_10Exit [label="{ If 0| { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block0_10Exit:0 -> Block0_14;
// Block0_10Exit:1 -> Block0_13;
// Block0_12 [label="\
// Block 12\n"];
// Block0_12 -> Block0_12Exit [arrowhead=none];
// Block0_12Exit [label="Jump" shape=oval];
// Block0_12Exit -> Block0_7;
// Block0_13 [label="\
// Block 13\n"];
// Block0_13 -> Block0_13Exit [arrowhead=none];
// Block0_13Exit [label="Jump" shape=oval];
// Block0_13Exit -> Block0_15;
// Block0_14 [label="\
// Block 14\nv27 := φ(\l\
// 	Block 10 => v6,\l\
// 	Block 18 => v10\l\
// )\l\
// v28 := φ(\l\
// 	Block 10 => v17,\l\
// 	Block 18 => v11\l\
// )\l\
// v29 := φ(\l\
// 	Block 10 => v18,\l\
// 	Block 18 => v12\l\
// )\l\
// "];
// Block0_14 -> Block0_14Exit;
// Block0_14Exit [label="{ If 1| { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block0_14Exit:0 -> Block0_20;
// Block0_14Exit:1 -> Block0_19;
// Block0_7 [label="\
// Block 7\nv35 := add(1, v17)\l\
// "];
// Block0_7 -> Block0_7Exit [arrowhead=none];
// Block0_7Exit [label="Jump" shape=oval];
// Block0_7Exit -> Block0_5;
// Block0_15 [label="\
// Block 15\nv8 := φ(\l\
// 	Block 13 => 0,\l\
// 	Block 17 => v16\l\
// )\l\
// v9 := lt(3, v8)\l\
// "];
// Block0_15 -> Block0_15Exit;
// Block0_15Exit [label="{ If v9| { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block0_15Exit:0 -> Block0_18;
// Block0_15Exit:1 -> Block0_16;
// Block0_19 [label="\
// Block 19\n"];
// Block0_19 -> Block0_19Exit [arrowhead=none];
// Block0_19Exit [label="Jump" shape=oval];
// Block0_19Exit -> Block0_21;
// Block0_20 [label="\
// Block 20\nv30 := φ(\l\
// 	Block 14 => v27,\l\
// 	Block 24 => v21\l\
// )\l\
// v32 := φ(\l\
// 	Block 14 => v28,\l\
// 	Block 24 => v22\l\
// )\l\
// v34 := φ(\l\
// 	Block 14 => v29,\l\
// 	Block 24 => v23\l\
// )\l\
// "];
// Block0_20 -> Block0_20Exit [arrowhead=none];
// Block0_20Exit [label="Jump" shape=oval];
// Block0_20Exit -> Block0_11;
// Block0_16 [label="\
// Block 16\nv13 := add(v17, v18)\l\
// v14 := add(v6, v13)\l\
// sstore(v14, v8)\l\
// "];
// Block0_16 -> Block0_16Exit [arrowhead=none];
// Block0_16Exit [label="Jump" shape=oval];
// Block0_16Exit -> Block0_17;
// Block0_18 [label="\
// Block 18\n"];
// Block0_18 -> Block0_18Exit [arrowhead=none];
// Block0_18Exit [label="Jump" shape=oval];
// Block0_18Exit -> Block0_14;
// Block0_21 [label="\
// Block 21\nv19 := φ(\l\
// 	Block 19 => 0,\l\
// 	Block 23 => v26\l\
// )\l\
// v20 := lt(3, v19)\l\
// "];
// Block0_21 -> Block0_21Exit;
// Block0_21Exit [label="{ If v20| { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block0_21Exit:0 -> Block0_24;
// Block0_21Exit:1 -> Block0_22;
// Block0_11 [label="\
// Block 11\nv31 := add(1, v30)\l\
// "];
// Block0_11 -> Block0_11Exit [arrowhead=none];
// Block0_11Exit [label="Jump" shape=oval];
// Block0_11Exit -> Block0_9;
// Block0_17 [label="\
// Block 17\nv16 := add(1, v8)\l\
// "];
// Block0_17 -> Block0_17Exit [arrowhead=none];
// Block0_17Exit [label="Jump" shape=oval];
// Block0_17Exit -> Block0_15;
// Block0_22 [label="\
// Block 22\nv24 := add(v28, v29)\l\
// v25 := add(v27, v24)\l\
// sstore(v25, v19)\l\
// "];
// Block0_22 -> Block0_22Exit [arrowhead=none];
// Block0_22Exit [label="Jump" shape=oval];
// Block0_22Exit -> Block0_23;
// Block0_24 [label="\
// Block 24\n"];
// Block0_24 -> Block0_24Exit [arrowhead=none];
// Block0_24Exit [label="Jump" shape=oval];
// Block0_24Exit -> Block0_20;
// Block0_23 [label="\
// Block 23\nv26 := add(1, v19)\l\
// "];
// Block0_23 -> Block0_23Exit [arrowhead=none];
// Block0_23Exit [label="Jump" shape=oval];
// Block0_23Exit -> Block0_21;
// }
