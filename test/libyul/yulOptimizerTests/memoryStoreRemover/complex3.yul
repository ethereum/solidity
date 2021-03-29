{
    let x := 32
    let y := 100
    // can be removed
    mstore(x, y)
    if calldataload(0) {
       let cds := calldatasize()
       // Reads from [0, 32), so does not invalidate location x!
       pop(call(1000, 0, 100, 0, 32, 64, 0))
    }

}
// ----
// step: memoryStoreRemover
//
// {
//     let x := 32
//     let y := 100
//     pop(y)
//     if calldataload(0)
//     {
//         let cds := calldatasize()
//         pop(call(1000, 0, 100, 0, 32, 64, 0))
//     }
// }
