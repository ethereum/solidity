{
    let key := 64
    let value := 128

    mstore(key, value)

    if callvalue() { revert(0, 0) }

    let _1 := 0
    let c := calldatasize()

    calldatacopy(128, _1, c)

    // Should prove that location key is not invalidated
    let key_1 := add(key, c)
    mstore(key_1, _1)
    let value_at_key := mload(key)

    // Should prove that both key and key_1 are not invalidated
    pop(delegatecall(gas(), 25, 128, calldatasize(), _1, _1))
    let value_at_key_ := mload(key)
    let value_at_key_1 := mload(key_1)

    let data := _1
    switch returndatasize()
    case 0 { data := 96 }
    default {
        let result := and(add(returndatasize(), 63), not(31))

        // Should be able to replace by value
        let memPtr := mload(key_1)

        let newFreePtr := add(memPtr, result)

        if or(gt(newFreePtr, 0xffffffffffffffff), lt(newFreePtr, memPtr))
        {
            let _tmp := shl(224, 0x4e487b71)
            mstore(_1, _tmp)

            let __tmp := 4
            let tmp_val := 0x41
            mstore(__tmp, tmp_val)

            let tmp_val_1 := 0x24
            revert(_1, tmp_val)
        }

        mstore(key, newFreePtr)
        data := memPtr


        mstore(memPtr, returndatasize())

        let tmp_val_2 := 0x20
        let tmp_val_3 := add(memPtr, tmp_val_2)

        returndatacopy(add(memPtr, 0x20), _1, returndatasize())
    }
    return(add(data, 0x20), mload(data))
}

// ----
// step: memoryLoadResolver
//
// {
//     let key := 64
//     let value := 128
//     mstore(key, value)
//     if callvalue() { revert(0, 0) }
//     let _1 := 0
//     let c := calldatasize()
//     calldatacopy(128, _1, c)
//     let key_1 := add(key, c)
//     mstore(key_1, _1)
//     let value_at_key := mload(key)
//     pop(delegatecall(gas(), 25, 128, calldatasize(), _1, _1))
//     let value_at_key_ := mload(key)
//     let value_at_key_1 := mload(key_1)
//     let data := _1
//     switch returndatasize()
//     case 0 { data := 96 }
//     default {
//         let result := and(add(returndatasize(), 63), not(31))
//         let memPtr := mload(key_1)
//         let newFreePtr := add(memPtr, result)
//         if or(gt(newFreePtr, 0xffffffffffffffff), lt(newFreePtr, memPtr))
//         {
//             let _tmp := shl(224, 0x4e487b71)
//             mstore(_1, _tmp)
//             let __tmp := 4
//             let tmp_val := 0x41
//             mstore(__tmp, tmp_val)
//             let tmp_val_1 := 0x24
//             revert(_1, tmp_val)
//         }
//         mstore(key, newFreePtr)
//         data := memPtr
//         mstore(memPtr, returndatasize())
//         let tmp_val_2 := 0x20
//         let tmp_val_3 := add(memPtr, tmp_val_2)
//         returndatacopy(add(memPtr, 0x20), _1, returndatasize())
//     }
//     return(add(data, 0x20), mload(data))
// }
