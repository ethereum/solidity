{
    let i := 0
    switch calldataload(0) case 0 { i := add(i, 1) }
    // Only becomes a copy of `i` after the expression simplifier, so `k` does not reference `i` directly.
    let i1 := add(i, 0)
    let k := add(i1, 2)
    sstore(k, add(sload(k), calldataload(64)))
    // The value of `i` is unknown both before and after this.
    switch calldataload(32) case 0 { i := add(i, 2) }
    sstore(i, calldataload(96))
    // `i` and `k` can be equal, so this cannot be resolved.
    mstore(0, sload(k))
    return(0, 32)
}
// ----
// step: loadResolver
//
// {
//     {
//         let i := 0
//         switch calldataload(i)
//         case 0 { i := 1 }
//         let _4 := 0
//         let i1 := i
//         let _5 := 2
//         let k := add(i1, _5)
//         sstore(k, add(sload(k), calldataload(64)))
//         let _10 := 32
//         switch calldataload(_10)
//         case 0 { i := add(i, _5) }
//         sstore(i, calldataload(96))
//         mstore(_4, sload(k))
//         return(_4, _10)
//     }
// }
