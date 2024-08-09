{
    let x := calldataload(3)
    if 0 {
        x := calldataload(77)
    }
    let y := calldataload(x)
    sstore(y, 0)
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
// "];
// Block0 -> Block0Exit;
// Block0Exit [label="{ If 0| { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block0Exit:0 -> Block2;
// Block0Exit:1 -> Block1;
// Block1 [label="\
// Block 1\nv4 := calldataload(77)\l\
// "];
// Block1 -> Block1Exit [arrowhead=none];
// Block1Exit [label="Jump" shape=oval];
// Block1Exit -> Block2;
// Block2 [label="\
// Block 2\nv5 := φ(\l\
// 	Block 0 => v1,\l\
// 	Block 1 => v4\l\
// )\l\
// v6 := calldataload(v5)\l\
// sstore(0, v6)\l\
// "];
// Block2Exit [label="MainExit"];
// Block2 -> Block2Exit;
// }
