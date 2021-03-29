{
    let x := 0
    let y := 10
    let i := 0
    for {} iszero(i) {i := add(i, 1)}
    {
        // cannot be removed!
        mstore(i, y)
    }
    pop(mload(200))
}
// ----
// step: memoryStoreRemover
//
// {
//     let x := 0
//     let y := 10
//     let i := 0
//     for { } iszero(i) { i := add(i, 1) }
//     { mstore(i, y) }
//     pop(mload(200))
// }
