{
    let x := 10
    let y := 10
    // should not be removed because of the if statement
    mstore(x, y)
    if calldataload(0) {
       // reads inside control flow
       pop(mload(x))
    }
}
// ----
// step: memoryStoreRemover
//
// {
//     let x := 10
//     let y := 10
//     mstore(x, y)
//     if calldataload(0) { pop(mload(x)) }
// }
