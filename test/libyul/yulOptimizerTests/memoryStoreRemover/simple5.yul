{
    // x is not in SSA form
    let x := 10
    if calldataload(0) {
       x := 25
    }
    let y := 20
    // Should not be removed because x is not in SSA form
    mstore(x, y)
}
// ----
// step: memoryStoreRemover
//
// {
//     let x := 10
//     if calldataload(0) { x := 25 }
//     let y := 20
//     mstore(x, y)
// }
