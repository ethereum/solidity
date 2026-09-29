{
    let v := sload(0)
    if calldataload(0) { v := sload(1) }
    // Only becomes a copy of `v` after the common subexpression eliminator, so `a` does not reference `v` directly.
    let w := add(v, 0)
    let a := add(w, 0x100)
    mstore(a, add(mload(a), 67))
    // The value of `v` is unknown both before and after this.
    if calldataload(32) { v := sload(2) }
    let b := add(v, 0x200)
    mstore(b, 7)
    // `a` and `b` can be equal, so this cannot be resolved.
    sstore(0, mload(a))
}
// ----
// step: loadResolver
//
// {
//     {
//         let _1 := 0
//         let v := sload(_1)
//         if calldataload(_1) { v := sload(1) }
//         let a := add(v, 0x100)
//         let _9 := add(mload(a), 67)
//         mstore(a, _9)
//         if calldataload(32) { v := sload(2) }
//         mstore(add(v, 0x200), 7)
//         sstore(_1, _9)
//     }
// }
