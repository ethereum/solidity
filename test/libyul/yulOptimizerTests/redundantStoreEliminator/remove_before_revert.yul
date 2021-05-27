{
    let c := calldataload(0)
    mstore(c, 4)
    if c {
        sstore(c, 2)
    }
    let d := 0
    revert(d, d)
}
// ----
// step: redundantStoreEliminator
//
// {
//     let c := calldataload(0)
//     let _2 := 4
//     if c { let _3 := 2 }
//     let d := 0
//     revert(d, d)
// }
