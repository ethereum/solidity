{
    function conditionallyStop() {
        if calldataload(0) { leave }
        return(0, 0)
    }
    let x := 0
    let y := 1
    sstore(x, y)
    conditionallyStop()
    sstore(x, y)
}
// ----
// step: unusedStoreEliminator
//
// {
//     {
//         let x := 0
//         let y := 1
//         conditionallyStop()
//         sstore(x, y)
//     }
//     function conditionallyStop()
//     {
//         if calldataload(0) { leave }
//         return(0, 0)
//     }
// }
