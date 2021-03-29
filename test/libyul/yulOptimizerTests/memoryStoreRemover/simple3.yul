{
    let x := 10
    let y := 20
    // This should not be removed
    mstore(x, y)
    // reads from x
    pop(mload(x))
}
// ----
// step: memoryStoreRemover
//
// {
//     let x := 10
//     let y := 20
//     mstore(x, y)
//     pop(mload(x))
// }
