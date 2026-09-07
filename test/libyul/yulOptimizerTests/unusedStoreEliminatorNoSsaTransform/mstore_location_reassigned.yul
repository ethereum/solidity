{
    let x := calldataload(4)
    let x_5 := x
    let a := add(x_5, 32)
    x := add(x_5, 32)
    let b := x
    let outLen := 32
    mstore(a, 0xAA)
    return(b, outLen)
}

// ----
// step: unusedStoreEliminatorNoSsaTransform
//
// {
//     let x := calldataload(4)
//     let x_5 := x
//     let a := add(x_5, 32)
//     x := add(x_5, 32)
//     let b := x
//     let outLen := 32
//     mstore(a, 0xAA)
//     return(b, outLen)
// }
