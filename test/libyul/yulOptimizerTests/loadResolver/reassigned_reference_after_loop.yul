{
    let i := 0
    for { let j := 0 } lt(j, calldataload(0)) { j := add(j, 1) } { i := add(i, 1) }
    // Only becomes a copy of `i` after the common subexpression eliminator, so `k` does not reference `i` directly.
    let i1 := add(i, 0)
    let k := add(i1, 2)
    sstore(k, add(sload(k), calldataload(64)))
    // The value of `i` is unknown both before and after this.
    for { let j := 0 } lt(j, calldataload(32)) { j := add(j, 1) } { i := add(i, 1) }
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
//         let j := i
//         for { } lt(j, calldataload(0)) { j := add(j, 1) }
//         { i := add(i, 1) }
//         let _3 := 0
//         let k := add(i, 2)
//         sstore(k, add(sload(k), calldataload(64)))
//         let j_1 := _3
//         for { } lt(j_1, calldataload(32)) { j_1 := add(j_1, 1) }
//         { i := add(i, 1) }
//         sstore(i, calldataload(96))
//         mstore(_3, sload(k))
//         return(_3, 32)
//     }
// }
