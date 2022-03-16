{
    let x := calldataload(1)

    switch calldataload(0)
    case 0 { mstore(0, x) }
    case 1 { mstore(0, x) }

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
//         case 0 { mstore(_2, x) }
//         case 1 { mstore(_2, x) }
//         sstore(_2, mload(_2))
//     }
// }
