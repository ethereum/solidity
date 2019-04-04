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
// ====
// step: fullSuite
// ----
// {
//     {
//         let _1 := 0x20
//         let _2 := 0
//         let _3 := mload(_2)
//         let pos := _1
//         let length := mload(_3)
//         mstore(_1, length)
//         pos := 64
//         let srcPtr := add(_3, _1)
//         let i := _2
//         for {
//         }
//         lt(i, length)
//         {
//             i := add(i, 1)
//         }
//         {
//             let _4 := mload(srcPtr)
//             let pos_1 := pos
//             let srcPtr_1 := _4
//             let i_1 := _2
//             for {
//             }
//             lt(i_1, 0x3)
//             {
//                 i_1 := add(i_1, 1)
//             }
//             {
//                 mstore(pos_1, and(mload(srcPtr_1), 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF))
//                 srcPtr_1 := add(srcPtr_1, _1)
//                 pos_1 := add(pos_1, _1)
//             }
//             srcPtr := add(srcPtr, _1)
//             pos := add(pos, 0x60)
//         }
//         let _5 := mload(64)
//         let _6 := mload(_1)
//         if slt(sub(_5, _6), 128)
//         {
//             revert(_2, _2)
//         }
//         let offset := calldataload(add(_6, 64))
//         let _7 := 0xffffffffffffffff
//         if gt(offset, _7)
//         {
//             revert(_2, _2)
//         }
//         let value2 := abi_decode_t_array$_t_uint256_$dyn_memory_ptr(add(_6, offset), _5)
//         let offset_1 := calldataload(add(_6, 96))
//         if gt(offset_1, _7)
//         {
//             revert(_2, _2)
//         }
//         let value3 := abi_decode_t_array$_t_array$_t_uint256_$2_memory_$dyn_memory_ptr(add(_6, offset_1), _5)
//         sstore(calldataload(_6), calldataload(add(_6, _1)))
//         sstore(value2, value3)
//         sstore(_2, pos)
//     }
//     function abi_decode_t_array$_t_array$_t_uint256_$2_memory_$dyn_memory_ptr(offset, end) -> array
//     {
//         if iszero(slt(add(offset, 0x1f), end))
//         {
//             revert(array, array)
//         }
//         let length := calldataload(offset)
//         array := allocateMemory(array_allocation_size_t_array$_t_address_$dyn_memory(length))
//         let dst := array
//         mstore(array, length)
//         let _1 := 0x20
//         dst := add(array, _1)
//         let src := add(offset, _1)
//         if gt(add(add(offset, mul(length, 0x40)), _1), end)
//         {
//             revert(0, 0)
//         }
//         let i := 0
//         for {
//         }
//         lt(i, length)
//         {
//             i := add(i, 1)
//         }
//         {
//             if iszero(slt(add(src, 0x1f), end))
//             {
//                 revert(0, 0)
//             }
//             let dst_1 := allocateMemory(array_allocation_size_t_array$_t_uint256_$2_memory(0x2))
//             let dst_2 := dst_1
//             let src_1 := src
//             let _2 := add(src, 0x40)
//             if gt(_2, end)
//             {
//                 revert(0, 0)
//             }
//             let i_1 := 0
//             for {
//             }
//             lt(i_1, 0x2)
//             {
//                 i_1 := add(i_1, 1)
//             }
//             {
//                 mstore(dst_1, calldataload(src_1))
//                 dst_1 := add(dst_1, _1)
//                 src_1 := add(src_1, _1)
//             }
//             mstore(dst, dst_2)
//             dst := add(dst, _1)
//             src := _2
//         }
//     }
//     function abi_decode_t_array$_t_uint256_$dyn_memory_ptr(offset, end) -> array
//     {
//         if iszero(slt(add(offset, 0x1f), end))
//         {
//             revert(array, array)
//         }
//         let length := calldataload(offset)
//         array := allocateMemory(array_allocation_size_t_array$_t_address_$dyn_memory(length))
//         let dst := array
//         mstore(array, length)
//         let _1 := 0x20
//         dst := add(array, _1)
//         let src := add(offset, _1)
//         if gt(add(add(offset, mul(length, _1)), _1), end)
//         {
//             revert(0, 0)
//         }
//         let i := 0
//         for {
//         }
//         lt(i, length)
//         {
//             i := add(i, 1)
//         }
//         {
//             mstore(dst, calldataload(src))
//             dst := add(dst, _1)
//             src := add(src, _1)
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
//     function array_allocation_size_t_array$_t_address_$dyn_memory(length) -> size
//     {
//         if gt(length, 0xffffffffffffffff)
//         {
//             revert(0, 0)
//         }
//         size := add(mul(length, 0x20), 0x20)
//     }
//     function array_allocation_size_t_array$_t_uint256_$2_memory(length) -> size
//     {
//         if gt(length, 0xffffffffffffffff)
//         {
//             revert(0, 0)
//         }
//         size := mul(length, 0x20)
//     }
// }
