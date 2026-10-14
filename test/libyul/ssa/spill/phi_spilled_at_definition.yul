// A spilled phi is addressed at its block entry (def-site), where the merged value is materialized
// on `stackIn`. Here the loop counter `i` is a phi at the loop header; with 17 always-live params
// kept live across the body's sstore chain the layout generator spills, and the phi can be among the
// spilled values. The `memoryguard` prelude (kept alive by `mstore(0x40, g)`) reserves the spill
// region MemoryAddressing bumps.
object "C" {
    code {
        let g := memoryguard(0x80)
        mstore(0x40, g)
        f(calldataload(0x0), calldataload(0x20), calldataload(0x40), calldataload(0x60), calldataload(0x80), calldataload(0xa0), calldataload(0xc0), calldataload(0xe0), calldataload(0x100), calldataload(0x120), calldataload(0x140), calldataload(0x160), calldataload(0x180), calldataload(0x1a0), calldataload(0x1c0), calldataload(0x1e0), calldataload(0x200))
        function f(b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15, b16, b17) {
            for { let i := 0 } lt(i, 5) { i := add(i, 1) } {
                sstore(i, b17)
                sstore(add(i, 1), b16)
                sstore(add(i, 2), b15)
                sstore(add(i, 3), b14)
                sstore(add(i, 4), b13)
                sstore(add(i, 5), b12)
                sstore(add(i, 6), b11)
                sstore(add(i, 7), b10)
                sstore(add(i, 8), b9)
                sstore(add(i, 9), b8)
                sstore(add(i, 10), b7)
                sstore(add(i, 11), b6)
                sstore(add(i, 12), b5)
                sstore(add(i, 13), b4)
                sstore(add(i, 14), b3)
                sstore(add(i, 15), b2)
                sstore(add(i, 16), b1)
            }
        }
    }
}
// ----
// object "C"
// ===== SSA CFG =====
// memoryguard = 0xe0
//
// #0:
//     v0 = memoryguard
//     v1 = const 0x40
//     builtin @mstore v1, v0
//     v3 = const 0x0200
//     v4 = builtin @calldataload v3
//     v5 = const 0x01e0
//     v6 = builtin @calldataload v5
//     v7 = const 0x01c0
//     v8 = builtin @calldataload v7
//     v9 = const 0x01a0
//     v10 = builtin @calldataload v9
//     v11 = const 0x0180
//     v12 = builtin @calldataload v11
//     v13 = const 0x0160
//     v14 = builtin @calldataload v13
//     v15 = const 0x0140
//     v16 = builtin @calldataload v15
//     v17 = const 0x0120
//     v18 = builtin @calldataload v17
//     v19 = const 0x0100
//     v20 = builtin @calldataload v19
//     v21 = const 0xe0
//     v22 = builtin @calldataload v21
//     v23 = const 0xc0
//     v24 = builtin @calldataload v23
//     v25 = const 0xa0
//     v26 = builtin @calldataload v25
//     v27 = const 0x80
//     v28 = builtin @calldataload v27
//     v29 = const 0x60
//     v30 = builtin @calldataload v29
//     v31 = builtin @calldataload v1
//     v32 = const 0x20
//     v33 = builtin @calldataload v32
//     v34 = const 0x00
//     v35 = builtin @calldataload v34
//     call @f v35, v33, v31, v30, v28, v26, v24, v22, v20, v18, v16, v14, v12, v10, v8, v6, v4
//     main_exit
//
// func @f(args: (v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16)) -> 0 {
// #0:
//     v0 = arg 0
//     v1 = arg 1
//     v2 = arg 2
//     v3 = arg 3
//     v4 = arg 4
//     v5 = arg 5
//     v6 = arg 6
//     v7 = arg 7
//     v8 = arg 8
//     v9 = arg 9
//     v10 = arg 10
//     v11 = arg 11
//     v12 = arg 12
//     v13 = arg 13
//     v14 = arg 14
//     v15 = arg 15
//     v16 = arg 16
//     v17 = const 0x00
//     v18 = const 0x05
//     v24 = const 0x01
//     v28 = const 0x02
//     v32 = const 0x03
//     v36 = const 0x04
//     v43 = const 0x06
//     v47 = const 0x07
//     v51 = const 0x08
//     v55 = const 0x09
//     v59 = const 0x0a
//     v63 = const 0x0b
//     v67 = const 0x0c
//     v71 = const 0x0d
//     v75 = const 0x0e
//     v79 = const 0x0f
//     v83 = const 0x10
//     upsilon v17 -> ^v19
//     jump #1
// #1: preds: #0, #2
//     v19 = phi
//     v20 = builtin @lt v19, v18
//     branch v20, #2, #4
// #2: preds: #1
//     builtin @sstore v19, v16
//     v25 = builtin @add v19, v24
//     builtin @sstore v25, v15
//     v29 = builtin @add v19, v28
//     builtin @sstore v29, v14
//     v33 = builtin @add v19, v32
//     builtin @sstore v33, v13
//     v37 = builtin @add v19, v36
//     builtin @sstore v37, v12
//     v40 = builtin @add v19, v18
//     builtin @sstore v40, v11
//     v44 = builtin @add v19, v43
//     builtin @sstore v44, v10
//     v48 = builtin @add v19, v47
//     builtin @sstore v48, v9
//     v52 = builtin @add v19, v51
//     builtin @sstore v52, v8
//     v56 = builtin @add v19, v55
//     builtin @sstore v56, v7
//     v60 = builtin @add v19, v59
//     builtin @sstore v60, v6
//     v64 = builtin @add v19, v63
//     builtin @sstore v64, v5
//     v68 = builtin @add v19, v67
//     builtin @sstore v68, v4
//     v72 = builtin @add v19, v71
//     builtin @sstore v72, v3
//     v76 = builtin @add v19, v75
//     builtin @sstore v76, v2
//     v80 = builtin @add v19, v79
//     builtin @sstore v80, v1
//     v84 = builtin @add v19, v83
//     builtin @sstore v84, v0
//     v86 = builtin @add v19, v24
//     upsilon v86 -> ^v19
//     jump #1
// #4: preds: #1
//     return
// }
//
// ===== spill info =====
// CFG[0] <main>
//   spilled:
//     v33 -> mem 0x80
//   mstore schedule:
//     mstore addr(v33) <- v33 (B#0)
// CFG[1] f
//   spilled:
//     v0 -> mem 0xa0
//     phi19 -> mem 0xc0
//   mstore schedule:
//     mstore addr(v0) <- v0 (B#0)
//     mstore addr(phi19) <- phi19 (B#1)
