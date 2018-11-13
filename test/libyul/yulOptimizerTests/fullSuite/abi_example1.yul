{
    let x := abi_encode_t_array$_t_array$_t_contract$_C_$55_$3_memory_$dyn_memory_ptr_to_t_array$_t_array$_t_address_$3_memory_$dyn_memory_ptr(mload(0), 0x20)
    let a, b, c, d := abi_decode_tuple_t_uint256t_uint256t_array$_t_uint256_$dyn_memory_ptrt_array$_t_array$_t_uint256_$2_memory_$dyn_memory_ptr(mload(0x20), mload(0x40))
    sstore(a, b)
    sstore(c, d)
    sstore(0, x)

    function abi_decode_t_address(offset, end) -> value
    {
        value := cleanup_revert_t_address(calldataload(offset))
    }
    function abi_decode_t_array$_t_address_$dyn_memory(offset, end) -> array
    {
        if iszero(slt(add(offset, 0x1f), end))
        {
            revert(0, 0)
        }
        let length := calldataload(offset)
        array := allocateMemory(array_allocation_size_t_array$_t_address_$dyn_memory(length))
        let dst := array
        mstore(array, length)
        offset := add(offset, 0x20)
        dst := add(dst, 0x20)
        let src := offset
        if gt(add(src, mul(length, 0x20)), end)
        {
            revert(0, 0)
        }
        for {
            let i := 0
        }
        lt(i, length)
        {
            i := add(i, 1)
        }
        {
            let elementPos := src
            mstore(dst, abi_decode_t_address(elementPos, end))
            dst := add(dst, 0x20)
            src := add(src, 0x20)
        }
    }
    function abi_decode_t_array$_t_array$_t_uint256_$2_memory_$dyn_memory_ptr(offset, end) -> array
    {
        if iszero(slt(add(offset, 0x1f), end))
        {
            revert(0, 0)
        }
        let length := calldataload(offset)
        array := allocateMemory(array_allocation_size_t_array$_t_array$_t_uint256_$2_memory_$dyn_memory_ptr(length))
        let dst := array
        mstore(array, length)
        offset := add(offset, 0x20)
        dst := add(dst, 0x20)
        let src := offset
        if gt(add(src, mul(length, 0x40)), end)
        {
            revert(0, 0)
        }
        for {
            let i := 0
        }
        lt(i, length)
        {
            i := add(i, 1)
        }
        {
            let elementPos := src
            mstore(dst, abi_decode_t_array$_t_uint256_$2_memory(elementPos, end))
            dst := add(dst, 0x20)
            src := add(src, 0x40)
        }
    }
    function abi_decode_t_array$_t_uint256_$2_memory(offset, end) -> array
    {
        if iszero(slt(add(offset, 0x1f), end))
        {
            revert(0, 0)
        }
        let length := 0x2
        array := allocateMemory(array_allocation_size_t_array$_t_uint256_$2_memory(length))
        let dst := array
        let src := offset
        if gt(add(src, mul(length, 0x20)), end)
        {
            revert(0, 0)
        }
        for {
            let i := 0
        }
        lt(i, length)
        {
            i := add(i, 1)
        }
        {
            let elementPos := src
            mstore(dst, abi_decode_t_uint256(elementPos, end))
            dst := add(dst, 0x20)
            src := add(src, 0x20)
        }
    }
    function abi_decode_t_array$_t_uint256_$dyn_memory(offset, end) -> array
    {
        if iszero(slt(add(offset, 0x1f), end))
        {
            revert(0, 0)
        }
        let length := calldataload(offset)
        array := allocateMemory(array_allocation_size_t_array$_t_uint256_$dyn_memory(length))
        let dst := array
        mstore(array, length)
        offset := add(offset, 0x20)
        dst := add(dst, 0x20)
        let src := offset
        if gt(add(src, mul(length, 0x20)), end)
        {
            revert(0, 0)
        }
        for {
            let i := 0
        }
        lt(i, length)
        {
            i := add(i, 1)
        }
        {
            let elementPos := src
            mstore(dst, abi_decode_t_uint256(elementPos, end))
            dst := add(dst, 0x20)
            src := add(src, 0x20)
        }
    }
    function abi_decode_t_array$_t_uint256_$dyn_memory_ptr(offset, end) -> array
    {
        if iszero(slt(add(offset, 0x1f), end))
        {
            revert(0, 0)
        }
        let length := calldataload(offset)
        array := allocateMemory(array_allocation_size_t_array$_t_uint256_$dyn_memory_ptr(length))
        let dst := array
        mstore(array, length)
        offset := add(offset, 0x20)
        dst := add(dst, 0x20)
        let src := offset
        if gt(add(src, mul(length, 0x20)), end)
        {
            revert(0, 0)
        }
        for {
            let i := 0
        }
        lt(i, length)
        {
            i := add(i, 1)
        }
        {
            let elementPos := src
            mstore(dst, abi_decode_t_uint256(elementPos, end))
            dst := add(dst, 0x20)
            src := add(src, 0x20)
        }
    }
    function abi_decode_t_contract$_C_$55(offset, end) -> value
    {
        value := cleanup_revert_t_contract$_C_$55(calldataload(offset))
    }
    function abi_decode_t_struct$_S_$11_memory_ptr(headStart, end) -> value
    {
        if slt(sub(end, headStart), 0x60)
        {
            revert(0, 0)
        }
        value := allocateMemory(0x60)
        {
            let offset := 0
            mstore(add(value, 0x0), abi_decode_t_uint256(add(headStart, offset), end))
        }
        {
            let offset := calldataload(add(headStart, 32))
            if gt(offset, 0xffffffffffffffff)
            {
                revert(0, 0)
            }
            mstore(add(value, 0x20), abi_decode_t_array$_t_uint256_$dyn_memory(add(headStart, offset), end))
        }
        {
            let offset := calldataload(add(headStart, 64))
            if gt(offset, 0xffffffffffffffff)
            {
                revert(0, 0)
            }
            mstore(add(value, 0x40), abi_decode_t_array$_t_address_$dyn_memory(add(headStart, offset), end))
        }
    }
    function abi_decode_t_uint256(offset, end) -> value
    {
        value := cleanup_revert_t_uint256(calldataload(offset))
    }
    function abi_decode_t_uint8(offset, end) -> value
    {
        value := cleanup_revert_t_uint8(calldataload(offset))
    }
    function abi_decode_tuple_t_contract$_C_$55t_uint8(headStart, dataEnd) -> value0, value1
    {
        if slt(sub(dataEnd, headStart), 64)
        {
            revert(0, 0)
        }
        {
            let offset := 0
            value0 := abi_decode_t_contract$_C_$55(add(headStart, offset), dataEnd)
        }
        {
            let offset := 32
            value1 := abi_decode_t_uint8(add(headStart, offset), dataEnd)
        }
    }
    function abi_decode_tuple_t_struct$_S_$11_memory_ptrt_uint256(headStart, dataEnd) -> value0, value1
    {
        if slt(sub(dataEnd, headStart), 64)
        {
            revert(0, 0)
        }
        {
            let offset := calldataload(add(headStart, 0))
            if gt(offset, 0xffffffffffffffff)
            {
                revert(0, 0)
            }
            value0 := abi_decode_t_struct$_S_$11_memory_ptr(add(headStart, offset), dataEnd)
        }
        {
            let offset := 32
            value1 := abi_decode_t_uint256(add(headStart, offset), dataEnd)
        }
    }
    function abi_decode_tuple_t_uint256t_uint256t_array$_t_uint256_$dyn_memory_ptrt_array$_t_array$_t_uint256_$2_memory_$dyn_memory_ptr(headStart, dataEnd) -> value0, value1, value2, value3
    {
        if slt(sub(dataEnd, headStart), 128)
        {
            revert(0, 0)
        }
        {
            let offset := 0
            value0 := abi_decode_t_uint256(add(headStart, offset), dataEnd)
        }
        {
            let offset := 32
            value1 := abi_decode_t_uint256(add(headStart, offset), dataEnd)
        }
        {
            let offset := calldataload(add(headStart, 64))
            if gt(offset, 0xffffffffffffffff)
            {
                revert(0, 0)
            }
            value2 := abi_decode_t_array$_t_uint256_$dyn_memory_ptr(add(headStart, offset), dataEnd)
        }
        {
            let offset := calldataload(add(headStart, 96))
            if gt(offset, 0xffffffffffffffff)
            {
                revert(0, 0)
            }
            value3 := abi_decode_t_array$_t_array$_t_uint256_$2_memory_$dyn_memory_ptr(add(headStart, offset), dataEnd)
        }
    }
    function abi_encode_t_array$_t_array$_t_contract$_C_$55_$3_memory_$dyn_memory_ptr_to_t_array$_t_array$_t_address_$3_memory_$dyn_memory_ptr(value, pos) -> end
    {
        let length := array_length_t_array$_t_array$_t_contract$_C_$55_$3_memory_$dyn_memory_ptr(value)
        mstore(pos, length)
        pos := add(pos, 0x20)
        let srcPtr := array_dataslot_t_array$_t_array$_t_contract$_C_$55_$3_memory_$dyn_memory_ptr(value)
        for {
            let i := 0
        }
        lt(i, length)
        {
            i := add(i, 1)
        }
        {
            abi_encode_t_array$_t_contract$_C_$55_$3_memory_to_t_array$_t_address_$3_memory_ptr(mload(srcPtr), pos)
            srcPtr := array_nextElement_t_array$_t_array$_t_contract$_C_$55_$3_memory_$dyn_memory_ptr(srcPtr)
            pos := add(pos, 0x60)
        }
        end := pos
    }
    function abi_encode_t_array$_t_contract$_C_$55_$3_memory_to_t_array$_t_address_$3_memory_ptr(value, pos)
    {
        let length := array_length_t_array$_t_contract$_C_$55_$3_memory(value)
        let srcPtr := array_dataslot_t_array$_t_contract$_C_$55_$3_memory(value)
        for {
            let i := 0
        }
        lt(i, length)
        {
            i := add(i, 1)
        }
        {
            abi_encode_t_contract$_C_$55_to_t_address(mload(srcPtr), pos)
            srcPtr := array_nextElement_t_array$_t_contract$_C_$55_$3_memory(srcPtr)
            pos := add(pos, 0x20)
        }
    }
    function abi_encode_t_bool_to_t_bool(value, pos)
    {
        mstore(pos, cleanup_assert_t_bool(value))
    }
    function abi_encode_t_contract$_C_$55_to_t_address(value, pos)
    {
        mstore(pos, convert_t_contract$_C_$55_to_t_address(value))
    }
    function abi_encode_t_uint16_to_t_uint16(value, pos)
    {
        mstore(pos, cleanup_assert_t_uint16(value))
    }
    function abi_encode_t_uint24_to_t_uint24(value, pos)
    {
        mstore(pos, cleanup_assert_t_uint24(value))
    }
    function abi_encode_tuple_t_bool__to_t_bool_(headStart, value0) -> tail
    {
        tail := add(headStart, 32)
        abi_encode_t_bool_to_t_bool(value0, add(headStart, 0))
    }
    function abi_encode_tuple_t_uint16_t_uint24_t_array$_t_array$_t_contract$_C_$55_$3_memory_$dyn_memory_ptr__to_t_uint16_t_uint24_t_array$_t_array$_t_address_$3_memory_$dyn_memory_ptr_(headStart, value2, value1, value0) -> tail
    {
        tail := add(headStart, 96)
        abi_encode_t_uint16_to_t_uint16(value0, add(headStart, 0))
        abi_encode_t_uint24_to_t_uint24(value1, add(headStart, 32))
        mstore(add(headStart, 64), sub(tail, headStart))
        tail := abi_encode_t_array$_t_array$_t_contract$_C_$55_$3_memory_$dyn_memory_ptr_to_t_array$_t_array$_t_address_$3_memory_$dyn_memory_ptr(value2, tail)
    }
    function allocateMemory(size) -> memPtr
    {
        memPtr := mload(64)
        let newFreePtr := add(memPtr, size)
        if or(gt(newFreePtr, 0xffffffffffffffff), lt(newFreePtr, memPtr))
        {
            revert(0, 0)
        }
        mstore(64, newFreePtr)
    }
    function array_allocation_size_t_array$_t_address_$dyn_memory(length) -> size
    {
        if gt(length, 0xffffffffffffffff)
        {
            revert(0, 0)
        }
        size := mul(length, 0x20)
        size := add(size, 0x20)
    }
    function array_allocation_size_t_array$_t_array$_t_uint256_$2_memory_$dyn_memory_ptr(length) -> size
    {
        if gt(length, 0xffffffffffffffff)
        {
            revert(0, 0)
        }
        size := mul(length, 0x20)
        size := add(size, 0x20)
    }
    function array_allocation_size_t_array$_t_uint256_$2_memory(length) -> size
    {
        if gt(length, 0xffffffffffffffff)
        {
            revert(0, 0)
        }
        size := mul(length, 0x20)
    }
    function array_allocation_size_t_array$_t_uint256_$dyn_memory(length) -> size
    {
        if gt(length, 0xffffffffffffffff)
        {
            revert(0, 0)
        }
        size := mul(length, 0x20)
        size := add(size, 0x20)
    }
    function array_allocation_size_t_array$_t_uint256_$dyn_memory_ptr(length) -> size
    {
        if gt(length, 0xffffffffffffffff)
        {
            revert(0, 0)
        }
        size := mul(length, 0x20)
        size := add(size, 0x20)
    }
    function array_dataslot_t_array$_t_array$_t_contract$_C_$55_$3_memory_$dyn_memory_ptr(memPtr) -> dataPtr
    {
        dataPtr := add(memPtr, 0x20)
    }
    function array_dataslot_t_array$_t_contract$_C_$55_$3_memory(memPtr) -> dataPtr
    {
        dataPtr := memPtr
    }
    function array_length_t_array$_t_array$_t_contract$_C_$55_$3_memory_$dyn_memory_ptr(value) -> length
    {
        length := mload(value)
    }
    function array_length_t_array$_t_contract$_C_$55_$3_memory(value) -> length
    {
        length := 0x3
    }
    function array_nextElement_t_array$_t_array$_t_contract$_C_$55_$3_memory_$dyn_memory_ptr(memPtr) -> nextPtr
    {
        nextPtr := add(memPtr, 0x20)
    }
    function array_nextElement_t_array$_t_contract$_C_$55_$3_memory(memPtr) -> nextPtr
    {
        nextPtr := add(memPtr, 0x20)
    }
    function cleanup_assert_t_address(value) -> cleaned
    {
        cleaned := cleanup_assert_t_uint160(value)
    }
    function cleanup_assert_t_bool(value) -> cleaned
    {
        cleaned := iszero(iszero(value))
    }
    function cleanup_assert_t_uint16(value) -> cleaned
    {
        cleaned := and(value, 0xFFFF)
    }
    function cleanup_assert_t_uint160(value) -> cleaned
    {
        cleaned := and(value, 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF)
    }
    function cleanup_assert_t_uint24(value) -> cleaned
    {
        cleaned := and(value, 0xFFFFFF)
    }
    function cleanup_revert_t_address(value) -> cleaned
    {
        cleaned := cleanup_assert_t_uint160(value)
    }
    function cleanup_revert_t_contract$_C_$55(value) -> cleaned
    {
        cleaned := cleanup_assert_t_address(value)
    }
    function cleanup_revert_t_uint256(value) -> cleaned
    {
        cleaned := value
    }
    function cleanup_revert_t_uint8(value) -> cleaned
    {
        cleaned := and(value, 0xFF)
    }
    function convert_t_contract$_C_$55_to_t_address(value) -> converted
    {
        converted := convert_t_contract$_C_$55_to_t_uint160(value)
    }
    function convert_t_contract$_C_$55_to_t_uint160(value) -> converted
    {
        converted := cleanup_assert_t_uint160(value)
    }
}
// ----
// fullSuite
// {
//     {
//         let _1 := 0x20
//         let _2 := 0
//         let _485 := mload(_2)
//         let abi_encode_pos := _1
//         let abi_encode_length_68 := mload(_485)
//         mstore(_1, abi_encode_length_68)
//         let abi_encode_pos_590 := 64
//         abi_encode_pos := abi_encode_pos_590
//         let abi_encode_srcPtr := add(_485, _1)
//         for {
//             let abi_encode_i_69 := _2
//         }
//         lt(abi_encode_i_69, abi_encode_length_68)
//         {
//             abi_encode_i_69 := add(abi_encode_i_69, 1)
//         }
//         {
//             let _931 := mload(abi_encode_srcPtr)
//             let abi_encode_pos_71_1037 := abi_encode_pos
//             let abi_encode_length_72_1038 := 0x3
//             let abi_encode_srcPtr_73_1039 := _931
//             for {
//                 let abi_encode_i_74_1040 := _2
//             }
//             lt(abi_encode_i_74_1040, abi_encode_length_72_1038)
//             {
//                 abi_encode_i_74_1040 := add(abi_encode_i_74_1040, 1)
//             }
//             {
//                 mstore(abi_encode_pos_71_1037, and(mload(abi_encode_srcPtr_73_1039), 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF))
//                 abi_encode_srcPtr_73_1039 := add(abi_encode_srcPtr_73_1039, _1)
//                 abi_encode_pos_71_1037 := add(abi_encode_pos_71_1037, _1)
//             }
//             abi_encode_srcPtr := add(abi_encode_srcPtr, _1)
//             abi_encode_pos := add(abi_encode_pos, 0x60)
//         }
//         let _933 := 0x40
//         let _487 := mload(_933)
//         let _488 := mload(_1)
//         let abi_decode_value0_60_618
//         let abi_decode_value0_60 := abi_decode_value0_60_618
//         let abi_decode_value1_61_619
//         let abi_decode_value1_61 := abi_decode_value1_61_619
//         let abi_decode_value2_620
//         let abi_decode_value2 := abi_decode_value2_620
//         let abi_decode_value3_621
//         let abi_decode_value3 := abi_decode_value3_621
//         if slt(sub(_487, _488), 128)
//         {
//             revert(_2, _2)
//         }
//         {
//             abi_decode_value0_60 := calldataload(_488)
//         }
//         {
//             abi_decode_value1_61 := calldataload(add(_488, 32))
//         }
//         {
//             let abi_decode_offset_64 := calldataload(add(_488, abi_encode_pos_590))
//             let _940 := 0xffffffffffffffff
//             if gt(abi_decode_offset_64, _940)
//             {
//                 revert(_2, _2)
//             }
//             let _942 := add(_488, abi_decode_offset_64)
//             if iszero(slt(add(_942, 0x1f), _487))
//             {
//                 revert(_2, _2)
//             }
//             let abi_decode_length_30_1046 := calldataload(_942)
//             if gt(abi_decode_length_30_1046, _940)
//             {
//                 revert(_2, _2)
//             }
//             let abi_decode_array_allo__561 := mul(abi_decode_length_30_1046, _1)
//             let abi_decode_array_29_279_1047 := allocateMemory(add(abi_decode_array_allo__561, _1))
//             let abi_decode_dst_31_1048 := abi_decode_array_29_279_1047
//             mstore(abi_decode_array_29_279_1047, abi_decode_length_30_1046)
//             let abi_decode_offset_27_281_1049 := add(_942, _1)
//             abi_decode_dst_31_1048 := add(abi_decode_array_29_279_1047, _1)
//             let abi_decode_src_32_1050 := abi_decode_offset_27_281_1049
//             if gt(add(add(_942, abi_decode_array_allo__561), _1), _487)
//             {
//                 revert(_2, _2)
//             }
//             for {
//                 let abi_decode_i_33_1052 := _2
//             }
//             lt(abi_decode_i_33_1052, abi_decode_length_30_1046)
//             {
//                 abi_decode_i_33_1052 := add(abi_decode_i_33_1052, 1)
//             }
//             {
//                 mstore(abi_decode_dst_31_1048, calldataload(abi_decode_src_32_1050))
//                 abi_decode_dst_31_1048 := add(abi_decode_dst_31_1048, _1)
//                 abi_decode_src_32_1050 := add(abi_decode_src_32_1050, _1)
//             }
//             abi_decode_value2 := abi_decode_array_29_279_1047
//         }
//         {
//             let abi_decode_offset_65 := calldataload(add(_488, 96))
//             let _945 := 0xffffffffffffffff
//             if gt(abi_decode_offset_65, _945)
//             {
//                 revert(_2, _2)
//             }
//             let _947 := add(_488, abi_decode_offset_65)
//             let abi_decode__489_1056 := 0x1f
//             if iszero(slt(add(_947, abi_decode__489_1056), _487))
//             {
//                 revert(_2, _2)
//             }
//             let abi_decode_length_6_1058 := calldataload(_947)
//             if gt(abi_decode_length_6_1058, _945)
//             {
//                 revert(_2, _2)
//             }
//             let abi_decode_array_5_254_1061 := allocateMemory(add(mul(abi_decode_length_6_1058, _1), _1))
//             let abi_decode_dst_7_1062 := abi_decode_array_5_254_1061
//             mstore(abi_decode_array_5_254_1061, abi_decode_length_6_1058)
//             let abi_decode_offset_3_256_1063 := add(_947, _1)
//             abi_decode_dst_7_1062 := add(abi_decode_array_5_254_1061, _1)
//             let abi_decode_src_8_1064 := abi_decode_offset_3_256_1063
//             if gt(add(add(_947, mul(abi_decode_length_6_1058, _933)), _1), _487)
//             {
//                 revert(_2, _2)
//             }
//             for {
//                 let abi_decode_i_9_1068 := _2
//             }
//             lt(abi_decode_i_9_1068, abi_decode_length_6_1058)
//             {
//                 abi_decode_i_9_1068 := add(abi_decode_i_9_1068, 1)
//             }
//             {
//                 if iszero(slt(add(abi_decode_src_8_1064, abi_decode__489_1056), _487))
//                 {
//                     revert(_2, _2)
//                 }
//                 let abi_decode_abi_decode_length_14_1069 := 0x2
//                 if _2
//                 {
//                     revert(_2, _2)
//                 }
//                 let allocateMe_memPtr_315 := mload(abi_encode_pos_590)
//                 let allocateMe_newFreePtr := add(allocateMe_memPtr_315, abi_encode_pos_590)
//                 if or(gt(allocateMe_newFreePtr, _945), lt(allocateMe_newFreePtr, allocateMe_memPtr_315))
//                 {
//                     revert(_2, _2)
//                 }
//                 mstore(abi_encode_pos_590, allocateMe_newFreePtr)
//                 let abi_decode_abi_decode_dst_15_1071 := allocateMe_memPtr_315
//                 let abi_decode_abi_decode_src_16_1072 := abi_decode_src_8_1064
//                 if gt(add(abi_decode_src_8_1064, abi_encode_pos_590), _487)
//                 {
//                     revert(_2, _2)
//                 }
//                 for {
//                     let abi_decode_abi_decode_i_17_1073 := _2
//                 }
//                 lt(abi_decode_abi_decode_i_17_1073, abi_decode_abi_decode_length_14_1069)
//                 {
//                     abi_decode_abi_decode_i_17_1073 := add(abi_decode_abi_decode_i_17_1073, 1)
//                 }
//                 {
//                     mstore(abi_decode_abi_decode_dst_15_1071, calldataload(abi_decode_abi_decode_src_16_1072))
//                     abi_decode_abi_decode_dst_15_1071 := add(abi_decode_abi_decode_dst_15_1071, _1)
//                     abi_decode_abi_decode_src_16_1072 := add(abi_decode_abi_decode_src_16_1072, _1)
//                 }
//                 mstore(abi_decode_dst_7_1062, allocateMe_memPtr_315)
//                 abi_decode_dst_7_1062 := add(abi_decode_dst_7_1062, _1)
//                 abi_decode_src_8_1064 := add(abi_decode_src_8_1064, _933)
//             }
//             abi_decode_value3 := abi_decode_array_5_254_1061
//         }
//         sstore(abi_decode_value0_60, abi_decode_value1_61)
//         sstore(abi_decode_value2, abi_decode_value3)
//         sstore(_2, abi_encode_pos)
//     }
//     function allocateMemory(size) -> memPtr
//     {
//         let _199 := 64
//         let memPtr_315 := mload(_199)
//         memPtr := memPtr_315
//         let newFreePtr := add(memPtr_315, size)
//         if or(gt(newFreePtr, 0xffffffffffffffff), lt(newFreePtr, memPtr_315))
//         {
//             let _204 := 0
//             revert(_204, _204)
//         }
//         mstore(_199, newFreePtr)
//     }
// }
