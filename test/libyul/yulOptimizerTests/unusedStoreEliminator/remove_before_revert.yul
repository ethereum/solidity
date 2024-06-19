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
// step: unusedStoreEliminator
//
// {
//     {
//         let c := calldataload(0)
//         let _1 := 4
//         if c { let _2 := 2 }
//         let d := 0
//         revert(d, d)
//     }
// }
