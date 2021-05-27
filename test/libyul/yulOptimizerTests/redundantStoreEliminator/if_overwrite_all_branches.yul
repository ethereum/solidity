{
    let c := calldataload(0)
    // This store will be overwritten in all branches and thus can be removed.
    sstore(c, 1)
    if c {
        sstore(c, 2)
    }
    sstore(c, 3)
}
// ----
// step: redundantStoreEliminator
//
// {
//     let c := calldataload(0)
//     let _2 := 1
//     if c { let _3 := 2 }
//     sstore(c, 3)
// }
