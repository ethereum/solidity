{
    let x := 10
    let y := 100
    if calldataload(0) {
       // should be removed
       mstore(x, y)
    }

    if calldataload(0) {
       pop(mload(y))
    }
}
// ----
// step: memoryStoreRemover
//
// {
//     let x := 10
//     let y := 100
//     if calldataload(0) { pop(y) }
//     if calldataload(0) { pop(mload(y)) }
// }
