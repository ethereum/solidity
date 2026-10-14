{
    let x := calldataload(3)
    if mload(42) {
        x := calldataload(77)
    }
    sstore(0, x)
}
// ----
// #0:
//     v0 = const 0x03
//     v1 = builtin @calldataload v0
//     v2 = const 0x2a
//     v3 = builtin @mload v2
//     v4 = const 0x4d
//     v9 = const 0x00
//     branch v3, #1, #3
// #1: preds: #0
//     v5 = builtin @calldataload v4
//     upsilon v5 -> ^v6
//     jump #2
// #2: preds: #3, #1
//     v6 = phi
//     builtin @sstore v9, v6
//     main_exit
// #3: preds: #0
//     upsilon v1 -> ^v6
//     jump #2
//
