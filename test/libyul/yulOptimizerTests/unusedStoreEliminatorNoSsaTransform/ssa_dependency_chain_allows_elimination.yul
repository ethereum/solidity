{
    let x := calldataload(0)
    let y := add(x, 32)
    let a := add(y, 32)
    let b := x
    let outLen := 32
    mstore(a, 0xAA)
    return(b, outLen)
}
// ----
// step: unusedStoreEliminatorNoSsaTransform
//
// {
//     let x := calldataload(0)
//     let y := add(x, 32)
//     let a := add(y, 32)
//     let b := x
//     let outLen := 32
//     return(b, outLen)
// }
