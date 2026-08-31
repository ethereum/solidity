// Spilling at a switch-merge block.
//
// The two switch arms each keep all 18 `var_x*` live across the join, so the merge block's stack-in
// is ~19 deep and reconciling either arm to it needs to reach below SWAP16.
{
    mstore(64, memoryguard(128))
    let var_x1 := calldataload(4)
    let var_x2 := sub(1, var_x1)
    let var_x3 := mul(0x03, var_x1)
    let var_x4 := sub(0x02, var_x1)
    let var_x5 := mul(0x05, var_x1)
    let var_x6 := mul(0x06, var_x1)
    let var_x7 := mul(0x07, var_x1)
    let var_x8 := sub(0x03, var_x1)
    let var_x9 := mul(0x09, var_x1)
    let var_x10 := mul(0x0a, var_x1)
    let var_x11 := mul(0x0b, var_x1)
    let var_x12 := mul(0x0c, var_x1)
    let var_x13 := mul(0x0d, var_x1)
    let var_x14 := mul(0x0e, var_x1)
    let var_x15 := mul(0x0f, var_x1)
    let var_x16 := sub(4, var_x1)
    let var_x17 := mul(0x11, var_x1)
    let var_x18 := mul(0x12, var_x1)
    switch gt(var_x18, 0x05f5e100)
    case 0 {
        var_x2 := add(var_x2, 0x02)
        var_x3 := add(var_x3, 0x02)
        var_x4 := add(var_x4, 0x02)
        var_x5 := add(var_x5, 0x02)
        var_x6 := add(var_x6, 0x02)
        var_x7 := add(var_x7, 0x02)
        var_x8 := add(var_x8, 0x02)
        var_x9 := add(var_x9, 0x02)
        var_x10 := add(var_x10, 0x02)
        var_x11 := add(var_x11, 0x02)
        var_x12 := add(var_x12, 0x02)
        var_x13 := add(var_x13, 0x02)
        var_x14 := add(var_x14, 0x02)
        var_x15 := add(var_x15, 0x02)
        var_x16 := add(var_x16, 0x02)
        var_x17 := add(var_x17, 0x02)
        var_x18 := add(var_x18, 0x02)
        var_x1 := add(var_x1, 0x02)
    }
    default {
        var_x1 := add(var_x1, 1)
        var_x2 := add(var_x2, 1)
        var_x3 := add(var_x3, 1)
        var_x4 := add(var_x4, 1)
        var_x5 := add(var_x5, 1)
        var_x6 := add(var_x6, 1)
        var_x7 := add(var_x7, 1)
        var_x8 := add(var_x8, 1)
        var_x9 := add(var_x9, 1)
        var_x10 := add(var_x10, 1)
        var_x11 := add(var_x11, 1)
        var_x12 := add(var_x12, 1)
        var_x13 := add(var_x13, 1)
        var_x14 := add(var_x14, 1)
        var_x15 := add(var_x15, 1)
        var_x16 := add(var_x16, 1)
        var_x17 := add(var_x17, 1)
        var_x18 := add(var_x18, 1)
    }
    mstore(memoryguard(128), add(add(add(add(add(add(add(add(add(add(add(add(add(add(add(add(add(var_x1, var_x2), var_x3), var_x4), var_x5), var_x6), var_x7), var_x8), var_x9), var_x10), var_x11), var_x12), var_x13), var_x14), var_x15), var_x16), var_x17), var_x18))
    return(memoryguard(128), 32)
}
// ----
// object "object"
// ===== SSA CFG =====
// memoryguard = 0xe0
//
// #0:
//     v0 = memoryguard
//     v1 = const 0x40
//     builtin @mstore v1, v0
//     v3 = const 0x04
//     v4 = builtin @calldataload v3
//     v5 = const 0x01
//     v6 = builtin @sub v5, v4
//     v7 = const 0x03
//     v8 = builtin @mul v7, v4
//     v9 = const 0x02
//     v10 = builtin @sub v9, v4
//     v11 = const 0x05
//     v12 = builtin @mul v11, v4
//     v13 = const 0x06
//     v14 = builtin @mul v13, v4
//     v15 = const 0x07
//     v16 = builtin @mul v15, v4
//     v17 = builtin @sub v7, v4
//     v18 = const 0x09
//     v19 = builtin @mul v18, v4
//     v20 = const 0x0a
//     v21 = builtin @mul v20, v4
//     v22 = const 0x0b
//     v23 = builtin @mul v22, v4
//     v24 = const 0x0c
//     v25 = builtin @mul v24, v4
//     v26 = const 0x0d
//     v27 = builtin @mul v26, v4
//     v28 = const 0x0e
//     v29 = builtin @mul v28, v4
//     v30 = const 0x0f
//     v31 = builtin @mul v30, v4
//     v32 = builtin @sub v3, v4
//     v33 = const 0x11
//     v34 = builtin @mul v33, v4
//     v35 = const 0x12
//     v36 = builtin @mul v35, v4
//     v37 = const 0x05f5e100
//     v38 = builtin @gt v36, v37
//     v39 = const 0x00
//     v40 = builtin @eq v38, v39
//     v150 = const 0x20
//     branch v40, #2, #3
// #1: preds: #2, #3
//     v77 = phi
//     v80 = phi
//     v83 = phi
//     v86 = phi
//     v89 = phi
//     v92 = phi
//     v95 = phi
//     v98 = phi
//     v101 = phi
//     v104 = phi
//     v107 = phi
//     v110 = phi
//     v113 = phi
//     v116 = phi
//     v119 = phi
//     v122 = phi
//     v125 = phi
//     v128 = phi
//     v131 = builtin @add v128, v125
//     v132 = builtin @add v131, v122
//     v133 = builtin @add v132, v119
//     v134 = builtin @add v133, v116
//     v135 = builtin @add v134, v113
//     v136 = builtin @add v135, v110
//     v137 = builtin @add v136, v107
//     v138 = builtin @add v137, v104
//     v139 = builtin @add v138, v101
//     v140 = builtin @add v139, v98
//     v141 = builtin @add v140, v95
//     v142 = builtin @add v141, v92
//     v143 = builtin @add v142, v89
//     v144 = builtin @add v143, v86
//     v145 = builtin @add v144, v83
//     v146 = builtin @add v145, v80
//     v147 = builtin @add v146, v77
//     v148 = memoryguard
//     builtin @mstore v148, v147
//     v151 = memoryguard
//     builtin @return v151, v150
//     terminated
// #2: preds: #0
//     v41 = builtin @add v6, v9
//     v42 = builtin @add v8, v9
//     v43 = builtin @add v10, v9
//     v44 = builtin @add v12, v9
//     v45 = builtin @add v14, v9
//     v46 = builtin @add v16, v9
//     v47 = builtin @add v17, v9
//     v48 = builtin @add v19, v9
//     v49 = builtin @add v21, v9
//     v50 = builtin @add v23, v9
//     v51 = builtin @add v25, v9
//     v52 = builtin @add v27, v9
//     v53 = builtin @add v29, v9
//     v54 = builtin @add v31, v9
//     v55 = builtin @add v32, v9
//     v56 = builtin @add v34, v9
//     v57 = builtin @add v36, v9
//     v58 = builtin @add v4, v9
//     upsilon v57 -> ^v77
//     upsilon v56 -> ^v80
//     upsilon v55 -> ^v83
//     upsilon v54 -> ^v86
//     upsilon v53 -> ^v89
//     upsilon v52 -> ^v92
//     upsilon v51 -> ^v95
//     upsilon v50 -> ^v98
//     upsilon v49 -> ^v101
//     upsilon v48 -> ^v104
//     upsilon v47 -> ^v107
//     upsilon v46 -> ^v110
//     upsilon v45 -> ^v113
//     upsilon v44 -> ^v116
//     upsilon v43 -> ^v119
//     upsilon v42 -> ^v122
//     upsilon v41 -> ^v125
//     upsilon v58 -> ^v128
//     jump #1
// #3: preds: #0
//     v59 = builtin @add v4, v5
//     v60 = builtin @add v6, v5
//     v61 = builtin @add v8, v5
//     v62 = builtin @add v10, v5
//     v63 = builtin @add v12, v5
//     v64 = builtin @add v14, v5
//     v65 = builtin @add v16, v5
//     v66 = builtin @add v17, v5
//     v67 = builtin @add v19, v5
//     v68 = builtin @add v21, v5
//     v69 = builtin @add v23, v5
//     v70 = builtin @add v25, v5
//     v71 = builtin @add v27, v5
//     v72 = builtin @add v29, v5
//     v73 = builtin @add v31, v5
//     v74 = builtin @add v32, v5
//     v75 = builtin @add v34, v5
//     v76 = builtin @add v36, v5
//     upsilon v76 -> ^v77
//     upsilon v75 -> ^v80
//     upsilon v74 -> ^v83
//     upsilon v73 -> ^v86
//     upsilon v72 -> ^v89
//     upsilon v71 -> ^v92
//     upsilon v70 -> ^v95
//     upsilon v69 -> ^v98
//     upsilon v68 -> ^v101
//     upsilon v67 -> ^v104
//     upsilon v66 -> ^v107
//     upsilon v65 -> ^v110
//     upsilon v64 -> ^v113
//     upsilon v63 -> ^v116
//     upsilon v62 -> ^v119
//     upsilon v61 -> ^v122
//     upsilon v60 -> ^v125
//     upsilon v59 -> ^v128
//     jump #1
//
// ===== spill info =====
// CFG[0] <main>
//   spilled:
//     v34 (value) -> mem 0x80
//     v55 (value) -> mem 0xa0
//     v74 (value) -> mem 0xc0
//   mstore schedule:
//     mstore addr(v34) <- v34 (B#0)
//     mstore addr(v55) <- v55 (B#2)
//     mstore addr(v74) <- v74 (B#3)
