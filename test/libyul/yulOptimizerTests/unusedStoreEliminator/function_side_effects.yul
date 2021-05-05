{
    function justStop() { return(0, 0) }
    function justRevert() { revert(0, 0) }

    let x := 0
    let y := 1
    let a := 0x80
    let b := 7
    let c := 9
    switch calldataload(0)
    case 0
    {
        sstore(x, y)
        mstore(a, b)
        justStop()
        sstore(x, y)
        mstore(a, b)
    }
    case 1
    {
        sstore(x, y)
        mstore(a, b)
        justRevert()
        sstore(x, y)
        mstore(a, b)
    }
}
// ----
// step: unusedStoreEliminator
//
// {
//     {
//         let x := 0
//         let y := 1
//         let a := 0x80
//         let b := 7
//         let c := 9
//         switch calldataload(0)
//         case 0 {
//             sstore(x, y)
//             mstore(a, b)
//             justStop()
//             sstore(x, y)
//         }
//         case 1 {
//             mstore(a, b)
//             justRevert()
//             sstore(x, y)
//         }
//     }
//     function justStop()
//     { return(0, 0) }
//     function justRevert()
//     { revert(0, 0) }
// }
