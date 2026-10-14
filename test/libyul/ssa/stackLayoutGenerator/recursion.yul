{
    function sum(n) -> s {
        if n {
            s := add(n, sum(sub(n, 1)))
        }
    }
    mstore(0, sum(10))
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
// [FunctionCallReturnLabel[0], lit0]\l\
// sum\l\
// [v1]\l\
// \l\
// [v1, lit2]\l\
// mstore\l\
// []\l\
// \l\
// OUT: []\l\
// "];
// Block0_0Exit [label="MainExit"];
// Block0_0 -> Block0_0Exit;
// FunctionEntry_sum_0 [label="function sum:
//  [1 returns] := sum(v0)
// spilled: {}"];
// FunctionEntry_sum_0 -> Block1_0;
// Block1_0 [label="\
// IN: [ReturnLabel[1], v0]\l\
// \l\
// OUT: [ReturnLabel[1], v0, v0]\l\
// "];
// Block1_0 -> Block1_0Exit;
// Block1_0Exit [label="{ If v0 | { <0> Zero | <1> NonZero }}" shape=Mrecord];
// Block1_0Exit:0 -> Block1_3 [style="solid"];
// Block1_0Exit:1 -> Block1_1 [style="solid"];
// Block1_1 [label="\
// IN: [ReturnLabel[1], v0]\l\
// \l\
// [ReturnLabel[1], v0, lit2, v0]\l\
// sub\l\
// [ReturnLabel[1], v0, v3]\l\
// \l\
// [ReturnLabel[1], v0, FunctionCallReturnLabel[0], v3]\l\
// sum\l\
// [ReturnLabel[1], v0, v4]\l\
// \l\
// [ReturnLabel[1], v4, v0]\l\
// add\l\
// [ReturnLabel[1], v5]\l\
// \l\
// OUT: [ReturnLabel[1], v5]\l\
// "];
// Block1_1 -> Block1_1Exit [arrowhead=none];
// Block1_1Exit [label="Jump" shape=oval];
// Block1_1Exit -> Block1_2 [style="solid"];
// Block1_3 [label="\
// IN: [ReturnLabel[1], JUNK]\l\
// \l\
// OUT: [ReturnLabel[1], JUNK]\l\
// "];
// Block1_3 -> Block1_3Exit [arrowhead=none];
// Block1_3Exit [label="Jump" shape=oval];
// Block1_3Exit -> Block1_2 [style="solid"];
// Block1_2 [label="\
// IN: [ReturnLabel[1], phi6]\l\
// \l\
// OUT: [phi6, ReturnLabel[1]]\l\
// "];
// Block1_2Exit [label="FunctionReturn[phi6]"];
// Block1_2 -> Block1_2Exit;
// }
