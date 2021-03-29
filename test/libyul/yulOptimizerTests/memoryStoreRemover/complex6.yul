{
    let x := 0
    let y := 10
    let i := 0
    for {} iszero(i) {i := add(i, 1)}
    {
        // can be removed!
        mstore(x, i)
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
//     { pop(i) }
//     pop(mload(200))
// }
