{
    function neverStop() {
        if calldataload(0) { leave } // prevent inlining
    }
    let x := 0
    let y := 1
    sstore(x, y) // should be removed
    neverStop()
    sstore(x, y)
}
// ----
// step: unusedStoreEliminator
//
// {
//     {
//         let x := 0
//         let y := 1
//         neverStop()
//         sstore(x, y)
//     }
//     function neverStop()
//     { if calldataload(0) { leave } }
// }
