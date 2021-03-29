{

    let x := 10
    let y := 20
    pop(mload(x))

    // can't be removed even though nothing is read afterwards, because we'll check reads from the
    // entire block.
    mstore(x, y)
}
// ----
// step: memoryStoreRemover
//
// {
//     let x := 10
//     let y := 20
//     pop(mload(x))
//     mstore(x, y)
// }
