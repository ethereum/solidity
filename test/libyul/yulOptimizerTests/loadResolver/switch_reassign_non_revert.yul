{
    let x := calldataload(1)

    switch calldataload(0)
    case 0 { revert(0, 0) }
    default { mstore(0, x) x := 7 }

    sstore(0, mload(0))
}
// ----
// step: loadResolver
//
// {
//     {
//         let x := calldataload(1)
//         let _2 := 0
//         switch calldataload(_2)
//         case 0 { revert(_2, _2) }
//         default {
//             mstore(_2, x)
//             x := 7
//         }
//         sstore(_2, mload(_2))
//     }
// }
