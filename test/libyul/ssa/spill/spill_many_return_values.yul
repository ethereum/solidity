{
    mstore(0x40, memoryguard(0x120))
    function many() -> r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15, r16, r17 {}
    let v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17 := many()
    sstore(v0, v1)
    sstore(v2, v3)
    sstore(v4, v5)
    sstore(v6, v7)
    sstore(v8, v9)
    sstore(v10, v11)
    sstore(v12, v13)
    sstore(v14, v15)
    sstore(v16, v17)
}
// ----
// object "object"
// ===== SSA CFG =====
// memoryguard = 0x0160
//
// #0:
//     v0 = memoryguard
//     v1 = const 0x40
//     builtin @mstore v1, v0
//     v3 = call @many
//     v4 = proj v3, 0
//     v5 = proj v3, 1
//     v6 = proj v3, 2
//     v7 = proj v3, 3
//     v8 = proj v3, 4
//     v9 = proj v3, 5
//     v10 = proj v3, 6
//     v11 = proj v3, 7
//     v12 = proj v3, 8
//     v13 = proj v3, 9
//     v14 = proj v3, 10
//     v15 = proj v3, 11
//     v16 = proj v3, 12
//     v17 = proj v3, 13
//     v18 = proj v3, 14
//     v19 = proj v3, 15
//     v20 = proj v3, 16
//     v21 = proj v3, 17
//     builtin @sstore v4, v5
//     builtin @sstore v6, v7
//     builtin @sstore v8, v9
//     builtin @sstore v10, v11
//     builtin @sstore v12, v13
//     builtin @sstore v14, v15
//     builtin @sstore v16, v17
//     builtin @sstore v18, v19
//     builtin @sstore v20, v21
//     main_exit
//
// func @many(args: ()) -> 18 {
// #0:
//     v0 = const 0x00
//     return v0, v0, v0, v0, v0, v0, v0, v0, v0, v0, v0, v0, v0, v0, v0, v0, v0, v0
// }
//
// ===== spill info =====
// CFG[0] <main>
//   spilled:
//     v5 (value) -> mem 0x0120
//     v21 (value) -> mem 0x0140
//   mstore schedule:
//     mstore addr(v5) <- v5 (B#0)
//     mstore addr(v21) <- v21 (B#0)
// CFG[1] many
//   spilled: none
