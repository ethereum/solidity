{
    mstore(64, 128)
    if callvalue() { revert(0, 0) }
    let _1 := 0
    // The following two statements destroy the
    // knowledge about mload(64)
    //calldatacopy(128, _1, calldatasize())
    //mstore(add(128, calldatasize()), _1)
    pop(delegatecall(gas(), loadimmutable("2"), 128, calldatasize(), _1, _1))
    let data := _1
    switch returndatasize()
    case 0 { data := 96 }
    default {
        let result := and(add(returndatasize(), 63), not(31))
        let memPtr := mload(64)
        let newFreePtr := add(memPtr, result)
        if or(gt(newFreePtr, 0xffffffffffffffff), lt(newFreePtr, memPtr))
        {
            mstore(_1, shl(224, 0x4e487b71))
            mstore(4, 0x41)
            revert(_1, 0x24)
        }
        mstore(64, newFreePtr)
        data := memPtr
        mstore(memPtr, returndatasize())
        returndatacopy(add(memPtr, 0x20), _1, returndatasize())
    }
    return(add(data, 0x20), mload(data))
}
// ----
// step: fullSuite
//
// {
//     {
//         let _1 := 128
//         mstore(64, _1)
//         if callvalue() { revert(0, 0) }
//         let _2 := 0
//         pop(delegatecall(gas(), loadimmutable("2"), _1, calldatasize(), _2, _2))
//         let data := _2
//         switch returndatasize()
//         case 0 { data := 96 }
//         default {
//             let newFreePtr := add(_1, and(add(returndatasize(), 63), not(31)))
//             if or(gt(newFreePtr, 0xffffffffffffffff), lt(newFreePtr, _1))
//             {
//                 mstore(_2, shl(224, 0x4e487b71))
//                 mstore(4, 0x41)
//                 revert(_2, 0x24)
//             }
//             mstore(64, newFreePtr)
//             data := _1
//             mstore(_1, returndatasize())
//             returndatacopy(160, _2, returndatasize())
//         }
//         return(add(data, 0x20), mload(data))
//     }
// }
