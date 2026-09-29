{
    let v := sload(0)
    let a1 := add(v, 0x20)
    let a := a1
    let x := 67
    // Makes the knowledge base compute the offsets of `a` and `a1` relative to `v`.
    mstore(v, x)
    mstore(a, x)
    v := sload(1)
    // `a1` still holds the old value of `v` plus 0x20, so `c` is 0x20 past `a`.
    let c := add(a1, 0x20)
    let y := 7
    mstore(c, y)
    // `a` and `c` do not overlap, so this can be resolved.
    sstore(0, mload(a))
}
// ----
// step: loadResolverSolo
//
// {
//     let v := sload(0)
//     let a1 := add(v, 0x20)
//     let a := a1
//     let x := 67
//     mstore(v, x)
//     mstore(a, x)
//     v := sload(1)
//     let c := add(a1, 0x20)
//     let y := 7
//     mstore(c, y)
//     sstore(0, x)
// }
