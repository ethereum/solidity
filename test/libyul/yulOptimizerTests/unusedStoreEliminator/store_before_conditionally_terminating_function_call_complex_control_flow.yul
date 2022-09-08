{
    function conditionallyStop() {
        if calldataload(0) { leave }
        return(0, 0)
    }
    function g() {
        let a := 0
        let b := 1
        sstore(a, b)
    }
    let x := 0
    let y := 1
    let z := 2
    switch calldataload(64)
    case 0 {
        sstore(z, x)
        g()
    }
    default {
        sstore(x, z) // used to be removed due to the StorageWriteRemovalBeforeConditionalTermination
    }
    conditionallyStop()
    switch calldataload(32)
    case 0 {
        revert(0, 0)
    }
    default {
        sstore(x, z)
    }
}
// ----
// step: unusedStoreEliminator
//
// {
//     {
//         let x := 0
//         let y := 1
//         let z := 2
//         switch calldataload(64)
//         case 0 {
//             sstore(z, x)
//             g()
//         }
//         default { sstore(x, z) }
//         conditionallyStop()
//         switch calldataload(32)
//         case 0 { revert(0, 0) }
//         default { sstore(x, z) }
//     }
//     function conditionallyStop()
//     {
//         if calldataload(0) { leave }
//         return(0, 0)
//     }
//     function g()
//     {
//         let a := 0
//         sstore(a, 1)
//     }
// }
