{
    let x := 10
    let y := 60
    // can be removed
    mstore(x, y)
    // Does not read from [10, 42)
    pop(mload(y))
}
// ----
// step: memoryStoreRemover
//
// {
//     let x := 10
//     let y := 60
//     pop(y)
//     pop(mload(y))
// }
