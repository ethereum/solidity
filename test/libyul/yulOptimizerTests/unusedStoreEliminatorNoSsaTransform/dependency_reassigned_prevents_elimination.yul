{
    let x := calldataload(0)
    x := calldataload(32) // Reassignment; breaks tracking of dependent variables
    let a := add(x, 32)
    let b := x
    let outLen := 32
    mstore(a, 0xAA) // Unused, no overlap with [b, b+outLen]
    return(b, outLen)
}
// ----
// step: unusedStoreEliminatorNoSsaTransform
//
// {
//     let x := calldataload(0)
//     x := calldataload(32)
//     let a := add(x, 32)
//     let b := x
//     let outLen := 32
//     mstore(a, 0xAA)
//     return(b, outLen)
// }
