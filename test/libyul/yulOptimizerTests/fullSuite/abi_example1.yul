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
//     let _1 := 0x20
//     let _2 := 0
//     let _218 := mload(_2)
//     let abi_encode_pos := _1
//     let abi_encode_length_68 := mload(_218)
//     mstore(_1, abi_encode_length_68)
//     abi_encode_pos := 64
//     let abi_encode_srcPtr := add(_218, _1)
//     let abi_encode_i_69 := _2
//     for {
//     }
//     lt(abi_encode_i_69, abi_encode_length_68)
//     {
//         abi_encode_i_69 := add(abi_encode_i_69, 1)
//     }
//     {
//         let _580 := mload(abi_encode_srcPtr)
//         let abi_encode_pos_71_672 := abi_encode_pos
//         let abi_encode_srcPtr_73_674 := _580
//         let abi_encode_i_74_675 := _2
//         for {
//         }
//         lt(abi_encode_i_74_675, 0x3)
//         {
//             abi_encode_i_74_675 := add(abi_encode_i_74_675, 1)
//         }
//         {
//             mstore(abi_encode_pos_71_672, and(mload(abi_encode_srcPtr_73_674), 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF))
//             abi_encode_srcPtr_73_674 := add(abi_encode_srcPtr_73_674, _1)
//             abi_encode_pos_71_672 := add(abi_encode_pos_71_672, _1)
//         }
//         abi_encode_srcPtr := add(abi_encode_srcPtr, _1)
//         abi_encode_pos := add(abi_encode_pos, 0x60)
//     }
//     let _220 := mload(64)
//     let _221 := mload(_1)
//     if slt(sub(_220, _221), 128)
//     {
//         revert(_2, _2)
//     }
//     let abi_decode_offset_64 := calldataload(add(_221, 64))
//     let abi_decode__74 := 0xffffffffffffffff
//     if gt(abi_decode_offset_64, abi_decode__74)
//     {
//         revert(_2, _2)
//     }
//     let abi_decode_value2_317 := abi_decode_t_array$_t_uint256_$dyn_memory_ptr(add(_221, abi_decode_offset_64), _220)
//     let abi_decode_offset_65 := calldataload(add(_221, 96))
//     if gt(abi_decode_offset_65, abi_decode__74)
//     {
//         revert(_2, _2)
//     }
//     let abi_decode_value3_318 := abi_decode_t_array$_t_array$_t_uint256_$2_memory_$dyn_memory_ptr(add(_221, abi_decode_offset_65), _220)
//     sstore(calldataload(_221), calldataload(add(_221, _1)))
//     sstore(abi_decode_value2_317, abi_decode_value3_318)
//     sstore(_2, abi_encode_pos)
//     function abi_decode_t_array$_t_array$_t_uint256_$2_memory_$dyn_memory_ptr(offset_3, end_4) -> array_5
//     {
//         if iszero(slt(add(offset_3, 0x1f), end_4))
//         {
//             revert(array_5, array_5)
//         }
//         let length_6 := calldataload(offset_3)
//         array_5 := allocateMemory(array_allocation_size_t_array$_t_address_$dyn_memory(length_6))
//         let dst_7 := array_5
//         mstore(array_5, length_6)
//         let _16 := 0x20
//         dst_7 := add(array_5, _16)
//         let src_8 := add(offset_3, _16)
//         if gt(add(add(offset_3, mul(length_6, 0x40)), _16), end_4)
//         {
//             revert(0, 0)
//         }
//         let i_9 := 0
//         for {
//         }
//         lt(i_9, length_6)
//         {
//             i_9 := add(i_9, 1)
//         }
//         {
//             if iszero(slt(add(src_8, 0x1f), end_4))
//             {
//                 revert(0, 0)
//             }
//             let abi_decode_dst_15 := allocateMemory(array_allocation_size_t_array$_t_uint256_$2_memory(0x2))
//             let abi_decode_dst_15_1155 := abi_decode_dst_15
//             let abi_decode_src_16 := src_8
//             let abi_decode__239 := add(src_8, 0x40)
//             if gt(abi_decode__239, end_4)
//             {
//                 revert(0, 0)
//             }
//             let abi_decode_i_17 := 0
//             for {
//             }
//             lt(abi_decode_i_17, 0x2)
//             {
//                 abi_decode_i_17 := add(abi_decode_i_17, 1)
//             }
//             {
//                 mstore(abi_decode_dst_15, calldataload(abi_decode_src_16))
//                 abi_decode_dst_15 := add(abi_decode_dst_15, _16)
//                 abi_decode_src_16 := add(abi_decode_src_16, _16)
//             }
//             mstore(dst_7, abi_decode_dst_15_1155)
//             dst_7 := add(dst_7, _16)
//             src_8 := abi_decode__239
//         }
//     }
//     function abi_decode_t_array$_t_uint256_$dyn_memory_ptr(offset_27, end_28) -> array_29
//     {
//         if iszero(slt(add(offset_27, 0x1f), end_28))
//         {
//             revert(array_29, array_29)
//         }
//         let length_30 := calldataload(offset_27)
//         array_29 := allocateMemory(array_allocation_size_t_array$_t_address_$dyn_memory(length_30))
//         let dst_31 := array_29
//         mstore(array_29, length_30)
//         let _52 := 0x20
//         dst_31 := add(array_29, _52)
//         let src_32 := add(offset_27, _52)
//         if gt(add(add(offset_27, mul(length_30, _52)), _52), end_28)
//         {
//             revert(0, 0)
//         }
//         let i_33 := 0
//         for {
//         }
//         lt(i_33, length_30)
//         {
//             i_33 := add(i_33, 1)
//         }
//         {
//             mstore(dst_31, calldataload(src_32))
//             dst_31 := add(dst_31, _52)
//             src_32 := add(src_32, _52)
//         }
//     }
//     function allocateMemory(size) -> memPtr
//     {
//         memPtr := mload(64)
//         let newFreePtr := add(memPtr, size)
//         if or(gt(newFreePtr, 0xffffffffffffffff), lt(newFreePtr, memPtr))
//         {
//             revert(0, 0)
//         }
//         mstore(64, newFreePtr)
//     }
//     function array_allocation_size_t_array$_t_address_$dyn_memory(length_90) -> size_91
//     {
//         if gt(length_90, 0xffffffffffffffff)
//         {
//             revert(0, 0)
//         }
//         size_91 := add(mul(length_90, 0x20), 0x20)
//     }
//     function array_allocation_size_t_array$_t_uint256_$2_memory(length_94) -> size_95
//     {
//         if gt(length_94, 0xffffffffffffffff)
//         {
//             revert(0, 0)
//         }
//         size_95 := mul(length_94, 0x20)
//     }
// }
