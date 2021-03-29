// Test to see if the rule is applied in presence of msize
{
    pop(msize())
    let x := 100
    // cannot be removed because of the msize
    mstore(x, 100)
}
// ----
// step: memoryStoreRemover
//
// {
//     pop(msize())
//     let x := 100
//     mstore(x, 100)
// }
