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
//     let _246 := mload(0)
//     let abi_encode_pos := 0x20
//     let abi_encode_length_5 := mload(_246)
//     mstore(0x20, abi_encode_length_5)
//     abi_encode_pos := 64
//     let abi_encode_srcPtr := add(_246, 0x20)
//     for {
//         let abi_encode_i_5 := 0
//     }
//     lt(abi_encode_i_5, abi_encode_length_5)
//     {
//         abi_encode_i_5 := add(abi_encode_i_5, 1)
//     }
//     {
//         let _456 := mload(abi_encode_srcPtr)
//         let abi_encode_pos_1_6 := abi_encode_pos
//         let abi_encode_srcPtr_1_5 := _456
//         for {
//             let abi_encode_i_6_5 := 0
//         }
//         lt(abi_encode_i_6_5, 0x3)
//         {
//             abi_encode_i_6_5 := add(abi_encode_i_6_5, 1)
//         }
//         {
//             mstore(abi_encode_pos_1_6, and(mload(abi_encode_srcPtr_1_5), 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF))
//             abi_encode_srcPtr_1_5 := add(abi_encode_srcPtr_1_5, 0x20)
//             abi_encode_pos_1_6 := add(abi_encode_pos_1_6, 0x20)
//         }
//         abi_encode_srcPtr := add(abi_encode_srcPtr, 0x20)
//         abi_encode_pos := add(abi_encode_pos, 0x60)
//     }
//     let _248 := mload(0x40)
//     let _250 := mload(0x20)
//     if slt(sub(_248, _250), 128)
//     {
//         revert(0, 0)
//     }
//     let abi_decode_value0_2_4 := calldataload(_250)
//     let abi_decode_value1_2_4 := calldataload(add(_250, 32))
//     let abi_decode_offset_18 := calldataload(add(_250, 64))
//     if gt(abi_decode_offset_18, 0xffffffffffffffff)
//     {
//         revert(0, 0)
//     }
//     let _474 := add(_250, abi_decode_offset_18)
//     if iszero(slt(add(_474, 0x1f), _248))
//     {
//         revert(0, 0)
//     }
//     let abi_decode_length_4_1 := calldataload(_474)
//     if gt(abi_decode_length_4_1, 0xffffffffffffffff)
//     {
//         revert(0, 0)
//     }
//     let abi_decode_array_allo__371_1 := mul(abi_decode_length_4_1, 0x20)
//     let abi_decode_array_4_1_1 := allocateMemory(add(abi_decode_array_allo__371_1, 0x20))
//     let abi_decode_dst_4_7 := abi_decode_array_4_1_1
//     mstore(abi_decode_array_4_1_1, abi_decode_length_4_1)
//     let abi_decode_offset_5_1_1 := add(_474, 0x20)
//     abi_decode_dst_4_7 := add(abi_decode_array_4_1_1, 0x20)
//     let abi_decode_src_4_5 := abi_decode_offset_5_1_1
//     if gt(add(add(_474, abi_decode_array_allo__371_1), 0x20), _248)
//     {
//         revert(0, 0)
//     }
//     for {
//         let abi_decode_i_4_5 := 0
//     }
//     lt(abi_decode_i_4_5, abi_decode_length_4_1)
//     {
//         abi_decode_i_4_5 := add(abi_decode_i_4_5, 1)
//     }
//     {
//         mstore(abi_decode_dst_4_7, calldataload(abi_decode_src_4_5))
//         abi_decode_dst_4_7 := add(abi_decode_dst_4_7, 0x20)
//         abi_decode_src_4_5 := add(abi_decode_src_4_5, 0x20)
//     }
//     let abi_decode_offset_19 := calldataload(add(_250, 96))
//     if gt(abi_decode_offset_19, 0xffffffffffffffff)
//     {
//         revert(0, 0)
//     }
//     let _481 := add(_250, abi_decode_offset_19)
//     if iszero(slt(add(_481, 0x1f), _248))
//     {
//         revert(0, 0)
//     }
//     let abi_decode_length_1_1 := calldataload(_481)
//     if gt(abi_decode_length_1_1, 0xffffffffffffffff)
//     {
//         revert(0, 0)
//     }
//     let abi_decode_array_1_1_1 := allocateMemory(add(mul(abi_decode_length_1_1, 0x20), 0x20))
//     let abi_decode_dst_1_7 := abi_decode_array_1_1_1
//     mstore(abi_decode_array_1_1_1, abi_decode_length_1_1)
//     let abi_decode_offset_2_1_1 := add(_481, 0x20)
//     abi_decode_dst_1_7 := add(abi_decode_array_1_1_1, 0x20)
//     let abi_decode_src_1_5 := abi_decode_offset_2_1_1
//     if gt(add(add(_481, mul(abi_decode_length_1_1, 0x40)), 0x20), _248)
//     {
//         revert(0, 0)
//     }
//     for {
//         let abi_decode_i_1_5 := 0
//     }
//     lt(abi_decode_i_1_5, abi_decode_length_1_1)
//     {
//         abi_decode_i_1_5 := add(abi_decode_i_1_5, 1)
//     }
//     {
//         if iszero(slt(add(abi_decode_src_1_5, 0x1f), _248))
//         {
//             revert(0, 0)
//         }
//         if 0
//         {
//             revert(0, 0)
//         }
//         let allocateMe_memPtr_5 := mload(64)
//         let allocateMe_newFreePtr := add(allocateMe_memPtr_5, 64)
//         if or(gt(allocateMe_newFreePtr, 0xffffffffffffffff), lt(allocateMe_newFreePtr, allocateMe_memPtr_5))
//         {
//             revert(0, 0)
//         }
//         mstore(64, allocateMe_newFreePtr)
//         let abi_decode_abi_decode_dst_2_5 := allocateMe_memPtr_5
//         let abi_decode_abi_decode_src_2_5 := abi_decode_src_1_5
//         if gt(add(abi_decode_src_1_5, 64), _248)
//         {
//             revert(0, 0)
//         }
//         for {
//             let abi_decode_abi_decode_i_2_5 := 0
//         }
//         lt(abi_decode_abi_decode_i_2_5, 0x2)
//         {
//             abi_decode_abi_decode_i_2_5 := add(abi_decode_abi_decode_i_2_5, 1)
//         }
//         {
//             mstore(abi_decode_abi_decode_dst_2_5, calldataload(abi_decode_abi_decode_src_2_5))
//             abi_decode_abi_decode_dst_2_5 := add(abi_decode_abi_decode_dst_2_5, 0x20)
//             abi_decode_abi_decode_src_2_5 := add(abi_decode_abi_decode_src_2_5, 0x20)
//         }
//         mstore(abi_decode_dst_1_7, allocateMe_memPtr_5)
//         abi_decode_dst_1_7 := add(abi_decode_dst_1_7, 0x20)
//         abi_decode_src_1_5 := add(abi_decode_src_1_5, 0x40)
//     }
//     sstore(abi_decode_value0_2_4, abi_decode_value1_2_4)
//     sstore(abi_decode_array_4_1_1, abi_decode_array_1_1_1)
//     sstore(0, abi_encode_pos)
//     function allocateMemory(size) -> memPtr
//     {
//         let memPtr_5 := mload(64)
//         memPtr := memPtr_5
//         let newFreePtr := add(memPtr_5, size)
//         if or(gt(newFreePtr, 0xffffffffffffffff), lt(newFreePtr, memPtr_5))
//         {
//             revert(0, 0)
//         }
//         mstore(64, newFreePtr)
//     }
// }
