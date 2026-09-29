{
    let t := calldataload(0)
    // Only becomes a copy of `t` after the common subexpression eliminator, so `c` does not reference `t` directly.
    let a := add(t, 0)
    let c := add(a, 2)
    let v := calldataload(64)
    let old := sload(c)
    sstore(c, v)
    // `c` still refers to the old value of `t`.
    if calldataload(128) { t := calldataload(32) }
    let x := sload(t)
    let w := calldataload(96)
    // `c` and `t` can be equal, so this store invalidates what is known about `sload(t)`.
    sstore(c, w)
    mstore(0, sload(t))
    return(0, 32)
}
// ----
// step: loadResolver
//
// {
//     {
//         let _1 := 0
//         let t := calldataload(_1)
//         let c := add(t, 2)
//         sstore(c, calldataload(64))
//         if calldataload(128) { t := calldataload(32) }
//         let x := sload(t)
//         sstore(c, calldataload(96))
//         mstore(_1, x)
//         return(_1, 32)
//     }
// }
