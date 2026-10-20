// `s` is only valid in the first loop iteration: the call inside the loop invalidates it
// for every subsequent iteration. A linear scan that does not account for loops could
// mistake `s` for always valid and eliminate `returndatacopy` regardless.
// `returndatacopy` using `s` must NOT be eliminated.
{
    let s := returndatasize()
    for { let i := 0 } lt(i, 10) { i := add(i, 1) } {
        returndatacopy(0, 0, s)
        pop(staticcall(gas(), 0, 0, 0, 0, 0))
    }
}
// ----
// step: unusedStoreEliminator
//
// {
//     {
//         let s := returndatasize()
//         let i := 0
//         let i_11 := i
//         for { }
//         lt(i, 10)
//         {
//             let i_14 := i
//             i := add(i_14, 1)
//             let i_12 := i
//         }
//         {
//             let i_13 := i
//             returndatacopy(0, 0, s)
//             pop(staticcall(gas(), 0, 0, 0, 0, 0))
//         }
//     }
// }
