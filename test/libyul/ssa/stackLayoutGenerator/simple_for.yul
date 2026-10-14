{
    let x
    for {let i := 0} x {i := add(i, 1)} {
        if mload(i) {
            x := add(x, i)
            x := add(x, mload(32))
        }
    }
    mstore(x, 33)
}
// ----
// digraph SSACFG {
// nodesep=0.7;
// graph[fontname="DejaVu Sans", rankdir=LR]
// node[shape=box,fontname="DejaVu Sans"];
//
// Entry [label="Entry
// spilled: {}"];
// Entry -> Block0_0;
// Block0_0 [label="\
// IN: []\l\
// \l\
// OUT: []\l\
// "];
// Block0_0 -> Block0_0Exit [arrowhead=none];
// Block0_0Exit [label="Jump" shape=oval];
// Block0_0Exit -> Block0_1 [style="solid"];
// Block0_1 [label="\
// IN: [phi1, phi2]\l\
// \l\
// OUT: [phi1, phi2, phi1]\l\
// "];
// Block0_1 -> Block0_1Exit;
// Block0_1Exit [label="{ If phi1 | { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block0_1Exit:0 -> Block0_4 [style="solid"];
// Block0_1Exit:1 -> Block0_2 [style="solid"];
// Block0_2 [label="\
// IN: [phi1, phi2]\l\
// \l\
// [phi1, phi2, phi2]\l\
// mload\l\
// [phi1, phi2, v3]\l\
// \l\
// OUT: [phi1, phi2, v3]\l\
// "];
// Block0_2 -> Block0_2Exit;
// Block0_2Exit [label="{ If v3 | { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block0_2Exit:0 -> Block0_3 [style="solid"];
// Block0_2Exit:1 -> Block0_5 [style="solid"];
// Block0_4 [label="\
// IN: [phi1, JUNK]\l\
// \l\
// [phi1, lit20, phi1]\l\
// mstore\l\
// [phi1]\l\
// \l\
// OUT: [phi1]\l\
// "];
// Block0_4Exit [label="MainExit"];
// Block0_4 -> Block0_4Exit;
// Block0_5 [label="\
// IN: [phi1, phi2]\l\
// \l\
// [phi2, phi2, phi1]\l\
// add\l\
// [phi2, v4]\l\
// \l\
// [phi2, v4, lit5]\l\
// mload\l\
// [phi2, v4, v6]\l\
// \l\
// [phi2, v6, v4]\l\
// add\l\
// [phi2, v7]\l\
// \l\
// OUT: [phi2, v7]\l\
// "];
// Block0_5 -> Block0_5Exit [arrowhead=none];
// Block0_5Exit [label="Jump" shape=oval];
// Block0_5Exit -> Block0_6 [style="solid"];
// Block0_3 [label="\
// IN: [phi1, phi2]\l\
// \l\
// OUT: [phi1, phi2]\l\
// "];
// Block0_3 -> Block0_3Exit [arrowhead=none];
// Block0_3Exit [label="Jump" shape=oval];
// Block0_3Exit -> Block0_6 [style="solid"];
// Block0_6 [label="\
// IN: [phi2, phi14]\l\
// \l\
// [phi14, lit8, phi2]\l\
// add\l\
// [phi14, v12]\l\
// \l\
// OUT: [phi14, v12]\l\
// "];
// Block0_6 -> Block0_6Exit [arrowhead=none];
// Block0_6Exit [label="Jump" shape=oval];
// Block0_6Exit -> Block0_1 [style="solid"];
// }
