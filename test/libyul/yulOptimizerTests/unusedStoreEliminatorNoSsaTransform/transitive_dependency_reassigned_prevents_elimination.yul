{
    let x := calldataload(0)
    let y := add(x, 32) // Transitively depends on x
    let b := x
    x := calldataload(32) // Reassignment; breaks tracking of x and, transitively, of y
    let a := y
    let outLen := 32
    mstore(a, 0xAA) // Unused, no overlap with [b, b+outLen]
    return(b, outLen)
}
// ----
// step: unusedStoreEliminatorNoSsaTransform
//
// {
//     let x := calldataload(0)
//     let y := add(x, 32)
//     let b := x
//     x := calldataload(32)
//     let a := y
//     let outLen := 32
//     mstore(a, 0xAA)
//     return(b, outLen)
// }
