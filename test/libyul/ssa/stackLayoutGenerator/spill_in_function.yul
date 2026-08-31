{
    function f() {
        // Many simultaneously-live locals inside a function with a return label
        // also pinning a stack slot — the layout generator must spill.
        let x01 := calldataload(0x01)
        let x02 := calldataload(0x02)
        let x03 := calldataload(0x03)
        let x04 := calldataload(0x04)
        let x05 := calldataload(0x05)
        let x06 := calldataload(0x06)
        let x07 := calldataload(0x07)
        let x08 := calldataload(0x08)
        let x09 := calldataload(0x09)
        let x10 := calldataload(0x0a)
        let x11 := calldataload(0x0b)
        let x12 := calldataload(0x0c)
        let x13 := calldataload(0x0d)
        let x14 := calldataload(0x0e)
        let x15 := calldataload(0x0f)
        let x16 := calldataload(0x10)
        let x17 := calldataload(0x11)
        let x18 := calldataload(0x12)
        sstore(x01, x18)
        sstore(x02, x17)
        sstore(x03, x16)
        sstore(x04, x15)
        sstore(x05, x14)
        sstore(x06, x13)
        sstore(x07, x12)
        sstore(x08, x11)
        sstore(x09, x10)
    }
    f()
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
// [FunctionCallReturnLabel[0]]\l\
// f\l\
// []\l\
// \l\
// OUT: []\l\
// "];
// Block0_0Exit [label="MainExit"];
// Block0_0 -> Block0_0Exit;
// FunctionEntry_f_0 [label="function f:
//  f()
// spilled: {v35}"];
// FunctionEntry_f_0 -> Block1_0;
// Block1_0 [label="\
// IN: [ReturnLabel[1]]\l\
// \l\
// [ReturnLabel[1], lit0]\l\
// calldataload\l\
// [ReturnLabel[1], v1]\l\
// \l\
// [ReturnLabel[1], v1, lit2]\l\
// calldataload\l\
// [ReturnLabel[1], v1, v3]\l\
// \l\
// [ReturnLabel[1], v1, v3, lit4]\l\
// calldataload\l\
// [ReturnLabel[1], v1, v3, v5]\l\
// \l\
// [ReturnLabel[1], v1, v3, v5, lit6]\l\
// calldataload\l\
// [ReturnLabel[1], v1, v3, v5, v7]\l\
// \l\
// [ReturnLabel[1], v1, v3, v5, v7, lit8]\l\
// calldataload\l\
// [ReturnLabel[1], v1, v3, v5, v7, v9]\l\
// \l\
// [ReturnLabel[1], v1, v3, v5, v7, v9, lit10]\l\
// calldataload\l\
// [ReturnLabel[1], v1, v3, v5, v7, v9, v11]\l\
// \l\
// [ReturnLabel[1], v1, v3, v5, v7, v9, v11, lit12]\l\
// calldataload\l\
// [ReturnLabel[1], v1, v3, v5, v7, v9, v11, v13]\l\
// \l\
// [ReturnLabel[1], v1, v3, v5, v7, v9, v11, v13, lit14]\l\
// calldataload\l\
// [ReturnLabel[1], v1, v3, v5, v7, v9, v11, v13, v15]\l\
// \l\
// [ReturnLabel[1], v1, v3, v5, v7, v9, v11, v13, v15, lit16]\l\
// calldataload\l\
// [ReturnLabel[1], v1, v3, v5, v7, v9, v11, v13, v15, v17]\l\
// \l\
// [ReturnLabel[1], v1, v3, v5, v7, v9, v11, v13, v15, v17, lit18]\l\
// calldataload\l\
// [ReturnLabel[1], v1, v3, v5, v7, v9, v11, v13, v15, v17, v19]\l\
// \l\
// [ReturnLabel[1], v1, v3, v5, v7, v9, v11, v13, v15, v17, v19, lit20]\l\
// calldataload\l\
// [ReturnLabel[1], v1, v3, v5, v7, v9, v11, v13, v15, v17, v19, v21]\l\
// \l\
// [ReturnLabel[1], v1, v3, v5, v7, v9, v11, v13, v15, v17, v19, v21, lit22]\l\
// calldataload\l\
// [ReturnLabel[1], v1, v3, v5, v7, v9, v11, v13, v15, v17, v19, v21, v23]\l\
// \l\
// [ReturnLabel[1], v1, v3, v5, v7, v9, v11, v13, v15, v17, v19, v21, v23, lit24]\l\
// calldataload\l\
// [ReturnLabel[1], v1, v3, v5, v7, v9, v11, v13, v15, v17, v19, v21, v23, v25]\l\
// \l\
// [ReturnLabel[1], v1, v3, v5, v7, v9, v11, v13, v15, v17, v19, v21, v23, v25, lit26]\l\
// calldataload\l\
// [ReturnLabel[1], v1, v3, v5, v7, v9, v11, v13, v15, v17, v19, v21, v23, v25, v27]\l\
// \l\
// [ReturnLabel[1], v1, v3, v5, v7, v9, v11, v13, v15, v17, v19, v21, v23, v25, v27, lit28]\l\
// calldataload\l\
// [ReturnLabel[1], v1, v3, v5, v7, v9, v11, v13, v15, v17, v19, v21, v23, v25, v27, v29]\l\
// \l\
// [ReturnLabel[1], v1, v3, v5, v7, v9, v11, v13, v15, v17, v19, v21, v23, v25, v27, v29, lit30]\l\
// calldataload\l\
// [ReturnLabel[1], v1, v3, v5, v7, v9, v11, v13, v15, v17, v19, v21, v23, v25, v27, v29, v31]\l\
// \l\
// [ReturnLabel[1], v1, v3, v5, v7, v9, v11, v13, v15, v17, v19, v21, v23, v25, v27, v29, v31, lit32]\l\
// calldataload\l\
// [ReturnLabel[1], v1, v3, v5, v7, v9, v11, v13, v15, v17, v19, v21, v23, v25, v27, v29, v31, v33]\l\
// \l\
// [ReturnLabel[1], v1, v3, v5, v7, v9, v11, v13, v15, v17, v19, v21, v23, v25, v27, v29, v31, v33, lit34]\l\
// calldataload\l\
// [ReturnLabel[1], v1, v3, v5, v7, v9, v11, v13, v15, v17, v19, v21, v23, v25, v27, v29, v31, v33, v35]\l\
// \l\
// [ReturnLabel[1], v33, v3, v5, v7, v9, v11, v13, v15, v17, v19, v21, v23, v25, v27, v29, v31, v35, v1]\l\
// sstore\l\
// [ReturnLabel[1], v33, v3, v5, v7, v9, v11, v13, v15, v17, v19, v21, v23, v25, v27, v29, v31]\l\
// \l\
// [ReturnLabel[1], v31, v29, v5, v7, v9, v11, v13, v15, v17, v19, v21, v23, v25, v27, v33, v3]\l\
// sstore\l\
// [ReturnLabel[1], v31, v29, v5, v7, v9, v11, v13, v15, v17, v19, v21, v23, v25, v27]\l\
// \l\
// [ReturnLabel[1], v27, v29, v25, v7, v9, v11, v13, v15, v17, v19, v21, v23, v31, v5]\l\
// sstore\l\
// [ReturnLabel[1], v27, v29, v25, v7, v9, v11, v13, v15, v17, v19, v21, v23]\l\
// \l\
// [ReturnLabel[1], v27, v23, v25, v21, v9, v11, v13, v15, v17, v19, v29, v7]\l\
// sstore\l\
// [ReturnLabel[1], v27, v23, v25, v21, v9, v11, v13, v15, v17, v19]\l\
// \l\
// [ReturnLabel[1], v19, v23, v25, v21, v17, v11, v13, v15, v27, v9]\l\
// sstore\l\
// [ReturnLabel[1], v19, v23, v25, v21, v17, v11, v13, v15]\l\
// \l\
// [ReturnLabel[1], v19, v23, v15, v21, v17, v13, v25, v11]\l\
// sstore\l\
// [ReturnLabel[1], v19, v23, v15, v21, v17, v13]\l\
// \l\
// [ReturnLabel[1], v19, v17, v15, v21, v23, v13]\l\
// sstore\l\
// [ReturnLabel[1], v19, v17, v15, v21]\l\
// \l\
// [ReturnLabel[1], v19, v17, v21, v15]\l\
// sstore\l\
// [ReturnLabel[1], v19, v17]\l\
// \l\
// [ReturnLabel[1], v19, v17]\l\
// sstore\l\
// [ReturnLabel[1]]\l\
// \l\
// OUT: [ReturnLabel[1]]\l\
// "];
// Block1_0Exit [label="FunctionReturn[]"];
// Block1_0 -> Block1_0Exit;
// }
