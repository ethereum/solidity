{
    let x := calldataload(1)

    mstore(0, x)

    if calldataload(0)
    { x := 7 mstore(0, 2) revert(0, 0x20) }

    sstore(0, mload(0))
}
// ----
// step: loadResolver
//
// {
//     {
//         let x := calldataload(1)
//         let _2 := 0
//         mstore(_2, x)
//         if calldataload(_2)
//         {
//             x := 7
//             mstore(_2, 2)
//             revert(_2, 0x20)
//         }
//         sstore(_2, x)
//     }
// }
