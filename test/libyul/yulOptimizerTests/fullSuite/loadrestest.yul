{
    let _1 := memoryguard(0x80)
    mstore(64, _1)
    let _2 := 0
    switch shr(224, calldataload(_2))
    case 0x06661abd {
        mstore(_1, sload(_2))
        return(_1, 32)
    }
    case 0xc2985578 {
        return(mload(64), _2)
    }
}
// ----
// step: fullSuite
//
// {
//     {
//         let _1 := memoryguard(0x80)
//         mstore(64, _1)
//         switch shr(224, calldataload(0))
//         case 0x06661abd {
//             mstore(_1, sload(0))
//             return(_1, 32)
//         }
//         case 0xc2985578 { return(_1, 0) }
//     }
// }
