{
    let x := 32
    let y := 100
    // cannot be removed
    mstore(x, y)
    let i := 1
    for {} lt(i, 10) {i := add(i, 1) } {
        // does not read from i, however, we cannot infer this. So, the above mstore
        // cannot be removed.
        pop(mload(i))
    }
}
// ----
// step: memoryStoreRemover
//
// {
//     let x := 32
//     let y := 100
//     mstore(x, y)
//     let i := 1
//     for { } lt(i, 10) { i := add(i, 1) }
//     { pop(mload(i)) }
// }
