{
    function conditionallyStop() {
        if calldataload(0) { leave }
        return(0, 0)
    }
    let x := 0
    let y := 1
    sstore(x, y) // used to be removed due to the to the StorageWriteRemovalBeforeConditionalTermination bug
    conditionallyStop()
    revert(0,0)
}
// ----
// step: unusedStoreEliminator
//
// {
//     {
//         let x := 0
//         sstore(x, 1)
//         conditionallyStop()
//         revert(0, 0)
//     }
//     function conditionallyStop()
//     {
//         if calldataload(0) { leave }
//         return(0, 0)
//     }
// }
