{
    let x := 10
    let y := 20
    mstore(x, y)
}
// ----
// step: memoryStoreRemover
//
// {
//     let x := 10
//     let y := 20
//     pop(y)
// }
