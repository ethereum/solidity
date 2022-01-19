{
    {
        let _1 := 64
        mstore(_1, 128)
        if iszero(lt(calldatasize(), 4))
        {
            let _2 := 0
            switch shr(224, calldataload(_2))
            case 0x37fe974a {
                if callvalue() { revert(_2, _2) }
                let ret := fun_computeLiquidity(abi_decode_tuple_address(calldatasize()))
                let memPos := mload(_1)
                return(memPos, sub(abi_encode_tuple_struct_LiquidityStatus(memPos, ret), memPos))
            }
            case 0x41976e09 {
                if callvalue() { revert(_2, _2) }
                let ret_1, ret_2 := fun_getPrice(abi_decode_tuple_address(calldatasize()))
                let memPos_1 := mload(_1)
                return(memPos_1, sub(abi_encode_uint256_uint256(memPos_1, ret_1, ret_2), memPos_1))
            }
            case 0x69a92ea3 {
                if callvalue() { revert(_2, _2) }
                abi_decode_12552(calldatasize())
                let memPos_2 := mload(_1)
                return(memPos_2, sub(abi_encode_bytes32(memPos_2, loadimmutable("3064")), memPos_2))
            }
            case 0x8eddcd92 {
                if callvalue() { revert(_2, _2) }
                let ret_3 := fun_getNewMarketParameters(abi_decode_tuple_address(calldatasize()))
                let memPos_3 := mload(_1)
                return(memPos_3, sub(abi_encode_struct_NewMarketParameters(memPos_3, ret_3), memPos_3))
            }
            case 0x9693fa6b {
                if callvalue() { revert(_2, _2) }
                let ret_4, ret_5, ret_6 := fun_getPriceFull(abi_decode_tuple_address(calldatasize()))
                let memPos_4 := mload(_1)
                return(memPos_4, sub(abi_encode_uint256_uint256_uint256(memPos_4, ret_4, ret_5, ret_6), memPos_4))
            }
            case 0xa1308f27 {
                if callvalue() { revert(_2, _2) }
                abi_decode_12552(calldatasize())
                let memPos_5 := mload(_1)
                return(memPos_5, sub(abi_encode_bytes32(memPos_5, loadimmutable("3062")), memPos_5))
            }
            case 0xc39b543a {
                if callvalue() { revert(_2, _2) }
                fun_requireLiquidity(abi_decode_tuple_address(calldatasize()))
                return(mload(_1), _2)
            }
            case 0xebd7ba19 {
                if callvalue() { revert(_2, _2) }
                let ret_7 := fun_computeAssetLiquidities(abi_decode_tuple_address(calldatasize()))
                let memPos_6 := mload(_1)
                return(memPos_6, sub(abi_encode_array_struct_AssetLiquidity_dyn(memPos_6, ret_7), memPos_6))
            }
            case 0xf70c2fde {
                if callvalue() { revert(_2, _2) }
                let param, param_1 := abi_decode_struct_AssetCachet_struct_AssetConfig(calldatasize())
                let var_twap, var_twapPeriod := modifier_FREEMEM(param, param_1)
                let memPos_7 := mload(_1)
                return(memPos_7, sub(abi_encode_uint256_uint256(memPos_7, var_twap, var_twapPeriod), memPos_7))
            }
        }
        revert(0, 0)
    }
    function cleanup_address(value) -> cleaned
    {
        cleaned := and(value, sub(shl(160, 1), 1))
    }
    function validator_revert_address(value)
    {
        if iszero(eq(value, and(value, sub(shl(160, 1), 1)))) { revert(0, 0) }
    }
    function abi_decode_address() -> value
    {
        value := calldataload(4)
        validator_revert_address(value)
    }
    function abi_decode_tuple_address(dataEnd) -> value0
    {
        if slt(add(dataEnd, not(3)), 32) { revert(0, 0) }
        let value := calldataload(4)
        validator_revert_address(value)
        value0 := value
    }
    function cleanup_bool(value) -> cleaned
    {
        cleaned := iszero(iszero(value))
    }
    function abi_encode_struct_LiquidityStatus(value, pos)
    {
        mstore(pos, mload(value))
        mstore(add(pos, 0x20), mload(add(value, 0x20)))
        mstore(add(pos, 0x40), mload(add(value, 0x40)))
        mstore(add(pos, 0x60), iszero(iszero(mload(add(value, 0x60)))))
    }
    function abi_encode_tuple_struct_LiquidityStatus(headStart, value0) -> tail
    {
        tail := add(headStart, 128)
        abi_encode_struct_LiquidityStatus(value0, headStart)
    }
    function abi_encode_uint256_uint256(headStart, value0, value1) -> tail
    {
        tail := add(headStart, 64)
        mstore(headStart, value0)
        mstore(add(headStart, 32), value1)
    }
    function abi_decode_12552(dataEnd)
    {
        if slt(add(dataEnd, not(3)), 0) { revert(0, 0) }
    }
    function abi_decode(headStart, dataEnd)
    {
        if slt(sub(dataEnd, headStart), 0) { revert(0, 0) }
    }
    function abi_encode_bytes32(headStart, value0) -> tail
    {
        tail := add(headStart, 32)
        mstore(headStart, value0)
    }
    function cleanup_uint32(value) -> cleaned
    {
        cleaned := and(value, 0xffffffff)
    }
    function cleanup_uint24(value) -> cleaned
    {
        cleaned := and(value, 0xffffff)
    }
    function abi_encode_struct_NewMarketParameters(headStart, value0) -> tail
    {
        tail := add(headStart, 224)
        mstore(headStart, and(mload(value0), 0xffff))
        let _1 := 0xffffffff
        mstore(add(headStart, 0x20), and(mload(add(value0, 0x20)), _1))
        let memberValue0 := mload(add(value0, 0x40))
        mstore(add(headStart, 0x40), and(mload(memberValue0), sub(shl(160, 1), 1)))
        mstore(add(headStart, 96), iszero(iszero(mload(add(memberValue0, 0x20)))))
        mstore(add(headStart, 128), and(mload(add(memberValue0, 0x40)), _1))
        mstore(add(headStart, 160), and(mload(add(memberValue0, 96)), _1))
        mstore(add(headStart, 192), and(mload(add(memberValue0, 128)), 0xffffff))
    }
    function abi_encode_uint256_uint256_uint256(headStart, value0, value1, value2) -> tail
    {
        tail := add(headStart, 96)
        mstore(headStart, value0)
        mstore(add(headStart, 32), value1)
        mstore(add(headStart, 64), value2)
    }
    function abi_encode_array_struct_AssetLiquidity_dyn(headStart, value0) -> tail
    {
        let _1 := 32
        let tail_1 := add(headStart, _1)
        mstore(headStart, _1)
        let pos := tail_1
        let length := mload(value0)
        mstore(tail_1, length)
        pos := add(headStart, 64)
        let srcPtr := add(value0, _1)
        let i := 0
        for { } lt(i, length) { i := add(i, 1) }
        {
            let _2 := mload(srcPtr)
            mstore(pos, and(mload(_2), sub(shl(160, 1), 1)))
            let memberValue0 := mload(add(_2, _1))
            abi_encode_struct_LiquidityStatus(memberValue0, add(pos, _1))
            pos := add(pos, 160)
            srcPtr := add(srcPtr, _1)
        }
        tail := pos
    }
    function panic_error_0x41()
    {
        mstore(0, shl(224, 0x4e487b71))
        mstore(4, 0x41)
        revert(0, 0x24)
    }
    function finalize_allocation_12563(memPtr)
    {
        let newFreePtr := add(memPtr, 160)
        if or(gt(newFreePtr, 0xffffffffffffffff), lt(newFreePtr, memPtr)) { panic_error_0x41() }
        mstore(64, newFreePtr)
    }
    function finalize_allocation_12670(memPtr)
    {
        let newFreePtr := add(memPtr, 64)
        if or(gt(newFreePtr, 0xffffffffffffffff), lt(newFreePtr, memPtr)) { panic_error_0x41() }
        mstore(64, newFreePtr)
    }
    function finalize_allocation_22851(memPtr)
    {
        let newFreePtr := add(memPtr, 128)
        if or(gt(newFreePtr, 0xffffffffffffffff), lt(newFreePtr, memPtr)) { panic_error_0x41() }
        mstore(64, newFreePtr)
    }
    function finalize_allocation_22852(memPtr)
    {
        let newFreePtr := add(memPtr, 96)
        if or(gt(newFreePtr, 0xffffffffffffffff), lt(newFreePtr, memPtr)) { panic_error_0x41() }
        mstore(64, newFreePtr)
    }
    function finalize_allocation(memPtr, size)
    {
        let newFreePtr := add(memPtr, and(add(size, 31), not(31)))
        if or(gt(newFreePtr, 0xffffffffffffffff), lt(newFreePtr, memPtr)) { panic_error_0x41() }
        mstore(64, newFreePtr)
    }
    function allocate_memory_12562() -> memPtr
    {
        memPtr := mload(64)
        let newFreePtr := add(memPtr, 0x01e0)
        if or(gt(newFreePtr, 0xffffffffffffffff), lt(newFreePtr, memPtr)) { panic_error_0x41() }
        mstore(64, newFreePtr)
    }
    function allocate_memory() -> memPtr
    {
        memPtr := mload(64)
        finalize_allocation_22851(memPtr)
    }
    function cleanup_uint112(value) -> cleaned
    {
        cleaned := and(value, 0xffffffffffffffffffffffffffff)
    }
    function abi_decode_uint112() -> value
    {
        value := calldataload(36)
        if iszero(eq(value, and(value, 0xffffffffffffffffffffffffffff))) { revert(0, 0) }
    }
    function cleanup_uint144(value) -> cleaned
    {
        cleaned := and(value, 0xffffffffffffffffffffffffffffffffffff)
    }
    function abi_decode_uint144() -> value
    {
        value := calldataload(68)
        if iszero(eq(value, and(value, 0xffffffffffffffffffffffffffffffffffff))) { revert(0, 0) }
    }
    function cleanup_uint96(value) -> cleaned
    {
        cleaned := and(value, 0xffffffffffffffffffffffff)
    }
    function abi_decode_uint96() -> value
    {
        value := calldataload(100)
        if iszero(eq(value, and(value, 0xffffffffffffffffffffffff))) { revert(0, 0) }
    }
    function cleanup_uint40(value) -> cleaned
    {
        cleaned := and(value, 0xffffffffff)
    }
    function abi_decode_uint40() -> value
    {
        value := calldataload(164)
        if iszero(eq(value, and(value, 0xffffffffff))) { revert(0, 0) }
    }
    function validator_revert_uint8(value)
    {
        if iszero(eq(value, and(value, 0xff))) { revert(0, 0) }
    }
    function abi_decode_uint8() -> value
    {
        value := calldataload(196)
        validator_revert_uint8(value)
    }
    function validator_revert_uint32(value)
    {
        if iszero(eq(value, and(value, 0xffffffff))) { revert(0, 0) }
    }
    function abi_decode_uint32_22844() -> value
    {
        value := calldataload(228)
        validator_revert_uint32(value)
    }
    function abi_decode_uint32_22846() -> value
    {
        value := calldataload(292)
        validator_revert_uint32(value)
    }
    function abi_decode_uint32() -> value
    {
        value := calldataload(356)
        validator_revert_uint32(value)
    }
    function cleanup_int96(value) -> cleaned
    {
        cleaned := signextend(11, value)
    }
    function abi_decode_int96() -> value
    {
        value := calldataload(260)
        if iszero(eq(value, signextend(11, value))) { revert(0, 0) }
    }
    function validator_revert_uint16(value)
    {
        if iszero(eq(value, and(value, 0xffff))) { revert(0, 0) }
    }
    function abi_decode_uint16() -> value
    {
        value := calldataload(324)
        validator_revert_uint16(value)
    }
    function validator_revert_bool(value)
    {
        if iszero(eq(value, iszero(iszero(value)))) { revert(0, 0) }
    }
    function abi_decode_struct_AssetConfig(end) -> value
    {
        if slt(add(end, not(483)), 0xa0) { revert(0, 0) }
        let memPtr := mload(64)
        let newFreePtr := add(memPtr, 0xa0)
        if or(gt(newFreePtr, 0xffffffffffffffff), lt(newFreePtr, memPtr)) { panic_error_0x41() }
        mstore(64, newFreePtr)
        value := memPtr
        let value_1 := calldataload(484)
        validator_revert_address(value_1)
        mstore(memPtr, value_1)
        let value_2 := calldataload(516)
        validator_revert_bool(value_2)
        mstore(add(memPtr, 32), value_2)
        let value_3 := calldataload(548)
        validator_revert_uint32(value_3)
        mstore(add(memPtr, 64), value_3)
        let value_4 := calldataload(580)
        validator_revert_uint32(value_4)
        mstore(add(memPtr, 96), value_4)
        let value_5 := calldataload(612)
        if iszero(eq(value_5, and(value_5, 0xffffff))) { revert(0, 0) }
        mstore(add(memPtr, 128), value_5)
    }
    function abi_decode_struct_AssetCachet_struct_AssetConfig(dataEnd) -> value0, value1
    {
        let _1 := add(dataEnd, not(3))
        if slt(_1, 640) { revert(0, 0) }
        if slt(_1, 0x01e0) { revert(0, 0) }
        let value := allocate_memory_12562()
        mstore(value, abi_decode_address())
        mstore(add(value, 32), abi_decode_uint112())
        mstore(add(value, 64), abi_decode_uint144())
        mstore(add(value, 96), abi_decode_uint96())
        mstore(add(value, 128), calldataload(132))
        mstore(add(value, 160), abi_decode_uint40())
        mstore(add(value, 192), abi_decode_uint8())
        mstore(add(value, 224), abi_decode_uint32_22844())
        mstore(add(value, 256), abi_decode_int96())
        mstore(add(value, 288), abi_decode_uint32_22846())
        mstore(add(value, 320), abi_decode_uint16())
        mstore(add(value, 352), abi_decode_uint32())
        mstore(add(value, 384), calldataload(388))
        mstore(add(value, 416), calldataload(420))
        mstore(add(value, 448), calldataload(452))
        value0 := value
        value1 := abi_decode_struct_AssetConfig(dataEnd)
    }
    function allocate_and_zero_memory_struct_struct_AssetConfig() -> memPtr
    {
        let memPtr_1 := mload(64)
        finalize_allocation_12563(memPtr_1)
        memPtr := memPtr_1
        mstore(memPtr_1, 0)
        mstore(add(memPtr_1, 32), 0)
        mstore(add(memPtr_1, 64), 0)
        mstore(add(memPtr_1, 96), 0)
        mstore(add(memPtr_1, 128), 0)
    }
    function allocate_and_zero_memory_struct_struct_NewMarketParameters() -> memPtr
    {
        let memPtr_1 := mload(64)
        let newFreePtr := add(memPtr_1, 96)
        if or(gt(newFreePtr, 0xffffffffffffffff), lt(newFreePtr, memPtr_1)) { panic_error_0x41() }
        mstore(64, newFreePtr)
        memPtr := memPtr_1
        mstore(memPtr_1, 0)
        mstore(add(memPtr_1, 32), 0)
        mstore(add(memPtr_1, 64), allocate_and_zero_memory_struct_struct_AssetConfig())
    }
    function write_to_memory_bool(memPtr)
    { mstore(memPtr, 0x01) }
    function write_to_memory_uint32_12567(memPtr)
    { mstore(memPtr, 0xffffffff) }
    function write_to_memory_uint32_12631(memPtr)
    { mstore(memPtr, 0x42c1d800) }
    function write_to_memory_uint32(memPtr, value)
    {
        mstore(memPtr, and(value, 0xffffffff))
    }
    function write_to_memory_uint24_12568(memPtr)
    { mstore(memPtr, 16777215) }
    function write_to_memory_uint24_12571(memPtr)
    { mstore(memPtr, 3000) }
    function write_to_memory_uint24_12572(memPtr)
    { mstore(memPtr, 10000) }
    function write_to_memory_uint24_12573(memPtr)
    { mstore(memPtr, 500) }
    function write_to_memory_uint24_12574(memPtr)
    { mstore(memPtr, 100) }
    function write_to_memory_uint24(memPtr)
    { mstore(memPtr, 1800) }
    function mapping_index_access_mapping_address_address_of_address_12569(key) -> dataSlot
    {
        mstore(0, and(key, sub(shl(160, 1), 1)))
        mstore(0x20, 0x0b)
        dataSlot := keccak256(0, 0x40)
    }
    function mapping_index_access_mapping_address_address_of_address_12580(key) -> dataSlot
    {
        mstore(0, and(key, sub(shl(160, 1), 1)))
        mstore(0x20, 0x08)
        dataSlot := keccak256(0, 0x40)
    }
    function mapping_index_access_mapping_address_address_of_address_12628(key) -> dataSlot
    {
        mstore(0, and(key, sub(shl(160, 1), 1)))
        mstore(0x20, 0x09)
        dataSlot := keccak256(0, 0x40)
    }
    function mapping_index_access_mapping_address_address_of_address_12656(key) -> dataSlot
    {
        mstore(0, and(key, sub(shl(160, 1), 1)))
        mstore(0x20, 0x07)
        dataSlot := keccak256(0, 0x40)
    }
    function mapping_index_access_mapping_address_address_of_address(slot, key) -> dataSlot
    {
        mstore(0, and(key, sub(shl(160, 1), 1)))
        mstore(0x20, slot)
        dataSlot := keccak256(0, 0x40)
    }
    function read_from_storage_split_offset_address(slot) -> value
    {
        value := and(sload(slot), sub(shl(160, 1), 1))
    }
    function panic_error_0x11()
    {
        mstore(0, shl(224, 0x4e487b71))
        mstore(4, 0x11)
        revert(0, 0x24)
    }
    function increment_uint256(value) -> ret
    {
        if eq(value, not(0)) { panic_error_0x11() }
        ret := add(value, 1)
    }
    function panic_error_0x32()
    {
        mstore(0, shl(224, 0x4e487b71))
        mstore(4, 0x32)
        revert(0, 0x24)
    }
    function memory_array_index_access_uint24(baseRef, index) -> addr
    {
        if iszero(lt(index, 0x04)) { panic_error_0x32() }
        addr := add(baseRef, shl(5, index))
    }
    function read_from_memoryt_uint24(ptr) -> returnValue
    {
        returnValue := and(mload(ptr), 0xffffff)
    }
    function abi_decode_address_fromMemory(headStart, dataEnd) -> value0
    {
        if slt(sub(dataEnd, headStart), 32) { revert(0, 0) }
        let value := mload(headStart)
        validator_revert_address(value)
        value0 := value
    }
    function abi_encode_address_address_uint24(headStart, value0, value1, value2) -> tail
    {
        tail := add(headStart, 96)
        let _1 := sub(shl(160, 1), 1)
        mstore(headStart, and(value0, _1))
        mstore(add(headStart, 32), and(value1, _1))
        mstore(add(headStart, 64), and(value2, 0xffffff))
    }
    function revert_forward()
    {
        let pos := mload(64)
        returndatacopy(pos, 0, returndatasize())
        revert(pos, returndatasize())
    }
    function abi_decode_uint128_fromMemory(headStart, dataEnd) -> value0
    {
        if slt(sub(dataEnd, headStart), 32) { revert(0, 0) }
        let value := mload(headStart)
        if iszero(eq(value, and(value, 0xffffffffffffffffffffffffffffffff))) { revert(0, 0) }
        value0 := value
    }
    function require_helper_stringliteral_15dc(condition)
    {
        if iszero(condition)
        {
            let memPtr := mload(64)
            mstore(memPtr, shl(229, 4594637))
            mstore(add(memPtr, 4), 32)
            mstore(add(memPtr, 36), 23)
            mstore(add(memPtr, 68), "e/no-uniswap-pool-avail")
            revert(memPtr, 100)
        }
    }
    function require_helper_stringliteral_cad5(condition)
    {
        if iszero(condition)
        {
            let memPtr := mload(64)
            mstore(memPtr, shl(229, 4594637))
            mstore(add(memPtr, 4), 32)
            mstore(add(memPtr, 36), 23)
            mstore(add(memPtr, 68), "e/bad-uniswap-pool-addr")
            revert(memPtr, 100)
        }
    }
    function write_to_memory_uint16_12575(memPtr)
    { mstore(memPtr, 2) }
    function write_to_memory_uint16_12577(memPtr)
    { mstore(memPtr, 3) }
    function write_to_memory_uint16_12581(memPtr)
    { mstore(memPtr, 1) }
    function write_to_memory_uint16(memPtr, value)
    {
        mstore(memPtr, and(value, 0xffff))
    }
    function abi_encode_uint16_to_uint16(headStart) -> tail
    {
        tail := add(headStart, 32)
        mstore(headStart, 10)
    }
    function abi_encode_uint16(headStart, value0) -> tail
    {
        tail := add(headStart, 32)
        mstore(headStart, and(value0, 0xffff))
    }
    function return_data_selector() -> sig
    {
        if gt(returndatasize(), 3)
        {
            returndatacopy(0, 0, 4)
            sig := shr(224, mload(0))
        }
    }
    function try_decode_error_message() -> ret
    {
        if lt(returndatasize(), 0x44) { leave }
        let data := mload(64)
        let _1 := not(3)
        returndatacopy(data, 4, add(returndatasize(), _1))
        let offset := mload(data)
        let _2 := returndatasize()
        let _3 := 0xffffffffffffffff
        if or(gt(offset, _3), gt(add(offset, 0x24), _2)) { leave }
        let msg := add(data, offset)
        let length := mload(msg)
        if gt(length, _3) { leave }
        if gt(add(add(msg, length), 0x20), add(add(data, returndatasize()), _1)) { leave }
        finalize_allocation(data, add(add(offset, length), 0x20))
        ret := msg
    }
    function abi_encode_stringliteral(headStart) -> tail
    {
        mstore(headStart, 32)
        mstore(add(headStart, 32), 30)
        mstore(add(headStart, 64), "e/risk/uniswap-pool-not-inited")
        tail := add(headStart, 96)
    }
    function abi_encode_stringliteral_92b3(pos) -> end
    {
        mstore(pos, "e/risk/uniswap/")
        end := add(pos, 15)
    }
    function copy_memory_to_memory(src, dst, length)
    {
        let i := 0
        for { } lt(i, length) { i := add(i, 32) }
        {
            mstore(add(dst, i), mload(add(src, i)))
        }
        if gt(i, length) { mstore(add(dst, length), 0) }
    }
    function abi_encode_string_memory_ptr(value, pos) -> end
    {
        let length := mload(value)
        copy_memory_to_memory(add(value, 0x20), pos, length)
        end := add(pos, length)
    }
    function abi_encode_string(headStart, value0) -> tail
    {
        mstore(headStart, 32)
        let length := mload(value0)
        mstore(add(headStart, 32), length)
        copy_memory_to_memory(add(value0, 32), add(headStart, 64), length)
        tail := add(add(headStart, and(add(length, 31), not(31))), 64)
    }
    function extract_returndata() -> data
    {
        switch returndatasize()
        case 0 { data := 96 }
        default {
            let _1 := returndatasize()
            if gt(_1, 0xffffffffffffffff) { panic_error_0x41() }
            let memPtr := mload(64)
            finalize_allocation(memPtr, add(and(add(_1, 31), not(31)), 0x20))
            mstore(memPtr, _1)
            data := memPtr
            returndatacopy(add(memPtr, 0x20), 0, returndatasize())
        }
    }
    function read_from_storage_split_offset_uint32(slot) -> value
    {
        value := and(shr(168, sload(slot)), 0xffffffff)
    }
    /// @ast-id 10602
    function fun_getNewMarketParameters(var_underlying) -> var_p_mpos
    {
        var_p_mpos := allocate_and_zero_memory_struct_struct_NewMarketParameters()
        let _1 := 64
        let _2 := add(var_p_mpos, _1)
        let _3 := 32
        write_to_memory_bool(add(mload(_2), _3))
        let _4 := 0
        mstore(add(mload(_2), _1), _4)
        write_to_memory_uint32_12567(add(mload(_2), 96))
        write_to_memory_uint24_12568(add(mload(_2), 128))
        let _5 := loadimmutable("10268")
        let _6 := sub(shl(160, 1), 1)
        switch eq(and(var_underlying, _6), and(_5, _6))
        case 0 {
            switch iszero(iszero(and(read_from_storage_split_offset_address(mapping_index_access_mapping_address_address_of_address_12569(var_underlying)), _6)))
            case 0 {
                let var_pool := _4
                let var_fee := _4
                let expr_10439_mpos := allocate_memory()
                write_to_memory_uint24_12571(expr_10439_mpos)
                write_to_memory_uint24_12572(add(expr_10439_mpos, _3))
                write_to_memory_uint24_12573(add(expr_10439_mpos, _1))
                write_to_memory_uint24_12574(add(expr_10439_mpos, 96))
                let var_bestLiquidity := _4
                let var_i := _4
                let _7 := and(loadimmutable("10270"), _6)
                for { }
                1
                {
                    var_i := increment_uint256(var_i)
                }
                {
                    let _8 := 4
                    if iszero(lt(var_i, _8)) { break }
                    let _9 := read_from_memoryt_uint24(memory_array_index_access_uint24(expr_10439_mpos, var_i))
                    let _10 := mload(_1)
                    mstore(_10, shl(225, 0x0b4c7741))
                    let _11 := staticcall(gas(), _7, _10, sub(abi_encode_address_address_uint24(add(_10, _8), var_underlying, _5, _9), _10), _10, _3)
                    if iszero(_11) { revert_forward() }
                    let expr := _4
                    if _11
                    {
                        finalize_allocation(_10, returndatasize())
                        expr := abi_decode_address_fromMemory(_10, add(_10, returndatasize()))
                    }
                    let _12 := and(expr, _6)
                    if iszero(_12) { continue }
                    let _13 := mload(_1)
                    mstore(_13, shl(225, 0x0d343281))
                    let _14 := staticcall(gas(), _12, _13, _8, _13, _3)
                    if iszero(_14) { revert_forward() }
                    let expr_1 := _4
                    if _14
                    {
                        finalize_allocation(_13, returndatasize())
                        expr_1 := abi_decode_uint128_fromMemory(_13, add(_13, returndatasize()))
                    }
                    let expr_2 := iszero(and(var_pool, _6))
                    if iszero(expr_2)
                    {
                        let _15 := 0xffffffffffffffffffffffffffffffff
                        expr_2 := gt(and(expr_1, _15), and(var_bestLiquidity, _15))
                    }
                    if expr_2
                    {
                        var_pool := expr
                        var_fee := read_from_memoryt_uint24(memory_array_index_access_uint24(expr_10439_mpos, var_i))
                        var_bestLiquidity := expr_1
                    }
                }
                let _16 := and(var_pool, _6)
                require_helper_stringliteral_15dc(iszero(iszero(_16)))
                require_helper_stringliteral_cad5(eq(cleanup_address(fun_computeUniswapPoolAddress(var_underlying, var_fee)), _16))
                write_to_memory_uint16_12575(var_p_mpos)
                write_to_memory_uint32(add(var_p_mpos, _3), and(var_fee, 16777215))
                if iszero(extcodesize(_16)) { revert(_4, _4) }
                let _17 := mload(_1)
                mstore(_17, shl(224, 0x32148f67))
                let trySuccessCondition := call(gas(), _16, _4, _17, sub(abi_encode_uint16_to_uint16(add(_17, 4)), _17), _17, _4)
                if trySuccessCondition
                {
                    finalize_allocation(_17, returndatasize())
                    abi_decode(_17, add(_17, returndatasize()))
                }
                switch iszero(trySuccessCondition)
                case 0 { }
                default {
                    let _18 := 1
                    if eq(147028384, return_data_selector())
                    {
                        let _19 := try_decode_error_message()
                        if _19
                        {
                            _18 := _4
                            if eq(keccak256(add(_19, _3), mload(_19)), 0xe01ebc6b01bbf458b3d355b6e649efe64599751670c5d19175619893ecf97529)
                            {
                                let _20 := mload(_1)
                                mstore(_20, shl(229, 4594637))
                                revert(_20, sub(abi_encode_stringliteral(add(_20, 4)), _20))
                            }
                            let expr_10582_mpos := mload(_1)
                            let _21 := sub(abi_encode_string_memory_ptr(_19, abi_encode_stringliteral_92b3(add(expr_10582_mpos, _3))), expr_10582_mpos)
                            mstore(expr_10582_mpos, add(_21, not(31)))
                            finalize_allocation(expr_10582_mpos, _21)
                            let _22 := mload(_1)
                            mstore(_22, shl(229, 4594637))
                            revert(_22, sub(abi_encode_string(add(_22, 4), expr_10582_mpos), _22))
                        }
                    }
                    if _18
                    {
                        fun_revertBytes(extract_returndata())
                    }
                }
            }
            default {
                write_to_memory_uint16_12577(var_p_mpos)
                mstore(add(var_p_mpos, _3), _4)
                write_to_memory_uint32(add(mload(_2), _1), read_from_storage_split_offset_uint32(mapping_index_access_mapping_address_address_of_address_12580(read_from_storage_split_offset_address(mapping_index_access_mapping_address_address_of_address_12569(var_underlying)))))
            }
        }
        default {
            write_to_memory_uint16_12581(var_p_mpos)
            mstore(add(var_p_mpos, _3), _4)
        }
    }
    /// @ast-id 164
    function fun_revertBytes(var_errMsg_mpos)
    {
        let expr := mload(var_errMsg_mpos)
        if iszero(iszero(expr))
        {
            revert(add(32, var_errMsg_mpos), expr)
        }
        let _1 := mload(64)
        mstore(_1, shl(229, 4594637))
        mstore(add(_1, 4), 32)
        mstore(add(_1, 36), 13)
        mstore(add(_1, 68), "e/empty-error")
        revert(_1, 100)
    }
    /// @ast-id 10658
    function fun_computeUniswapPoolAddress(var_underlying, var_fee) -> var
    {
        let var_tokenA := var_underlying
        let var_tokenB := loadimmutable("10268")
        let var_tokenB_1 := var_tokenB
        let _1 := sub(shl(160, 1), 1)
        if gt(and(var_underlying, _1), and(var_tokenB, _1))
        {
            var_tokenB := var_underlying
            var_tokenA := var_tokenB_1
        }
        let expr_10648_mpos := mload(64)
        let _2 := add(expr_10648_mpos, 0x20)
        let _3 := sub(abi_encode_address_address_uint24(_2, var_tokenA, var_tokenB, var_fee), expr_10648_mpos)
        mstore(expr_10648_mpos, add(_3, not(31)))
        finalize_allocation(expr_10648_mpos, _3)
        let expr := keccak256(_2, mload(expr_10648_mpos))
        let expr_10651_mpos := mload(64)
        let _4 := add(expr_10651_mpos, 0x20)
        mstore(_4, shl(248, 255))
        mstore(add(expr_10651_mpos, 33), and(shl(96, loadimmutable("10270")), not(0xffffffffffffffffffffffff)))
        mstore(add(expr_10651_mpos, 53), expr)
        mstore(add(expr_10651_mpos, 85), loadimmutable("10272"))
        mstore(expr_10651_mpos, 85)
        finalize_allocation_22851(expr_10651_mpos)
        var := cleanup_address(cleanup_address(keccak256(_4, mload(expr_10651_mpos))))
    }
    /// @ast-id 147
    function modifier_FREEMEM(var_assetCache_mpos, var_config_mpos) -> _1, _2
    {
        let _3 := 0x40
        let var_origFreeMemPtr := mload(_3)
        let var_twap := 0
        let var_twapPeriod := var_twap
        var_twap := var_twap
        var_twapPeriod := 0
        let expr_component, expr_component_1, expr_component_2, expr_component_3 := fun_resolvePricingConfig(var_assetCache_mpos, var_config_mpos)
        let _4 := 0xffff
        let _5 := and(expr_component_1, _4)
        switch eq(_5, 0x01)
        case 0 {
            switch eq(_5, 0x02)
            case 0 {
                let _6 := mload(_3)
                mstore(_6, shl(229, 4594637))
                revert(_6, sub(abi_encode_stringliteral_ee51(add(_6, 4)), _6))
            }
            default {
                let _7 := 0xffffff
                let expr := fun_computeUniswapPoolAddress(expr_component, and(expr_component_2, _7))
                let var_ago := and(expr_component_3, _7)
                let expr_mpos := allocate_and_zero_memory_array_array_uint32_dyn()
                write_to_memory_uint32(memory_array_index_access_uint32_dyn_12584(expr_mpos), var_ago)
                mstore(memory_array_index_access_uint32_dyn_12585(expr_mpos), var_twapPeriod)
                let expr_mpos_1 := mload(_3)
                let _8 := add(expr_mpos_1, 0x20)
                let _9 := shl(224, 0x883bdbfd)
                mstore(_8, _9)
                let _10 := sub(abi_encode_array_uint32_dyn(add(expr_mpos_1, 36), expr_mpos), expr_mpos_1)
                let _11 := not(31)
                mstore(expr_mpos_1, add(_10, _11))
                finalize_allocation(expr_mpos_1, _10)
                let expr_component_4 := staticcall(gas(), expr, _8, mload(expr_mpos_1), var_twapPeriod, var_twapPeriod)
                let var_data_mpos := extract_returndata()
                if iszero(expr_component_4)
                {
                    let expr_1 := keccak256(add(var_data_mpos, 0x20), mload(var_data_mpos))
                    let expr_mpos_2 := mload(_3)
                    let _12 := add(expr_mpos_2, 0x20)
                    mstore(_12, shl(229, 4594637))
                    let _13 := sub(abi_encode_stringliteral_d30c(add(expr_mpos_2, 36)), expr_mpos_2)
                    mstore(expr_mpos_2, add(_13, _11))
                    finalize_allocation(expr_mpos_2, _13)
                    if iszero(eq(expr_1, keccak256(_12, mload(expr_mpos_2))))
                    {
                        fun_revertBytes(var_data_mpos)
                    }
                    let _14 := and(expr, sub(shl(160, 1), 1))
                    let _15 := mload(_3)
                    mstore(_15, shl(224, 0x3850c7bd))
                    let _16 := staticcall(gas(), _14, _15, 4, _15, 224)
                    if iszero(_16) { revert_forward() }
                    let expr_component_5 := var_twapPeriod
                    let expr_component_6 := var_twapPeriod
                    let expr_component_7 := var_twapPeriod
                    if _16
                    {
                        finalize_allocation(_15, returndatasize())
                        let expr_component_8, expr_component_9, expr_component_10, expr_component_11, expr_component_12, expr_component_13, expr_component_14 := abi_decode_uint160t_int24t_uint16t_uint16t_uint16t_uint8t_bool_fromMemory(_15, add(_15, returndatasize()))
                        expr_component_5 := expr_component_10
                        expr_component_6 := expr_component_11
                        expr_component_7 := expr_component_12
                    }
                    let expr_2 := mod_uint16(checked_add_uint16(expr_component_5), expr_component_6)
                    let _17 := mload(_3)
                    let _18 := shl(224, 0x252c09d7)
                    mstore(_17, _18)
                    let _19 := staticcall(gas(), _14, _17, sub(abi_encode_uint16(add(_17, 4), expr_2), _17), _17, 128)
                    if iszero(_19) { revert_forward() }
                    let expr_component_15 := var_twapPeriod
                    let expr_component_16 := var_twapPeriod
                    if _19
                    {
                        finalize_allocation(_17, returndatasize())
                        let expr_component_17, expr_component_18, expr_component_19, expr_component_20 := abi_decode_uint32t_int56t_uint160t_bool_fromMemory(_17, add(_17, returndatasize()))
                        expr_component_15 := expr_component_17
                        expr_component_16 := expr_component_20
                    }
                    let var_oldestAvailableAge := expr_component_15
                    if iszero(expr_component_16)
                    {
                        let _20 := mload(_3)
                        mstore(_20, _18)
                        let _21 := staticcall(gas(), _14, _20, sub(abi_encode_rational_by(add(_20, 4)), _20), _20, 128)
                        if iszero(_21) { revert_forward() }
                        let expr_component_21 := var_twapPeriod
                        if _21
                        {
                            finalize_allocation(_20, returndatasize())
                            let expr_component_22, expr_component_23, expr_component_24, expr_component_25 := abi_decode_uint32t_int56t_uint160t_bool_fromMemory(_20, add(_20, returndatasize()))
                            expr_component_21 := expr_component_22
                        }
                        var_oldestAvailableAge := expr_component_21
                    }
                    let _22 := and(expr_component_6, _4)
                    let expr_3 := eq(_22, and(expr_component_7, _4))
                    if expr_3 { expr_3 := lt(_22, _4) }
                    if expr_3
                    {
                        let expr_4 := checked_add_uint16(expr_component_6)
                        if iszero(extcodesize(_14))
                        {
                            revert(var_twapPeriod, var_twapPeriod)
                        }
                        let _23 := mload(_3)
                        mstore(_23, shl(224, 0x32148f67))
                        let _24 := call(gas(), _14, var_twapPeriod, _23, sub(abi_encode_uint16(add(_23, 4), expr_4), _23), _23, var_twapPeriod)
                        if iszero(_24) { revert_forward() }
                        if _24
                        {
                            finalize_allocation(_23, returndatasize())
                            abi_decode(_23, add(_23, returndatasize()))
                        }
                    }
                    let _25 := 0xffffffff
                    var_ago := checked_sub_uint256(timestamp(), and(var_oldestAvailableAge, _25))
                    write_to_memory_uint32(memory_array_index_access_uint32_dyn_12584(expr_mpos), and(var_ago, _25))
                    let expr_mpos_3 := mload(_3)
                    let _26 := add(expr_mpos_3, 0x20)
                    mstore(_26, _9)
                    let _27 := sub(abi_encode_array_uint32_dyn(add(expr_mpos_3, 36), expr_mpos), expr_mpos_3)
                    mstore(expr_mpos_3, add(_27, _11))
                    finalize_allocation(expr_mpos_3, _27)
                    let expr_component_26 := staticcall(gas(), expr, _26, mload(expr_mpos_3), var_twapPeriod, var_twapPeriod)
                    var_data_mpos := extract_returndata()
                    if iszero(expr_component_26)
                    {
                        fun_revertBytes(var_data_mpos)
                    }
                }
                let expr_mpos_4 := abi_decode_array_int56_dyn_fromMemory(add(var_data_mpos, 0x20), add(add(var_data_mpos, mload(var_data_mpos)), 0x20))
                let _28 := read_from_memoryt_int56(memory_array_index_access_uint32_dyn_12585(expr_mpos_4))
                let expr_5 := checked_sub_int56(_28, read_from_memoryt_int56(memory_array_index_access_uint32_dyn_12584(expr_mpos_4)))
                let var := fun_decodeSqrtPriceX96(var_assetCache_mpos, cleanup_address(fun_getSqrtRatioAtTick(convert_int56_to_int24(checked_div_int56(expr_5, signextend(6, var_ago))))))
                var_twapPeriod := var_ago
                var_twap := var
            }
        }
        default {
            var_twap := 0x0de0b6b3a7640000
            var_twapPeriod := and(expr_component_3, 0xffffff)
        }
        _1 := var_twap
        _2 := var_twapPeriod
        mstore(_3, var_origFreeMemPtr)
    }
    /// @ast-id 147
    function modifier_FREEMEM_12660(var_assetCache_mpos, var_config_mpos) -> _1, _2
    {
        let _3 := 0x40
        let var_origFreeMemPtr := mload(_3)
        let var_twap := 0x00
        let var_twapPeriod := var_twap
        var_twap := var_twap
        var_twapPeriod := 0x00
        let expr_component, expr_component_1, expr_component_2, expr_component_3 := fun_resolvePricingConfig(var_assetCache_mpos, var_config_mpos)
        let _4 := 0xffff
        let _5 := and(expr_component_1, _4)
        switch eq(_5, 0x01)
        case 0 {
            switch eq(_5, 0x02)
            case 0 {
                let _6 := mload(_3)
                mstore(_6, shl(229, 4594637))
                revert(_6, sub(abi_encode_stringliteral_ee51(add(_6, 4)), _6))
            }
            default {
                let _7 := 0xffffff
                let expr := fun_computeUniswapPoolAddress(expr_component, and(expr_component_2, _7))
                let var_ago := and(expr_component_3, _7)
                let expr_mpos := allocate_and_zero_memory_array_array_uint32_dyn()
                write_to_memory_uint32(memory_array_index_access_uint32_dyn_12584(expr_mpos), var_ago)
                mstore(memory_array_index_access_uint32_dyn_12585(expr_mpos), var_twapPeriod)
                let expr_mpos_1 := mload(_3)
                let _8 := add(expr_mpos_1, 0x20)
                let _9 := shl(224, 0x883bdbfd)
                mstore(_8, _9)
                let _10 := sub(abi_encode_array_uint32_dyn(add(expr_mpos_1, 36), expr_mpos), expr_mpos_1)
                let _11 := not(31)
                mstore(expr_mpos_1, add(_10, _11))
                finalize_allocation(expr_mpos_1, _10)
                let expr_component_4 := staticcall(gas(), expr, _8, mload(expr_mpos_1), var_twapPeriod, var_twapPeriod)
                let var_data_mpos := extract_returndata()
                if iszero(expr_component_4)
                {
                    let expr_1 := keccak256(add(var_data_mpos, 0x20), mload(var_data_mpos))
                    let expr_mpos_2 := mload(_3)
                    let _12 := add(expr_mpos_2, 0x20)
                    mstore(_12, shl(229, 4594637))
                    let _13 := sub(abi_encode_stringliteral_d30c(add(expr_mpos_2, 36)), expr_mpos_2)
                    mstore(expr_mpos_2, add(_13, _11))
                    finalize_allocation(expr_mpos_2, _13)
                    if iszero(eq(expr_1, keccak256(_12, mload(expr_mpos_2))))
                    {
                        fun_revertBytes(var_data_mpos)
                    }
                    let _14 := and(expr, sub(shl(160, 1), 1))
                    let _15 := mload(_3)
                    mstore(_15, shl(224, 0x3850c7bd))
                    let _16 := staticcall(gas(), _14, _15, 4, _15, 224)
                    if iszero(_16) { revert_forward() }
                    let expr_component_5 := var_twapPeriod
                    let expr_component_6 := var_twapPeriod
                    let expr_component_7 := var_twapPeriod
                    if _16
                    {
                        finalize_allocation(_15, returndatasize())
                        let expr_component_8, expr_component_9, expr_component_10, expr_component_11, expr_component_12, expr_component_13, expr_component_14 := abi_decode_uint160t_int24t_uint16t_uint16t_uint16t_uint8t_bool_fromMemory(_15, add(_15, returndatasize()))
                        expr_component_5 := expr_component_10
                        expr_component_6 := expr_component_11
                        expr_component_7 := expr_component_12
                    }
                    let expr_2 := mod_uint16(checked_add_uint16(expr_component_5), expr_component_6)
                    let _17 := mload(_3)
                    let _18 := shl(224, 0x252c09d7)
                    mstore(_17, _18)
                    let _19 := staticcall(gas(), _14, _17, sub(abi_encode_uint16(add(_17, 4), expr_2), _17), _17, 128)
                    if iszero(_19) { revert_forward() }
                    let expr_component_15 := var_twapPeriod
                    let expr_component_16 := var_twapPeriod
                    if _19
                    {
                        finalize_allocation(_17, returndatasize())
                        let expr_component_17, expr_component_18, expr_component_19, expr_component_20 := abi_decode_uint32t_int56t_uint160t_bool_fromMemory(_17, add(_17, returndatasize()))
                        expr_component_15 := expr_component_17
                        expr_component_16 := expr_component_20
                    }
                    let var_oldestAvailableAge := expr_component_15
                    if iszero(expr_component_16)
                    {
                        let _20 := mload(_3)
                        mstore(_20, _18)
                        let _21 := staticcall(gas(), _14, _20, sub(abi_encode_rational_by(add(_20, 4)), _20), _20, 128)
                        if iszero(_21) { revert_forward() }
                        let expr_component_21 := var_twapPeriod
                        if _21
                        {
                            finalize_allocation(_20, returndatasize())
                            let expr_component_22, expr_component_23, expr_component_24, expr_component_25 := abi_decode_uint32t_int56t_uint160t_bool_fromMemory(_20, add(_20, returndatasize()))
                            expr_component_21 := expr_component_22
                        }
                        var_oldestAvailableAge := expr_component_21
                    }
                    let _22 := and(expr_component_6, _4)
                    let expr_3 := eq(_22, and(expr_component_7, _4))
                    if expr_3 { expr_3 := lt(_22, _4) }
                    if expr_3
                    {
                        let expr_4 := checked_add_uint16(expr_component_6)
                        if iszero(extcodesize(_14))
                        {
                            revert(var_twapPeriod, var_twapPeriod)
                        }
                        let _23 := mload(_3)
                        mstore(_23, shl(224, 0x32148f67))
                        let _24 := call(gas(), _14, var_twapPeriod, _23, sub(abi_encode_uint16(add(_23, 4), expr_4), _23), _23, var_twapPeriod)
                        if iszero(_24) { revert_forward() }
                        if _24
                        {
                            finalize_allocation(_23, returndatasize())
                            abi_decode(_23, add(_23, returndatasize()))
                        }
                    }
                    let _25 := 0xffffffff
                    var_ago := checked_sub_uint256(timestamp(), and(var_oldestAvailableAge, _25))
                    write_to_memory_uint32(memory_array_index_access_uint32_dyn_12584(expr_mpos), and(var_ago, _25))
                    let expr_mpos_3 := mload(_3)
                    let _26 := add(expr_mpos_3, 0x20)
                    mstore(_26, _9)
                    let _27 := sub(abi_encode_array_uint32_dyn(add(expr_mpos_3, 36), expr_mpos), expr_mpos_3)
                    mstore(expr_mpos_3, add(_27, _11))
                    finalize_allocation(expr_mpos_3, _27)
                    let expr_component_26 := staticcall(gas(), expr, _26, mload(expr_mpos_3), var_twapPeriod, var_twapPeriod)
                    var_data_mpos := extract_returndata()
                    if iszero(expr_component_26)
                    {
                        fun_revertBytes(var_data_mpos)
                    }
                }
                let expr_mpos_4 := abi_decode_array_int56_dyn_fromMemory(add(var_data_mpos, 0x20), add(add(var_data_mpos, mload(var_data_mpos)), 0x20))
                let _28 := read_from_memoryt_int56(memory_array_index_access_uint32_dyn_12585(expr_mpos_4))
                let expr_5 := checked_sub_int56(_28, read_from_memoryt_int56(memory_array_index_access_uint32_dyn_12584(expr_mpos_4)))
                let var := fun_decodeSqrtPriceX96(var_assetCache_mpos, cleanup_address(fun_getSqrtRatioAtTick(convert_int56_to_int24(checked_div_int56(expr_5, signextend(6, var_ago))))))
                var_twapPeriod := var_ago
                var_twap := var
            }
        }
        default {
            var_twap := 0x0de0b6b3a7640000
            var_twapPeriod := and(expr_component_3, 0xffffff)
        }
        _1 := var_twap
        _2 := var_twapPeriod
        mstore(_3, var_origFreeMemPtr)
    }
    function abi_encode_stringliteral_ee51(headStart) -> tail
    {
        mstore(headStart, 32)
        mstore(add(headStart, 32), 22)
        mstore(add(headStart, 64), "e/unknown-pricing-type")
        tail := add(headStart, 96)
    }
    function array_allocation_size_array_uint32_dyn(length) -> size
    {
        if gt(length, 0xffffffffffffffff) { panic_error_0x41() }
        size := add(shl(5, length), 0x20)
    }
    function allocate_and_zero_memory_array_array_uint32_dyn() -> memPtr
    {
        let memPtr_1 := mload(64)
        finalize_allocation_22852(memPtr_1)
        mstore(memPtr_1, 0x02)
        memPtr := memPtr_1
        calldatacopy(add(memPtr_1, 0x20), calldatasize(), 64)
    }
    function memory_array_index_access_uint32_dyn_12584(baseRef) -> addr
    {
        if iszero(mload(baseRef)) { panic_error_0x32() }
        addr := add(baseRef, 32)
    }
    function memory_array_index_access_uint32_dyn_12585(baseRef) -> addr
    {
        if iszero(lt(0x01, mload(baseRef))) { panic_error_0x32() }
        addr := add(baseRef, 64)
    }
    function memory_array_index_access_uint32_dyn(baseRef, index) -> addr
    {
        if iszero(lt(index, mload(baseRef))) { panic_error_0x32() }
        addr := add(add(baseRef, shl(5, index)), 32)
    }
    function abi_encode_array_uint32_dyn(headStart, value0) -> tail
    {
        let _1 := 32
        let tail_1 := add(headStart, _1)
        mstore(headStart, _1)
        let pos := tail_1
        let length := mload(value0)
        mstore(tail_1, length)
        pos := add(headStart, 64)
        let srcPtr := add(value0, _1)
        let i := 0
        for { } lt(i, length) { i := add(i, 1) }
        {
            mstore(pos, and(mload(srcPtr), 0xffffffff))
            pos := add(pos, _1)
            srcPtr := add(srcPtr, _1)
        }
        tail := pos
    }
    function abi_encode_stringliteral_d30c(headStart) -> tail
    {
        mstore(headStart, 32)
        mstore(add(headStart, 32), 3)
        mstore(add(headStart, 64), "OLD")
        tail := add(headStart, 96)
    }
    function abi_decode_uint160t_int24t_uint16t_uint16t_uint16t_uint8t_bool_fromMemory(headStart, dataEnd) -> value0, value1, value2, value3, value4, value5, value6
    {
        if slt(sub(dataEnd, headStart), 224) { revert(0, 0) }
        let value := mload(headStart)
        validator_revert_address(value)
        value0 := value
        let value_1 := mload(add(headStart, 32))
        if iszero(eq(value_1, signextend(2, value_1))) { revert(0, 0) }
        value1 := value_1
        let value_2 := mload(add(headStart, 64))
        validator_revert_uint16(value_2)
        value2 := value_2
        let value_3 := mload(add(headStart, 96))
        validator_revert_uint16(value_3)
        value3 := value_3
        let value_4 := mload(add(headStart, 128))
        validator_revert_uint16(value_4)
        value4 := value_4
        let value_5 := mload(add(headStart, 160))
        validator_revert_uint8(value_5)
        value5 := value_5
        let value_6 := mload(add(headStart, 192))
        validator_revert_bool(value_6)
        value6 := value_6
    }
    function checked_add_uint16(x) -> sum
    {
        let x_1 := and(x, 0xffff)
        if gt(x_1, 65534) { panic_error_0x11() }
        sum := add(x_1, 1)
    }
    function panic_error_0x12()
    {
        mstore(0, shl(224, 0x4e487b71))
        mstore(4, 0x12)
        revert(0, 0x24)
    }
    function mod_uint16(x, y) -> r
    {
        let _1 := 0xffff
        let y_1 := and(y, _1)
        if iszero(y_1) { panic_error_0x12() }
        r := mod(and(x, _1), y_1)
    }
    function abi_decode_int56_fromMemory(offset) -> value
    {
        value := mload(offset)
        if iszero(eq(value, signextend(6, value))) { revert(0, 0) }
    }
    function abi_decode_uint32t_int56t_uint160t_bool_fromMemory(headStart, dataEnd) -> value0, value1, value2, value3
    {
        if slt(sub(dataEnd, headStart), 128) { revert(0, 0) }
        let value := mload(headStart)
        validator_revert_uint32(value)
        value0 := value
        value1 := abi_decode_int56_fromMemory(add(headStart, 32))
        let value_1 := mload(add(headStart, 64))
        validator_revert_address(value_1)
        value2 := value_1
        let value_2 := mload(add(headStart, 96))
        validator_revert_bool(value_2)
        value3 := value_2
    }
    function abi_encode_rational_by(headStart) -> tail
    {
        tail := add(headStart, 32)
        mstore(headStart, 0)
    }
    function checked_sub_uint256(x, y) -> diff
    {
        if lt(x, y) { panic_error_0x11() }
        diff := sub(x, y)
    }
    function abi_decode_array_int56_dyn_fromMemory(headStart, dataEnd) -> value0
    {
        let _1 := 32
        if slt(sub(dataEnd, headStart), _1) { revert(0, 0) }
        let offset := mload(headStart)
        if gt(offset, 0xffffffffffffffff) { revert(0, 0) }
        let _2 := add(headStart, offset)
        if iszero(slt(add(_2, 0x1f), dataEnd)) { revert(0, 0) }
        let _3 := mload(_2)
        let _4 := array_allocation_size_array_uint32_dyn(_3)
        let memPtr := mload(64)
        finalize_allocation(memPtr, _4)
        let dst := memPtr
        mstore(memPtr, _3)
        dst := add(memPtr, _1)
        let srcEnd := add(add(_2, shl(5, _3)), _1)
        if gt(srcEnd, dataEnd) { revert(0, 0) }
        let src := add(_2, _1)
        for { } lt(src, srcEnd) { src := add(src, _1) }
        {
            mstore(dst, abi_decode_int56_fromMemory(src))
            dst := add(dst, _1)
        }
        value0 := memPtr
    }
    function read_from_memoryt_int56(ptr) -> returnValue
    {
        returnValue := signextend(6, mload(ptr))
    }
    function checked_sub_int56(x, y) -> diff
    {
        let x_1 := signextend(6, x)
        let y_1 := signextend(6, y)
        let _1 := slt(y_1, 0)
        if and(iszero(_1), slt(x_1, add(not(0x7fffffffffffff), y_1))) { panic_error_0x11() }
        if and(_1, sgt(x_1, add(0x7fffffffffffff, y_1))) { panic_error_0x11() }
        diff := sub(x_1, y_1)
    }
    function checked_div_int56(x, y) -> r
    {
        let x_1 := signextend(6, x)
        let y_1 := signextend(6, y)
        if iszero(y_1) { panic_error_0x12() }
        if and(eq(x_1, not(0x7fffffffffffff)), eq(y_1, not(0))) { panic_error_0x11() }
        r := sdiv(x_1, y_1)
    }
    function convert_int56_to_int24(value) -> converted
    {
        converted := signextend(2, value)
    }
    function read_from_memoryt_address(ptr) -> returnValue
    {
        returnValue := and(mload(ptr), sub(shl(160, 1), 1))
    }
    function checked_mul_uint256_12666(x) -> product
    {
        let _1 := 0xee6b2800
        if and(iszero(iszero(x)), gt(_1, div(not(0), x))) { panic_error_0x11() }
        product := mul(x, _1)
    }
    function checked_mul_uint256(x, y) -> product
    {
        if and(iszero(iszero(x)), gt(y, div(not(0), x))) { panic_error_0x11() }
        product := mul(x, y)
    }
    function checked_div_uint256_12624(y) -> r
    {
        if iszero(y) { panic_error_0x12() }
        r := div(shl(192, 1), y)
    }
    function checked_div_uint256_12625(y) -> r
    {
        if iszero(y) { panic_error_0x12() }
        r := div(0xc097ce7bc90715b34b9f1000000000, y)
    }
    function checked_div_uint256_12645(y) -> r
    {
        if iszero(y) { panic_error_0x12() }
        r := div(0xffffffffffffffffffffffffffff, y)
    }
    function checked_div_uint256_12649(x) -> r
    {
        r := div(x, 0x033b2e3c9fd0803ce8000000)
    }
    function checked_div_uint256_12661(x) -> r
    {
        r := div(x, 0x0de0b6b3a7640000)
    }
    function checked_div_uint256_12662(x) -> r
    { r := div(x, 0xee6b2800) }
    function checked_div_uint256_12692(y) -> r
    {
        if iszero(y) { panic_error_0x12() }
        r := div(not(0), y)
    }
    function checked_div_uint256_22855(x) -> r
    {
        r := div(x, 0x3782dace9d900000)
    }
    function checked_div_uint256(x, y) -> r
    {
        if iszero(y) { panic_error_0x12() }
        r := div(x, y)
    }
    /// @ast-id 10755
    function fun_decodeSqrtPriceX96(var_assetCache_10661_mpos, var_sqrtPriceX96) -> var_price
    {
        let _1 := sub(shl(160, 1), 1)
        let _2 := and(mload(var_assetCache_10661_mpos), _1)
        switch lt(_2, and(loadimmutable("10268"), _1))
        case 0 {
            let _3 := mload(add(var_assetCache_10661_mpos, 416))
            if and(1, gt(_3, 0x12725dd1d243aba0e75fe645cc4873f9e65afe688c928e1f21)) { panic_error_0x11() }
            var_price := fun_mulDiv(var_sqrtPriceX96, var_sqrtPriceX96, checked_div_uint256_12624(mul(0x0de0b6b3a7640000, _3)))
            if iszero(var_price)
            {
                var_price := 0xc097ce7bc90715b34b9f1000000000
                leave
            }
            var_price := checked_div_uint256_12625(var_price)
        }
        default {
            let expr := fun_mulDiv_22854(var_sqrtPriceX96, var_sqrtPriceX96)
            var_price := checked_div_uint256(expr, mload(add(var_assetCache_10661_mpos, 416)))
        }
        let _4 := 0xc097ce7bc90715b34b9f1000000000
        switch gt(var_price, _4)
        case 0 {
            if iszero(var_price) { var_price := 1 }
        }
        default { var_price := _4 }
    }
    function require_helper_stringliteral_e566(condition)
    {
        if iszero(condition)
        {
            let memPtr := mload(64)
            mstore(memPtr, shl(229, 4594637))
            mstore(add(memPtr, 4), 32)
            mstore(add(memPtr, 36), 25)
            mstore(add(memPtr, 68), "e/nested-price-forwarding")
            revert(memPtr, 100)
        }
    }
    /// @ast-id 11067
    function fun_resolvePricingConfig(var_assetCache_10981_mpos, var_config_10984_mpos) -> var_underlying, var_pricingType, var_pricingParameters, var_twapWindow
    {
        let _1 := 0xffff
        let cleaned := and(mload(add(var_assetCache_10981_mpos, 320)), _1)
        switch eq(cleaned, 0x03)
        case 0 {
            var_underlying := and(mload(var_assetCache_10981_mpos), sub(shl(160, 1), 1))
            var_pricingType := cleaned
            var_pricingParameters := and(mload(add(var_assetCache_10981_mpos, 352)), 0xffffffff)
            var_twapWindow := and(mload(add(var_config_10984_mpos, 128)), 0xffffff)
        }
        default {
            var_underlying := read_from_storage_split_offset_address(mapping_index_access_mapping_address_address_of_address_12569(cleanup_address(mload(var_assetCache_10981_mpos))))
            let expr_11011_mpos := fun_resolveAssetConfig(var_underlying)
            var_twapWindow := cleanup_uint24(mload(add(expr_11011_mpos, 128)))
            let _2 := sload(mapping_index_access_mapping_address_address_of_address_12628(cleanup_address(mload(expr_11011_mpos))))
            var_pricingType := and(shr(208, _2), _1)
            var_pricingParameters := shr(224, _2)
            require_helper_stringliteral_e566(iszero(eq(var_pricingType, 0x03)))
        }
    }
    function write_to_memory_address(memPtr, value)
    {
        mstore(memPtr, and(value, sub(shl(160, 1), 1)))
    }
    function require_helper_stringliteral(condition)
    {
        if iszero(condition)
        {
            let memPtr := mload(64)
            mstore(memPtr, shl(229, 4594637))
            mstore(add(memPtr, 4), 32)
            mstore(add(memPtr, 36), 22)
            mstore(add(memPtr, 68), "e/market-not-activated")
            revert(memPtr, 100)
        }
    }
    /// @ast-id 878
    function fun_resolveAssetConfig(var_underlying) -> var_825_mpos
    {
        pop(allocate_and_zero_memory_struct_struct_AssetConfig())
        let _1 := sub(shl(160, 1), 1)
        mstore(0, and(var_underlying, _1))
        mstore(0x20, 0x08)
        let _2 := keccak256(0, 0x40)
        let memPtr := mload(0x40)
        finalize_allocation_12563(memPtr)
        let _3 := sload(_2)
        let _4 := and(_3, _1)
        mstore(memPtr, _4)
        mstore(add(memPtr, 0x20), iszero(iszero(and(shr(160, _3), 0xff))))
        let _5 := 0xffffffff
        mstore(add(memPtr, 0x40), and(shr(168, _3), _5))
        let _6 := add(memPtr, 96)
        mstore(_6, and(shr(200, _3), _5))
        let _7 := add(memPtr, 128)
        mstore(_7, shr(232, _3))
        require_helper_stringliteral(iszero(iszero(_4)))
        if eq(and(cleanup_uint32(mload(_6)), _5), _5)
        {
            write_to_memory_uint32_12631(_6)
        }
        let _8 := 0xffffff
        if eq(and(cleanup_uint24(mload(_7)), _8), _8) { write_to_memory_uint24(_7) }
        var_825_mpos := memPtr
    }
    /// @ast-id 11182
    function fun_getPrice(var_underlying) -> var_twap, var_twapPeriod
    {
        let expr_11154_mpos := fun_resolveAssetConfig(var_underlying)
        let _1 := sub(shl(160, 1), 1)
        let _2 := and(mload(expr_11154_mpos), _1)
        let _3 := 0
        mstore(_3, _2)
        let _4 := 0x20
        mstore(_4, 0x09)
        let _5 := 0x40
        let _6 := fun_loadAssetCache(var_underlying, keccak256(_3, _5))
        let var_origFreeMemPtr := mload(_5)
        let var_twap_1 := _3
        let var_twapPeriod_1 := _3
        var_twap_1 := _3
        var_twapPeriod_1 := _3
        let expr_component, expr_component_1, expr_component_2, expr_component_3 := fun_resolvePricingConfig(_6, expr_11154_mpos)
        let _7 := 0xffff
        let _8 := and(expr_component_1, _7)
        switch eq(_8, 1)
        case 0 {
            switch eq(_8, 0x02)
            case 0 {
                let _9 := mload(_5)
                mstore(_9, shl(229, 4594637))
                revert(_9, sub(abi_encode_stringliteral_ee51(add(_9, 4)), _9))
            }
            default {
                let _10 := 0xffffff
                let expr := fun_computeUniswapPoolAddress(expr_component, and(expr_component_2, _10))
                let var_ago := and(expr_component_3, _10)
                let expr_mpos := allocate_and_zero_memory_array_array_uint32_dyn()
                write_to_memory_uint32(memory_array_index_access_uint32_dyn_12584(expr_mpos), var_ago)
                mstore(memory_array_index_access_uint32_dyn_12585(expr_mpos), _3)
                let expr_mpos_1 := mload(_5)
                let _11 := add(expr_mpos_1, _4)
                let _12 := shl(224, 0x883bdbfd)
                mstore(_11, _12)
                let _13 := sub(abi_encode_array_uint32_dyn(add(expr_mpos_1, 36), expr_mpos), expr_mpos_1)
                let _14 := not(31)
                mstore(expr_mpos_1, add(_13, _14))
                finalize_allocation(expr_mpos_1, _13)
                let expr_component_4 := staticcall(gas(), expr, _11, mload(expr_mpos_1), _3, _3)
                let var_data_mpos := extract_returndata()
                if iszero(expr_component_4)
                {
                    let expr_1 := keccak256(add(var_data_mpos, _4), mload(var_data_mpos))
                    let expr_mpos_2 := mload(_5)
                    let _15 := add(expr_mpos_2, _4)
                    mstore(_15, shl(229, 4594637))
                    let _16 := sub(abi_encode_stringliteral_d30c(add(expr_mpos_2, 36)), expr_mpos_2)
                    mstore(expr_mpos_2, add(_16, _14))
                    finalize_allocation(expr_mpos_2, _16)
                    if iszero(eq(expr_1, keccak256(_15, mload(expr_mpos_2))))
                    {
                        fun_revertBytes(var_data_mpos)
                    }
                    let _17 := and(expr, _1)
                    let _18 := mload(_5)
                    mstore(_18, shl(224, 0x3850c7bd))
                    let _19 := staticcall(gas(), _17, _18, 4, _18, 224)
                    if iszero(_19) { revert_forward() }
                    let expr_component_5 := _3
                    let expr_component_6 := _3
                    let expr_component_7 := _3
                    if _19
                    {
                        finalize_allocation(_18, returndatasize())
                        let expr_component_8, expr_component_9, expr_component_10, expr_component_11, expr_component_12, expr_component_13, expr_component_14 := abi_decode_uint160t_int24t_uint16t_uint16t_uint16t_uint8t_bool_fromMemory(_18, add(_18, returndatasize()))
                        expr_component_5 := expr_component_10
                        expr_component_6 := expr_component_11
                        expr_component_7 := expr_component_12
                    }
                    let expr_2 := mod_uint16(checked_add_uint16(expr_component_5), expr_component_6)
                    let _20 := mload(_5)
                    let _21 := shl(224, 0x252c09d7)
                    mstore(_20, _21)
                    let _22 := staticcall(gas(), _17, _20, sub(abi_encode_uint16(add(_20, 4), expr_2), _20), _20, 128)
                    if iszero(_22) { revert_forward() }
                    let expr_component_15 := _3
                    let expr_component_16 := _3
                    if _22
                    {
                        finalize_allocation(_20, returndatasize())
                        let expr_component_17, expr_component_18, expr_component_19, expr_component_20 := abi_decode_uint32t_int56t_uint160t_bool_fromMemory(_20, add(_20, returndatasize()))
                        expr_component_15 := expr_component_17
                        expr_component_16 := expr_component_20
                    }
                    let var_oldestAvailableAge := expr_component_15
                    if iszero(expr_component_16)
                    {
                        let _23 := mload(_5)
                        mstore(_23, _21)
                        let _24 := staticcall(gas(), _17, _23, sub(abi_encode_rational_by(add(_23, 4)), _23), _23, 128)
                        if iszero(_24) { revert_forward() }
                        let expr_component_21 := _3
                        if _24
                        {
                            finalize_allocation(_23, returndatasize())
                            let expr_component_22, expr_component_23, expr_component_24, expr_component_25 := abi_decode_uint32t_int56t_uint160t_bool_fromMemory(_23, add(_23, returndatasize()))
                            expr_component_21 := expr_component_22
                        }
                        var_oldestAvailableAge := expr_component_21
                    }
                    let _25 := and(expr_component_6, _7)
                    let expr_3 := eq(_25, and(expr_component_7, _7))
                    if expr_3 { expr_3 := lt(_25, _7) }
                    if expr_3
                    {
                        let expr_4 := checked_add_uint16(expr_component_6)
                        if iszero(extcodesize(_17)) { revert(_3, _3) }
                        let _26 := mload(_5)
                        mstore(_26, shl(224, 0x32148f67))
                        let _27 := call(gas(), _17, _3, _26, sub(abi_encode_uint16(add(_26, 4), expr_4), _26), _26, _3)
                        if iszero(_27) { revert_forward() }
                        if _27
                        {
                            finalize_allocation(_26, returndatasize())
                            abi_decode(_26, add(_26, returndatasize()))
                        }
                    }
                    let _28 := 0xffffffff
                    var_ago := checked_sub_uint256(timestamp(), and(var_oldestAvailableAge, _28))
                    write_to_memory_uint32(memory_array_index_access_uint32_dyn_12584(expr_mpos), and(var_ago, _28))
                    let expr_mpos_3 := mload(_5)
                    let _29 := add(expr_mpos_3, _4)
                    mstore(_29, _12)
                    let _30 := sub(abi_encode_array_uint32_dyn(add(expr_mpos_3, 36), expr_mpos), expr_mpos_3)
                    mstore(expr_mpos_3, add(_30, _14))
                    finalize_allocation(expr_mpos_3, _30)
                    let expr_component_26 := staticcall(gas(), expr, _29, mload(expr_mpos_3), _3, _3)
                    var_data_mpos := extract_returndata()
                    if iszero(expr_component_26)
                    {
                        fun_revertBytes(var_data_mpos)
                    }
                }
                let expr_mpos_4 := abi_decode_array_int56_dyn_fromMemory(add(var_data_mpos, _4), add(add(var_data_mpos, mload(var_data_mpos)), _4))
                let _31 := read_from_memoryt_int56(memory_array_index_access_uint32_dyn_12585(expr_mpos_4))
                let expr_5 := checked_sub_int56(_31, read_from_memoryt_int56(memory_array_index_access_uint32_dyn_12584(expr_mpos_4)))
                let var := fun_decodeSqrtPriceX96(_6, cleanup_address(fun_getSqrtRatioAtTick(convert_int56_to_int24(checked_div_int56(expr_5, signextend(6, var_ago))))))
                var_twapPeriod_1 := var_ago
                var_twap_1 := var
            }
        }
        default {
            var_twap_1 := 0x0de0b6b3a7640000
            var_twapPeriod_1 := and(expr_component_3, 0xffffff)
        }
        mstore(_5, var_origFreeMemPtr)
        var_twapPeriod := var_twapPeriod_1
        var_twap := var_twap_1
    }
    function allocate_and_zero_memory_struct_struct_AssetCache() -> memPtr
    {
        let memPtr_1 := mload(64)
        let newFreePtr := add(memPtr_1, 480)
        if or(gt(newFreePtr, 0xffffffffffffffff), lt(newFreePtr, memPtr_1)) { panic_error_0x41() }
        mstore(64, newFreePtr)
        memPtr := memPtr_1
        let _1 := 0
        mstore(memPtr_1, _1)
        mstore(add(memPtr_1, 32), _1)
        mstore(add(memPtr_1, 64), _1)
        mstore(add(memPtr_1, 96), _1)
        mstore(add(memPtr_1, 128), _1)
        mstore(add(memPtr_1, 160), _1)
        mstore(add(memPtr_1, 192), _1)
        mstore(add(memPtr_1, 224), _1)
        mstore(add(memPtr_1, 256), _1)
        mstore(add(memPtr_1, 288), _1)
        mstore(add(memPtr_1, 320), _1)
        mstore(add(memPtr_1, 352), _1)
        mstore(add(memPtr_1, 384), _1)
        mstore(add(memPtr_1, 416), _1)
        mstore(add(memPtr_1, 448), _1)
    }
    function update_storage_value_offsett_uint96_to_uint96(slot, value)
    {
        let _1 := sload(slot)
        sstore(slot, or(and(_1, sub(shl(160, 1), 1)), and(shl(160, value), shl(160, 0xffffffffffffffffffffffff))))
    }
    function update_storage_value_offsett_uint112_to_uint112(slot, value)
    {
        sstore(slot, or(and(sload(slot), not(0xffffffffffffffffffffffffffff)), and(value, 0xffffffffffffffffffffffffffff)))
    }
    function update_storage_value_offsett_uint144_to_uint144(slot, value)
    {
        let _1 := sload(slot)
        sstore(slot, or(and(_1, 0xffffffffffffffffffffffffffff), and(shl(112, value), not(0xffffffffffffffffffffffffffff))))
    }
    /// @ast-id 1304
    function fun_loadAssetCache(var_underlying, var_assetStorage_1248_slot) -> var_assetCache_1252_mpos
    {
        var_assetCache_1252_mpos := allocate_and_zero_memory_struct_struct_AssetCache()
        if fun_initAssetCache(var_underlying, var_assetStorage_1248_slot, var_assetCache_1252_mpos)
        {
            sstore(var_assetStorage_1248_slot, or(and(sload(var_assetStorage_1248_slot), not(0xffffffffff)), and(mload(add(var_assetCache_1252_mpos, 160)), 0xffffffffff)))
            let cleaned := and(mload(var_assetCache_1252_mpos), sub(shl(160, 1), 1))
            let _1 := add(var_assetStorage_1248_slot, 1)
            sstore(_1, or(and(sload(_1), shl(160, 0xffffffffffffffffffffffff)), cleaned))
            update_storage_value_offsett_uint96_to_uint96(_1, cleanup_uint96(mload(add(var_assetCache_1252_mpos, 96))))
            let _2 := cleanup_uint112(mload(add(var_assetCache_1252_mpos, 32)))
            let _3 := add(var_assetStorage_1248_slot, 3)
            update_storage_value_offsett_uint112_to_uint112(_3, _2)
            update_storage_value_offsett_uint144_to_uint144(_3, cleanup_uint144(mload(add(var_assetCache_1252_mpos, 64))))
            sstore(add(var_assetStorage_1248_slot, 4), mload(add(var_assetCache_1252_mpos, 128)))
        }
    }
    function write_to_memory_uint40(memPtr, value)
    {
        mstore(memPtr, and(value, 0xffffffffff))
    }
    function write_to_memory_uint8(memPtr, value)
    {
        mstore(memPtr, and(value, 0xff))
    }
    function write_to_memory_int96(memPtr, value)
    {
        mstore(memPtr, signextend(11, value))
    }
    function extract_from_storage_value_offsett_uint96(slot_value) -> value
    { value := shr(160, slot_value) }
    function write_to_memory_uint96(memPtr, value)
    {
        mstore(memPtr, and(value, 0xffffffffffffffffffffffff))
    }
    function read_from_storage_split_offset_uint112(slot) -> value
    {
        value := and(sload(slot), 0xffffffffffffffffffffffffffff)
    }
    function write_to_memory_uint112(memPtr, value)
    {
        mstore(memPtr, and(value, 0xffffffffffffffffffffffffffff))
    }
    function read_from_storage_split_offset_uint144(slot) -> value
    {
        value := shr(112, sload(slot))
    }
    function write_to_memory_uint144(memPtr, value)
    {
        mstore(memPtr, and(value, 0xffffffffffffffffffffffffffffffffffff))
    }
    function checked_add_int256(x) -> sum
    {
        let _1 := slt(x, 0)
        let _2 := 0x033b2e3c9fd0803ce8000000
        if and(iszero(_1), sgt(_2, sub(sub(shl(255, 1), 1), x))) { panic_error_0x11() }
        if and(_1, slt(_2, sub(shl(255, 1), x))) { panic_error_0x11() }
        sum := add(x, _2)
    }
    function checked_add_uint256(x, y) -> sum
    {
        if gt(x, not(y)) { panic_error_0x11() }
        sum := add(x, y)
    }
    /// @ast-id 1243
    function fun_initAssetCache(var_underlying, var_assetStorage_slot, var_assetCache_917_mpos) -> var_dirty
    {
        var_dirty := 0x00
        write_to_memory_address(var_assetCache_917_mpos, var_underlying)
        let _1 := sload(var_assetStorage_slot)
        let _2 := add(var_assetCache_917_mpos, 160)
        let _3 := 0xffffffffff
        write_to_memory_uint40(_2, and(_1, _3))
        let _4 := and(shr(40, _1), 0xff)
        write_to_memory_uint8(add(var_assetCache_917_mpos, 192), _4)
        let _5 := 0xffffffff
        write_to_memory_uint32(add(var_assetCache_917_mpos, 224), and(shr(48, _1), _5))
        let _6 := add(var_assetCache_917_mpos, 256)
        write_to_memory_int96(_6, signextend(11, shr(80, _1)))
        let _7 := add(var_assetCache_917_mpos, 288)
        write_to_memory_uint32(_7, and(shr(176, _1), _5))
        write_to_memory_uint16(add(var_assetCache_917_mpos, 320), and(shr(208, _1), 0xffff))
        write_to_memory_uint32(add(var_assetCache_917_mpos, 352), shr(224, _1))
        let _8 := extract_from_storage_value_offsett_uint96(sload(add(var_assetStorage_slot, 1)))
        let _9 := add(var_assetCache_917_mpos, 96)
        write_to_memory_uint96(_9, _8)
        let _10 := sload(add(var_assetStorage_slot, 3))
        let _11 := add(var_assetCache_917_mpos, 32)
        let _12 := 0xffffffffffffffffffffffffffff
        write_to_memory_uint112(_11, and(_10, _12))
        let _13 := add(var_assetCache_917_mpos, 64)
        write_to_memory_uint144(_13, shr(112, _10))
        let _14 := sload(add(var_assetStorage_slot, 4))
        let _15 := add(var_assetCache_917_mpos, 128)
        mstore(_15, _14)
        let expr := exp(0x0a, and(sub(0x12, _4), 0xff))
        let _16 := add(var_assetCache_917_mpos, 416)
        mstore(_16, expr)
        let expr_1 := checked_div_uint256_12645(expr)
        let _17 := add(var_assetCache_917_mpos, 448)
        mstore(_17, expr_1)
        let expr_2 := modifier_FREEMEM_12646(var_assetCache_917_mpos, address())
        switch iszero(gt(expr_2, mload(_17)))
        case 0 {
            mstore(add(var_assetCache_917_mpos, 384), 0x00)
        }
        default {
            mstore(add(var_assetCache_917_mpos, 384), mul(expr_2, mload(_16)))
        }
        let _18 := cleanup_uint40(cleanup_uint40(mload(_2)))
        if iszero(eq(timestamp(), _18))
        {
            var_dirty := 1
            let expr_3 := fun_rpow(checked_add_int256(cleanup_int96(cleanup_int96(mload(_6)))), checked_sub_uint256(timestamp(), _18))
            let expr_4 := checked_div_uint256_12649(checked_mul_uint256(expr_3, mload(_15)))
            let expr_5 := checked_mul_uint256(cleanup_uint144(cleanup_uint144(mload(_13))), expr_4)
            let expr_6 := checked_div_uint256(expr_5, mload(_15))
            let var_newReserveBalance := cleanup_uint96(cleanup_uint96(mload(_9)))
            let var_newTotalBalances := cleanup_uint112(cleanup_uint112(mload(_11)))
            let expr_7 := checked_sub_uint256(expr_6, cleanup_uint144(cleanup_uint144(mload(_13))))
            let _19 := cleanup_uint32(mload(_7))
            let expr_8 := 0x00
            switch eq(and(_19, _5), _5)
            case 0 { expr_8 := _19 }
            default { expr_8 := 0x36d61600 }
            let expr_9 := checked_div_uint256_22855(checked_mul_uint256(expr_7, and(expr_8, _5)))
            if iszero(iszero(expr_9))
            {
                let _20 := mload(add(var_assetCache_917_mpos, 384))
                let expr_10 := checked_add_uint256(_20, div(expr_6, 0x3b9aca00))
                let expr_11 := checked_mul_uint256(expr_10, var_newTotalBalances)
                var_newTotalBalances := checked_div_uint256(expr_11, checked_sub_uint256(expr_10, expr_9))
                var_newReserveBalance := checked_add_uint256(var_newReserveBalance, checked_sub_uint256(var_newTotalBalances, cleanup_uint112(cleanup_uint112(mload(_11)))))
            }
            let expr_12 := iszero(gt(var_newTotalBalances, _12))
            if expr_12
            {
                expr_12 := iszero(gt(expr_6, 0xffffffffffffffffffffffffffffffffffff))
            }
            if expr_12
            {
                write_to_memory_uint144(_13, fun_encodeDebtAmount(expr_6))
                mstore(_15, expr_4)
                write_to_memory_uint40(_2, and(timestamp(), _3))
                if iszero(eq(var_newTotalBalances, cleanup_uint112(cleanup_uint112(mload(_11)))))
                {
                    write_to_memory_uint96(_9, fun_encodeSmallAmount(var_newReserveBalance))
                    write_to_memory_uint112(_11, fun_encodeAmount(var_newTotalBalances))
                }
            }
        }
    }
    /// @ast-id 1369
    function fun_encodeAmount(var_amount) -> var
    {
        let _1 := 0xffffffffffffffffffffffffffff
        if gt(var_amount, _1)
        {
            let memPtr := mload(64)
            mstore(memPtr, shl(229, 4594637))
            mstore(add(memPtr, 4), 32)
            mstore(add(memPtr, 36), 28)
            mstore(add(memPtr, 68), "e/amount-too-large-to-encode")
            revert(memPtr, 100)
        }
        var := and(var_amount, _1)
    }
    /// @ast-id 1389
    function fun_encodeSmallAmount(var_amount) -> var
    {
        let _1 := 0xffffffffffffffffffffffff
        if gt(var_amount, _1)
        {
            let memPtr := mload(64)
            mstore(memPtr, shl(229, 4594637))
            mstore(add(memPtr, 4), 32)
            mstore(add(memPtr, 36), 34)
            mstore(add(memPtr, 68), "e/small-amount-too-large-to-enco")
            mstore(add(memPtr, 100), "de")
            revert(memPtr, 132)
        }
        var := and(var_amount, _1)
    }
    /// @ast-id 1409
    function fun_encodeDebtAmount(var_amount) -> var
    {
        let _1 := 0xffffffffffffffffffffffffffffffffffff
        if gt(var_amount, _1)
        {
            let memPtr := mload(64)
            mstore(memPtr, shl(229, 4594637))
            mstore(add(memPtr, 4), 32)
            mstore(add(memPtr, 36), 33)
            mstore(add(memPtr, 68), "e/debt-amount-too-large-to-encod")
            mstore(add(memPtr, 100), "e")
            revert(memPtr, 132)
        }
        var := and(var_amount, _1)
    }
    /// @ast-id 147
    function modifier_FREEMEM_12646(var_assetCache_mpos, var_account) -> _1
    {
        let var_origFreeMemPtr := mload(0x40)
        _1 := fun_callBalanceOf_inner(var_assetCache_mpos, var_account)
        mstore(0x40, var_origFreeMemPtr)
    }
    function fun_callBalanceOf_inner(var_assetCache_mpos, var_account) -> var
    {
        let _1 := sub(shl(160, 1), 1)
        let cleaned := and(mload(var_assetCache_mpos), _1)
        let expr_1542_mpos := mload(64)
        let _2 := add(expr_1542_mpos, 0x20)
        mstore(_2, shl(224, 0x70a08231))
        mstore(add(expr_1542_mpos, 36), and(var_account, _1))
        mstore(expr_1542_mpos, 36)
        finalize_allocation_22852(expr_1542_mpos)
        let expr_1543_component := staticcall(0x4e20, cleaned, _2, mload(expr_1542_mpos), 0, 0)
        let expr_component_mpos := extract_returndata()
        let expr := iszero(expr_1543_component)
        if iszero(expr)
        {
            expr := lt(mload(expr_component_mpos), 0x20)
        }
        if expr
        {
            var := 0
            leave
        }
        if slt(sub(add(expr_component_mpos, mload(expr_component_mpos)), expr_component_mpos), 0x20) { revert(0, 0) }
        var := mload(add(expr_component_mpos, 0x20))
    }
    /// @ast-id 11294
    function fun_getPriceFull(var_underlying) -> var_twap, var_twapPeriod, var_currPrice
    {
        var_currPrice := 0
        let expr_11199_mpos := fun_resolveAssetConfig(var_underlying)
        let _1 := mapping_index_access_mapping_address_address_of_address_12628(cleanup_address(mload(expr_11199_mpos)))
        let expr_11215_mpos := fun_loadAssetCache(var_underlying, _1)
        let var_twap_1, var_twapPeriod_1 := modifier_FREEMEM(expr_11215_mpos, expr_11199_mpos)
        var_twapPeriod := var_twapPeriod_1
        var_twap := var_twap_1
        let expr_11235_component, expr_component, expr_component_1, expr_component_2 := fun_resolvePricingConfig(expr_11215_mpos, expr_11199_mpos)
        let _2 := and(expr_component, 0xffff)
        switch eq(_2, 0x01)
        case 0 {
            let expr := eq(_2, 0x02)
            if iszero(expr) { expr := eq(_2, 0x03) }
            switch expr
            case 0 {
                let _3 := mload(64)
                mstore(_3, shl(229, 4594637))
                revert(_3, sub(abi_encode_stringliteral_ee51(add(_3, 4)), _3))
            }
            default {
                let expr_11258_mpos := fun_loadAssetCache(expr_11235_component, _1)
                let expr_address := cleanup_address(cleanup_address(fun_computeUniswapPoolAddress(expr_11235_component, and(expr_component_1, 0xffffff))))
                let _4 := mload(64)
                mstore(_4, shl(224, 0x3850c7bd))
                let _5 := staticcall(gas(), expr_address, _4, 4, _4, 224)
                if iszero(_5) { revert_forward() }
                let expr_component_3 := 0
                if _5
                {
                    finalize_allocation(_4, returndatasize())
                    let expr_component_4, expr_component_5, expr_component_6, expr_component_7, expr_component_8, expr_component_9, expr_component_10 := abi_decode_uint160t_int24t_uint16t_uint16t_uint16t_uint8t_bool_fromMemory(_4, add(_4, returndatasize()))
                    expr_component_3 := expr_component_4
                }
                var_currPrice := fun_decodeSqrtPriceX96(expr_11258_mpos, and(expr_component_3, sub(shl(160, 1), 1)))
            }
        }
        default {
            var_currPrice := 0x0de0b6b3a7640000
        }
    }
    function allocate_and_zero_memory_struct_struct_LiquidityStatus() -> memPtr
    {
        let memPtr_1 := mload(64)
        let newFreePtr := add(memPtr_1, 128)
        if or(gt(newFreePtr, 0xffffffffffffffff), lt(newFreePtr, memPtr_1)) { panic_error_0x41() }
        mstore(64, newFreePtr)
        memPtr := memPtr_1
        mstore(memPtr_1, 0)
        mstore(add(memPtr_1, 32), 0)
        mstore(add(memPtr_1, 64), 0)
        mstore(add(memPtr_1, 96), 0)
    }
    /// @ast-id 11550
    function fun_computeLiquidity(var_account) -> var__mpos
    {
        pop(allocate_and_zero_memory_struct_struct_LiquidityStatus())
        var__mpos := fun_computeLiquidityRaw(var_account, fun_getEnteredMarketsArray(var_account))
    }
    function allocate_and_zero_memory_array_array_address_dyn() -> memPtr
    {
        let memPtr_1 := mload(64)
        finalize_allocation_12670(memPtr_1)
        mstore(memPtr_1, 0x01)
        memPtr := memPtr_1
        calldatacopy(add(memPtr_1, 0x20), calldatasize(), 0x20)
    }
    function storage_array_index_access_address_ptr(array, index) -> slot, offset
    {
        if iszero(lt(index, 0x0100000000)) { panic_error_0x32() }
        slot := add(array, index)
        offset := 0
    }
    function extract_from_storage_value_dynamict_address(slot_value, offset) -> value
    {
        value := and(shr(shl(3, offset), slot_value), sub(shl(160, 1), 1))
    }
    /// @ast-id 510
    function fun_getEnteredMarketsArray(var_account) -> var_438_mpos
    {
        let _1 := sub(shl(160, 1), 1)
        mstore(0, and(var_account, _1))
        mstore(0x20, 0x06)
        let value := and(shr(48, sload(keccak256(0, 0x40))), 0xffffffff)
        let value_1 := and(shr(80, sload(keccak256(0, 0x40))), _1)
        let _2 := array_allocation_size_array_uint32_dyn(value)
        let memPtr := mload(0x40)
        finalize_allocation(memPtr, _2)
        mstore(memPtr, value)
        calldatacopy(add(memPtr, 0x20), calldatasize(), add(array_allocation_size_array_uint32_dyn(value), not(31)))
        if iszero(value)
        {
            var_438_mpos := memPtr
            leave
        }
        let _3 := mapping_index_access_mapping_address_address_of_address_12656(var_account)
        write_to_memory_address(memory_array_index_access_uint32_dyn_12584(memPtr), value_1)
        let var_i := 1
        for { }
        lt(var_i, value)
        {
            var_i := increment_uint256(var_i)
        }
        {
            let _4, _5 := storage_array_index_access_address_ptr(_3, var_i)
            write_to_memory_address(memory_array_index_access_uint32_dyn(memPtr, var_i), extract_from_storage_value_dynamict_address(sload(_4), _5))
        }
        var_438_mpos := memPtr
    }
    /// @ast-id 11533
    function fun_computeLiquidityRaw(var_account, var_underlyings_mpos) -> var_status_mpos
    {
        var_status_mpos := allocate_and_zero_memory_struct_struct_LiquidityStatus()
        mstore(var_status_mpos, 0x00)
        let _1 := add(var_status_mpos, 32)
        mstore(_1, 0x00)
        let _2 := add(var_status_mpos, 64)
        mstore(_2, 0x00)
        let _3 := add(var_status_mpos, 96)
        mstore(_3, 0x00)
        pop(allocate_and_zero_memory_struct_struct_AssetConfig())
        let zero_struct_AssetCache_mpos := allocate_and_zero_memory_struct_struct_AssetCache()
        let var_i := 0x00
        for { }
        1
        {
            var_i := increment_uint256(var_i)
        }
        {
            if iszero(lt(var_i, mload(var_underlyings_mpos))) { break }
            let _4 := read_from_memoryt_address(memory_array_index_access_uint32_dyn(var_underlyings_mpos, var_i))
            let var_price := 0x00
            let var_assetCacheAndPriceInited := 0x00
            var_price := 0x00
            let var_config_mpos := fun_resolveAssetConfig(_4)
            let _5 := mapping_index_access_mapping_address_address_of_address_12628(cleanup_address(mload(var_config_mpos)))
            let var_balance := cleanup_uint112(read_from_storage_split_offset_uint112(mapping_index_access_mapping_address_address_of_address(add(_5, 5), var_account)))
            let var_owed := cleanup_uint144(read_from_storage_split_offset_uint144(mapping_index_access_mapping_address_address_of_address(add(_5, 5), var_account)))
            let expr := iszero(iszero(var_balance))
            if expr
            {
                expr := iszero(iszero(and(cleanup_uint32(mload(add(var_config_mpos, 64))), 0xffffffff)))
            }
            if expr
            {
                pop(fun_initAssetCache(_4, _5, zero_struct_AssetCache_mpos))
                let var_twap, var_twapPeriod := modifier_FREEMEM_12660(zero_struct_AssetCache_mpos, var_config_mpos)
                var_price := var_twap
                var_assetCacheAndPriceInited := 0x01
                let expr_1 := checked_div_uint256_12661(checked_mul_uint256(div(checked_mul_uint256(var_balance, fun_computeExchangeRate(zero_struct_AssetCache_mpos)), 0x0de0b6b3a7640000), var_twap))
                mstore(var_status_mpos, checked_add_uint256(mload(var_status_mpos), checked_div_uint256_12662(checked_mul_uint256(expr_1, cleanup_uint32(cleanup_uint32(mload(add(var_config_mpos, 64))))))))
            }
            if iszero(iszero(var_owed))
            {
                if iszero(var_assetCacheAndPriceInited)
                {
                    pop(fun_initAssetCache(_4, _5, zero_struct_AssetCache_mpos))
                    let var_twap_1, var_twapPeriod_1 := modifier_FREEMEM_12660(zero_struct_AssetCache_mpos, var_config_mpos)
                    var_price := var_twap_1
                }
                mstore(_2, increment_uint256(mload(_2)))
                if cleanup_bool(mload(add(var_config_mpos, 32))) { write_to_memory_bool(_3) }
                let expr_2 := checked_div_uint256_12661(checked_mul_uint256(div(fun_roundUpOwed(zero_struct_AssetCache_mpos, fun_getCurrentOwedExact(_5, zero_struct_AssetCache_mpos, var_account, shr(112, sload(mapping_index_access_mapping_address_address_of_address(add(_5, 5), var_account))))), 0x3b9aca00), var_price))
                let _6 := add(var_config_mpos, 96)
                let expr_3 := iszero(iszero(and(cleanup_uint32(mload(_6)), 0xffffffff)))
                let expr_4 := 0x00
                switch expr_3
                case 0 {
                    expr_4 := 0xffffffffffffffffffffffffffffffffffff
                }
                default {
                    let expr_5 := checked_mul_uint256_12666(expr_2)
                    expr_4 := checked_div_uint256(expr_5, cleanup_uint32(cleanup_uint32(mload(_6))))
                }
                mstore(_1, checked_add_uint256(mload(_1), expr_4))
            }
        }
    }
    /// @ast-id 1440
    function fun_computeExchangeRate(var_assetCache_1412_mpos) -> var
    {
        let _1 := add(var_assetCache_1412_mpos, 32)
        let _2 := 0xffffffffffffffffffffffffffff
        if iszero(and(mload(_1), _2))
        {
            var := 0x0de0b6b3a7640000
            leave
        }
        let _3 := mload(add(var_assetCache_1412_mpos, 384))
        let expr := checked_add_uint256(_3, div(and(mload(add(var_assetCache_1412_mpos, 64)), 0xffffffffffffffffffffffffffffffffffff), 0x3b9aca00))
        let _4 := 0x0de0b6b3a7640000
        if and(iszero(iszero(expr)), gt(_4, div(not(0), expr))) { panic_error_0x11() }
        let cleaned := and(mload(_1), _2)
        if iszero(cleaned) { panic_error_0x12() }
        var := div(mul(expr, _4), cleaned)
    }
    /// @ast-id 2027
    function fun_getCurrentOwedExact(var_assetStorage_1997_slot, var_assetCache_mpos, var_account, var_owed) -> var
    {
        if iszero(var_owed)
        {
            var := 0x00
            leave
        }
        let expr := checked_mul_uint256(var_owed, mload(add(var_assetCache_mpos, 128)))
        let _1 := sload(add(mapping_index_access_mapping_address_address_of_address(add(var_assetStorage_1997_slot, 5), var_account), 1))
        if iszero(_1) { panic_error_0x12() }
        var := div(expr, _1)
    }
    /// @ast-id 2063
    function fun_roundUpOwed(var_assetCache_2030_mpos, var_owed) -> var_
    {
        if iszero(var_owed)
        {
            var_ := 0x00
            leave
        }
        let _1 := mload(add(var_assetCache_2030_mpos, 416))
        let _2 := 0x3b9aca00
        let product := mul(_2, _1)
        if iszero(product) { panic_error_0x12() }
        var_ := mul(mul(div(add(add(var_owed, product), not(0)), product), _1), _2)
    }
    /// @ast-id 11632
    function fun_computeAssetLiquidities(var_account) -> var_mpos
    {
        let expr_11567_mpos := fun_getEnteredMarketsArray(var_account)
        let _1 := mload(expr_11567_mpos)
        let _2 := array_allocation_size_array_uint32_dyn(_1)
        let _3 := 64
        let memPtr := mload(_3)
        finalize_allocation(memPtr, _2)
        mstore(memPtr, _1)
        let _4 := add(array_allocation_size_array_uint32_dyn(_1), not(31))
        let i := 0
        let i_1 := i
        for { } lt(i_1, _4) { i_1 := add(i_1, 32) }
        {
            let memPtr_1 := mload(_3)
            finalize_allocation_12670(memPtr_1)
            mstore(memPtr_1, i)
            let _5 := allocate_and_zero_memory_struct_struct_LiquidityStatus()
            let _6 := 32
            mstore(add(memPtr_1, _6), _5)
            mstore(add(add(memPtr, i_1), _6), memPtr_1)
        }
        let expr_mpos := allocate_and_zero_memory_array_array_address_dyn()
        let var_i := i
        for { }
        0x01
        {
            var_i := increment_uint256(var_i)
        }
        {
            if iszero(lt(var_i, mload(expr_11567_mpos))) { break }
            let _7 := read_from_memoryt_address(memory_array_index_access_uint32_dyn(expr_11567_mpos, var_i))
            write_to_memory_address(memory_array_index_access_uint32_dyn_12584(expr_mpos), _7)
            write_to_memory_address(mload(memory_array_index_access_uint32_dyn(memPtr, var_i)), _7)
            mstore(add(mload(memory_array_index_access_uint32_dyn(memPtr, var_i)), 32), fun_computeLiquidityRaw(var_account, expr_mpos))
        }
        var_mpos := memPtr
    }
    /// @ast-id 11667
    function fun_requireLiquidity(var_account)
    {
        pop(allocate_and_zero_memory_struct_struct_LiquidityStatus())
        let var_mpos := fun_computeLiquidityRaw(var_account, fun_getEnteredMarketsArray(var_account))
        let expr := iszero(mload(add(var_mpos, 96)))
        if iszero(expr)
        {
            expr := eq(mload(add(var_mpos, 64)), 0x01)
        }
        if iszero(expr)
        {
            let memPtr := mload(64)
            mstore(memPtr, shl(229, 4594637))
            mstore(add(memPtr, 4), 32)
            mstore(add(memPtr, 36), 28)
            mstore(add(memPtr, 68), "e/borrow-isolation-violation")
            revert(memPtr, 100)
        }
        let _1 := mload(var_mpos)
        if lt(_1, mload(add(var_mpos, 32)))
        {
            let memPtr_1 := mload(64)
            mstore(memPtr_1, shl(229, 4594637))
            mstore(add(memPtr_1, 4), 32)
            mstore(add(memPtr_1, 36), 22)
            mstore(add(memPtr_1, 68), "e/collateral-violation")
            revert(memPtr_1, 100)
        }
    }
    function require_helper(condition)
    {
        if iszero(condition) { revert(0, 0) }
    }
    /// @ast-id 13238
    function fun_mulDiv_22854(var_a, var_b) -> var_result
    {
        let usr$mm := mulmod(var_a, var_b, not(0))
        let var_prod0 := mul(var_a, var_b)
        let _1 := lt(usr$mm, var_prod0)
        let _2 := sub(usr$mm, var_prod0)
        let var_prod1 := sub(_2, _1)
        if eq(_2, _1)
        {
            var_result := div(var_prod0, 0x12725dd1d243aba0e75fe645cc4873f9e6)
            leave
        }
        let _3 := 0x12725dd1d243aba0e75fe645cc4873f9e6
        if iszero(gt(_3, var_prod1)) { revert(0, 0) }
        let var_remainder := mulmod(var_a, var_b, _3)
        var_result := mul(or(shr(0x01, sub(var_prod0, var_remainder)), shl(255, sub(var_prod1, gt(var_remainder, var_prod0)))), 0x80bf433070d530fec992d281993084ea0851f19952bccb6b6d6a90f542af7c3b)
    }
    /// @ast-id 13238
    function fun_mulDiv(var_a, var_b, var_denominator) -> var_result
    {
        let usr$mm := mulmod(var_a, var_b, not(0))
        let var_prod0 := mul(var_a, var_b)
        let _1 := lt(usr$mm, var_prod0)
        let _2 := sub(usr$mm, var_prod0)
        let var_prod1 := sub(_2, _1)
        if eq(_2, _1)
        {
            require_helper(iszero(iszero(var_denominator)))
            var_result := div(var_prod0, var_denominator)
            leave
        }
        require_helper(gt(var_denominator, var_prod1))
        let var_remainder := mulmod(var_a, var_b, var_denominator)
        let expr := and(var_denominator, add(not(var_denominator), 0x01))
        let var_denominator_1 := div(var_denominator, expr)
        let _3 := 0x02
        let expr_1 := xor(mul(0x03, var_denominator_1), _3)
        let expr_2 := mul(expr_1, sub(_3, mul(var_denominator_1, expr_1)))
        let expr_3 := mul(expr_2, sub(_3, mul(var_denominator_1, expr_2)))
        let expr_4 := mul(expr_3, sub(_3, mul(var_denominator_1, expr_3)))
        let expr_5 := mul(expr_4, sub(_3, mul(var_denominator_1, expr_4)))
        let expr_6 := mul(expr_5, sub(_3, mul(var_denominator_1, expr_5)))
        var_result := mul(or(div(sub(var_prod0, var_remainder), expr), mul(sub(var_prod1, gt(var_remainder, var_prod0)), add(div(sub(0, expr), expr), 0x01))), mul(expr_6, sub(_3, mul(var_denominator_1, expr_6))))
    }
    /// @ast-id 13412
    function fun_rpow(var_x, var_n) -> var_z
    {
        switch var_x
        case 0 {
            switch var_n
            case 0 {
                var_z := 0x033b2e3c9fd0803ce8000000
            }
            default { var_z := 0 }
        }
        default {
            let _1 := 1
            switch and(var_n, _1)
            case 0 {
                var_z := 0x033b2e3c9fd0803ce8000000
            }
            default { var_z := var_x }
            var_n := shr(_1, var_n)
            for { } var_n { var_n := shr(_1, var_n) }
            {
                let usr$xx := mul(var_x, var_x)
                if iszero(eq(div(usr$xx, var_x), var_x))
                {
                    let _2 := 0
                    revert(_2, _2)
                }
                let _3 := 0x019d971e4fe8401e74000000
                let usr$xxRound := add(usr$xx, _3)
                if lt(usr$xxRound, usr$xx)
                {
                    let _4 := 0
                    revert(_4, _4)
                }
                let _5 := 0x033b2e3c9fd0803ce8000000
                var_x := div(usr$xxRound, _5)
                if and(var_n, _1)
                {
                    let usr$zx := mul(var_z, var_x)
                    if and(iszero(iszero(var_x)), iszero(eq(div(usr$zx, var_x), var_z)))
                    {
                        let _6 := 0
                        revert(_6, _6)
                    }
                    let usr$zxRound := add(usr$zx, _3)
                    if lt(usr$zxRound, usr$zx)
                    {
                        let _7 := 0
                        revert(_7, _7)
                    }
                    var_z := div(usr$zxRound, _5)
                }
            }
        }
    }
    function require_helper_stringliteral_846b(condition)
    {
        if iszero(condition)
        {
            let memPtr := mload(64)
            mstore(memPtr, shl(229, 4594637))
            mstore(add(memPtr, 4), 32)
            mstore(add(memPtr, 36), 1)
            mstore(add(memPtr, 68), "T")
            revert(memPtr, 100)
        }
    }
    /// @ast-id 13812
    function fun_getSqrtRatioAtTick(var_tick) -> var_sqrtPriceX96
    {
        let _1 := signextend(2, var_tick)
        let expr := 0x00
        switch slt(_1, expr)
        case 0 { expr := _1 }
        default { expr := sub(0x00, _1) }
        require_helper_stringliteral_846b(iszero(gt(expr, 887272)))
        let expr_1 := 0x00
        switch iszero(iszero(and(expr, 0x01)))
        case 0 { expr_1 := shl(128, 1) }
        default {
            expr_1 := 0xfffcb933bd6fad37aa2d162d1a594001
        }
        let var_ratio := and(expr_1, 0xffffffffffffffffffffffffffffffffff)
        if iszero(iszero(and(expr, 2)))
        {
            var_ratio := shr(128, mul(var_ratio, 0xfff97272373d413259a46990580e213a))
        }
        if iszero(iszero(and(expr, 0x04)))
        {
            var_ratio := shr(128, mul(var_ratio, 0xfff2e50f5f656932ef12357cf3c7fdcc))
        }
        if iszero(iszero(and(expr, 0x08)))
        {
            var_ratio := shr(128, mul(var_ratio, 0xffe5caca7e10e4e61c3624eaa0941cd0))
        }
        if iszero(iszero(and(expr, 0x10)))
        {
            var_ratio := shr(128, mul(var_ratio, 0xffcb9843d60f6159c9db58835c926644))
        }
        if iszero(iszero(and(expr, 0x20)))
        {
            var_ratio := shr(128, mul(var_ratio, 0xff973b41fa98c081472e6896dfb254c0))
        }
        if iszero(iszero(and(expr, 0x40)))
        {
            var_ratio := shr(128, mul(var_ratio, 0xff2ea16466c96a3843ec78b326b52861))
        }
        let _2 := 0x80
        if iszero(iszero(and(expr, _2)))
        {
            var_ratio := shr(_2, mul(var_ratio, 0xfe5dee046a99a2a811c461f1969c3053))
        }
        if iszero(iszero(and(expr, 0x0100)))
        {
            var_ratio := shr(_2, mul(var_ratio, 0xfcbe86c7900a88aedcffc83b479aa3a4))
        }
        if iszero(iszero(and(expr, 0x0200)))
        {
            var_ratio := shr(_2, mul(var_ratio, 0xf987a7253ac413176f2b074cf7815e54))
        }
        if iszero(iszero(and(expr, 0x0400)))
        {
            var_ratio := shr(_2, mul(var_ratio, 0xf3392b0822b70005940c7a398e4b70f3))
        }
        if iszero(iszero(and(expr, 0x0800)))
        {
            var_ratio := shr(_2, mul(var_ratio, 0xe7159475a2c29b7443b29c7fa6e889d9))
        }
        if iszero(iszero(and(expr, 0x1000)))
        {
            var_ratio := shr(_2, mul(var_ratio, 0xd097f3bdfd2022b8845ad8f792aa5825))
        }
        if iszero(iszero(and(expr, 0x2000)))
        {
            var_ratio := shr(_2, mul(var_ratio, 0xa9f746462d870fdf8a65dc1f90e061e5))
        }
        if iszero(iszero(and(expr, 0x4000)))
        {
            var_ratio := shr(_2, mul(var_ratio, 0x70d869a156d2a1b890bb3df62baf32f7))
        }
        if iszero(iszero(and(expr, 0x8000)))
        {
            var_ratio := shr(_2, mul(var_ratio, 0x31be135f97d08fd981231505542fcfa6))
        }
        if iszero(iszero(and(expr, 0x010000)))
        {
            var_ratio := shr(_2, mul(var_ratio, 0x09aa508b5b7a84e1c677de54f3e99bc9))
        }
        if iszero(iszero(and(expr, 0x020000)))
        {
            var_ratio := shr(_2, mul(var_ratio, 0x5d6af8dedb81196699c329225ee604))
        }
        if iszero(iszero(and(expr, 0x040000)))
        {
            var_ratio := shr(_2, mul(var_ratio, 0x2216e584f5fa1ea926041bedfe98))
        }
        if iszero(iszero(and(expr, 0x080000)))
        {
            var_ratio := shr(_2, mul(var_ratio, 0x048a170391f7dc42444e8fa2))
        }
        if sgt(_1, 0x00)
        {
            var_ratio := checked_div_uint256_12692(var_ratio)
        }
        let expr_2 := 0x00
        switch iszero(and(var_ratio, 0xffffffff))
        case 0 { expr_2 := 0x01 }
        default { expr_2 := 0x00 }
        var_sqrtPriceX96 := and(add(shr(0x20, var_ratio), and(expr_2, 0xff)), sub(shl(160, 1), 1))
    }
}
// ====
// stackOptimization: true
// ----
