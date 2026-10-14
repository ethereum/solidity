// Back-edge stack layouts that require spilling to be shuffled successfully
object "C" {
code {
    let g := memoryguard(0x80)
    mstore(0x40, g)
    sstore(0, f(calldataload(0x0)))
    function f(bound) -> r {
        let x0 := 1
        let x1 := 2
        let x2 := 3
        let x3 := 4
        let x4 := 5
        let x5 := 6
        let x6 := 7
        let x7 := 8
        let x8 := 9
        let x9 := 10
        let x10 := 11
        let x11 := 12
        let x12 := 13
        let x13 := 14
        let x14 := 15
        let x15 := 16
        let x16 := 17
        let x17 := 18
        let x18 := 19
        for { let c := 0 } lt(c, bound) { c := add(c, 1) } {
            x0 := add(x0, c)
            x1 := add(x1, c)
            x2 := add(x2, c)
            x3 := add(x3, c)
            x4 := add(x4, c)
            x5 := add(x5, c)
            x6 := add(x6, c)
            x7 := add(x7, c)
            x8 := add(x8, c)
            x9 := add(x9, c)
            x10 := add(x10, c)
            x11 := add(x11, c)
            x12 := add(x12, c)
            x13 := add(x13, c)
            x14 := add(x14, c)
            x15 := add(x15, c)
            x16 := add(x16, c)
            x17 := add(x17, c)
            x18 := add(x18, c)
        }
        r := add(add(add(add(add(add(add(add(add(add(add(add(add(add(add(add(add(add(x0, x1), x2), x3), x4), x5), x6), x7), x8), x9), x10), x11), x12), x13), x14), x15), x16), x17), x18)
    }
}
}
// ----
// object "C"
// ===== SSA CFG =====
// memoryguard = 0x0180
//
// #0:
//     v0 = memoryguard
//     v1 = const 0x40
//     builtin @mstore v1, v0
//     v3 = const 0x00
//     v4 = builtin @calldataload v3
//     v5 = call @f v4
//     builtin @sstore v3, v5
//     main_exit
//
// func @f(args: (v0)) -> 1 {
// #0:
//     v0 = arg 0
//     v1 = const 0x00
//     v2 = const 0x01
//     v3 = const 0x02
//     v4 = const 0x03
//     v5 = const 0x04
//     v6 = const 0x05
//     v7 = const 0x06
//     v8 = const 0x07
//     v9 = const 0x08
//     v10 = const 0x09
//     v11 = const 0x0a
//     v12 = const 0x0b
//     v13 = const 0x0c
//     v14 = const 0x0d
//     v15 = const 0x0e
//     v16 = const 0x0f
//     v17 = const 0x10
//     v18 = const 0x11
//     v19 = const 0x12
//     v20 = const 0x13
//     upsilon v1 -> ^v22
//     upsilon v2 -> ^v24
//     upsilon v3 -> ^v26
//     upsilon v4 -> ^v28
//     upsilon v5 -> ^v30
//     upsilon v6 -> ^v32
//     upsilon v7 -> ^v34
//     upsilon v8 -> ^v36
//     upsilon v9 -> ^v38
//     upsilon v10 -> ^v40
//     upsilon v11 -> ^v42
//     upsilon v12 -> ^v44
//     upsilon v13 -> ^v46
//     upsilon v14 -> ^v48
//     upsilon v15 -> ^v50
//     upsilon v16 -> ^v52
//     upsilon v17 -> ^v54
//     upsilon v18 -> ^v56
//     upsilon v19 -> ^v58
//     upsilon v20 -> ^v60
//     jump #1
// #1: preds: #0, #2
//     v22 = phi
//     v23 = builtin @lt v22, v0
//     v24 = phi
//     v26 = phi
//     v28 = phi
//     v30 = phi
//     v32 = phi
//     v34 = phi
//     v36 = phi
//     v38 = phi
//     v40 = phi
//     v42 = phi
//     v44 = phi
//     v46 = phi
//     v48 = phi
//     v50 = phi
//     v52 = phi
//     v54 = phi
//     v56 = phi
//     v58 = phi
//     v60 = phi
//     branch v23, #2, #4
// #2: preds: #1
//     v25 = builtin @add v24, v22
//     v27 = builtin @add v26, v22
//     v29 = builtin @add v28, v22
//     v31 = builtin @add v30, v22
//     v33 = builtin @add v32, v22
//     v35 = builtin @add v34, v22
//     v37 = builtin @add v36, v22
//     v39 = builtin @add v38, v22
//     v41 = builtin @add v40, v22
//     v43 = builtin @add v42, v22
//     v45 = builtin @add v44, v22
//     v47 = builtin @add v46, v22
//     v49 = builtin @add v48, v22
//     v51 = builtin @add v50, v22
//     v53 = builtin @add v52, v22
//     v55 = builtin @add v54, v22
//     v57 = builtin @add v56, v22
//     v59 = builtin @add v58, v22
//     v61 = builtin @add v60, v22
//     v62 = builtin @add v22, v2
//     upsilon v62 -> ^v22
//     upsilon v25 -> ^v24
//     upsilon v27 -> ^v26
//     upsilon v29 -> ^v28
//     upsilon v31 -> ^v30
//     upsilon v33 -> ^v32
//     upsilon v35 -> ^v34
//     upsilon v37 -> ^v36
//     upsilon v39 -> ^v38
//     upsilon v41 -> ^v40
//     upsilon v43 -> ^v42
//     upsilon v45 -> ^v44
//     upsilon v47 -> ^v46
//     upsilon v49 -> ^v48
//     upsilon v51 -> ^v50
//     upsilon v53 -> ^v52
//     upsilon v55 -> ^v54
//     upsilon v57 -> ^v56
//     upsilon v59 -> ^v58
//     upsilon v61 -> ^v60
//     jump #1
// #4: preds: #1
//     v105 = builtin @add v24, v26
//     v106 = builtin @add v105, v28
//     v107 = builtin @add v106, v30
//     v108 = builtin @add v107, v32
//     v109 = builtin @add v108, v34
//     v110 = builtin @add v109, v36
//     v111 = builtin @add v110, v38
//     v112 = builtin @add v111, v40
//     v113 = builtin @add v112, v42
//     v114 = builtin @add v113, v44
//     v115 = builtin @add v114, v46
//     v116 = builtin @add v115, v48
//     v117 = builtin @add v116, v50
//     v118 = builtin @add v117, v52
//     v119 = builtin @add v118, v54
//     v120 = builtin @add v119, v56
//     v121 = builtin @add v120, v58
//     v122 = builtin @add v121, v60
//     return v122
// }
//
// ===== spill info =====
// CFG[0] <main>
//   spilled: none
// CFG[1] f
//   spilled:
//     phi52 -> mem 0x80
//     phi54 -> mem 0xa0
//     v55 -> mem 0xc0
//     phi56 -> mem 0xe0
//     v57 -> mem 0x0100
//     phi58 -> mem 0x0120
//     v59 -> mem 0x0140
//     phi60 -> mem 0x0160
//   mstore schedule:
//     mstore addr(phi52) <- phi52 (B#1)
//     mstore addr(phi54) <- phi54 (B#1)
//     mstore addr(v55) <- v55 (B#2)
//     mstore addr(phi56) <- phi56 (B#1)
//     mstore addr(v57) <- v57 (B#2)
//     mstore addr(phi58) <- phi58 (B#1)
//     mstore addr(v59) <- v59 (B#2)
//     mstore addr(phi60) <- phi60 (B#1)
