{
    let x := 0
    let y := 32
    // can be removed
    mstore(x, y)
    return(0, 0)
}
// ----
// step: memoryStoreRemover
//
// {
//     let x := 0
//     let y := 32
//     pop(y)
//     return(0, 0)
// }
