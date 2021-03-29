{
    let x := 10
    let y := 20
    let z := 30

    // even though `x` is assigned twice, both `mstores` can be removed.
    mstore(x, y)
    mstore(x, z)
    pop(mload(100))
}
// ----
// step: memoryStoreRemover
//
// {
//     let x := 10
//     let y := 20
//     let z := 30
//     pop(y)
//     pop(z)
//     pop(mload(100))
// }
