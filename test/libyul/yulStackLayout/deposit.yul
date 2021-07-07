    object "DepositContract_541_deployed" {
        code {
            {
                mstore(64, 128)
                if iszero(lt(calldatasize(), 4))
                {
                    let _1 := 0
                    switch shr(224, calldataload(_1))
                    case 0x01ffc9a7 {
                        if callvalue() { revert(_1, _1) }
                        return(128, add(abi_encode_bool(fun_supportsInterface(abi_decode_bytes4(calldatasize()))), not(127)))
                    }
                    case 0x22895118 {
                        let param, param_1, param_2, param_3, param_4, param_5, param_6 := abi_decode_bytes_calldatat_bytes_calldatat_bytes_calldatat_bytes32(calldatasize())
                        fun_deposit(param, param_1, param_2, param_3, param_4, param_5, param_6)
                        return(mload(64), _1)
                    }
                    case 0x621fd130 {
                        if callvalue() { revert(_1, _1) }
                        abi_decode(calldatasize())
                        let ret := fun_to_little_endian(convert_uint256_to_uint64(sload(0x20)))
                        let memPos := mload(64)
                        return(memPos, sub(abi_encode_bytes(memPos, ret), memPos))
                    }
                    case 0xc5f2892f {
                        if callvalue() { revert(_1, _1) }
                        abi_decode(calldatasize())
                        let ret_1 := fun_get_deposit_root()
                        let memPos_1 := mload(64)
                        return(memPos_1, sub(abi_encode_bytes32(memPos_1, ret_1), memPos_1))
                    }
                }
                revert(0, 0)
            }
            function abi_decode_bytes_calldata(offset, end) -> arrayPos, length
            {
                if iszero(slt(add(offset, 0x1f), end)) { revert(0, 0) }
                length := calldataload(offset)
                if gt(length, 0xffffffffffffffff) { revert(0, 0) }
                arrayPos := add(offset, 0x20)
                if gt(add(add(offset, length), 0x20), end) { revert(0, 0) }
            }
            function abi_decode(dataEnd)
            {
                if slt(add(dataEnd, not(3)), 0) { revert(0, 0) }
            }
            function abi_decode_bytes4(dataEnd) -> value0
            {
                if slt(add(dataEnd, not(3)), 32) { revert(0, 0) }
                let value := calldataload(4)
                if iszero(eq(value, and(value, shl(224, 0xffffffff)))) { revert(0, 0) }
                value0 := value
            }
            function abi_decode_bytes_calldatat_bytes_calldatat_bytes_calldatat_bytes32(dataEnd) -> value0, value1, value2, value3, value4, value5, value6
            {
                if slt(add(dataEnd, not(3)), 128) { revert(0, 0) }
                let offset := calldataload(4)
                let _1 := 0xffffffffffffffff
                if gt(offset, _1) { revert(0, 0) }
                let value0_1, value1_1 := abi_decode_bytes_calldata(add(4, offset), dataEnd)
                value0 := value0_1
                value1 := value1_1
                let offset_1 := calldataload(36)
                if gt(offset_1, _1) { revert(0, 0) }
                let value2_1, value3_1 := abi_decode_bytes_calldata(add(4, offset_1), dataEnd)
                value2 := value2_1
                value3 := value3_1
                let offset_2 := calldataload(68)
                if gt(offset_2, _1) { revert(0, 0) }
                let value4_1, value5_1 := abi_decode_bytes_calldata(add(4, offset_2), dataEnd)
                value4 := value4_1
                value5 := value5_1
                value6 := calldataload(100)
            }
            function abi_encode_bytes_calldata_ptr(start, length, pos) -> end
            {
                mstore(pos, length)
                calldatacopy(add(pos, 0x20), start, length)
                mstore(add(add(pos, length), 0x20), 0)
                end := add(add(pos, and(add(length, 31), not(31))), 0x20)
            }
            function abi_encode_bytes_calldata(start, length, pos) -> end
            {
                calldatacopy(pos, start, length)
                let _1 := add(pos, length)
                mstore(_1, 0)
                end := _1
            }
            function abi_encode_bytes_to_bytes(value, pos) -> end
            {
                let length := mload(value)
                mstore(pos, length)
                copy_memory_to_memory(add(value, 0x20), add(pos, 0x20), length)
                end := add(add(pos, and(add(length, 31), not(31))), 0x20)
            }
            function abi_encode_bytes_memory_ptr(value, pos) -> end
            {
                let length := mload(value)
                copy_memory_to_memory(add(value, 0x20), pos, length)
                end := add(pos, length)
            }
            function abi_encode_packed_bytes32_bytes32(pos, value0, value1) -> end
            {
                mstore(pos, value0)
                mstore(add(pos, 32), value1)
                end := add(pos, 64)
            }
            function abi_encode_packed_bytes32_bytes_calldata(pos, value0, value1, value2) -> end
            {
                mstore(pos, value0)
                calldatacopy(add(pos, 32), value1, value2)
                let _1 := add(add(pos, value2), 32)
                mstore(_1, 0)
                end := _1
            }
            function abi_encode_packed_bytes32_bytes_bytes24(pos, value0, value1) -> end
            {
                mstore(pos, value0)
                let length := mload(value1)
                copy_memory_to_memory(add(value1, 32), add(pos, 32), length)
                let _1 := add(pos, length)
                mstore(add(_1, 32), 0)
                end := add(_1, 56)
            }
            function abi_encode_packed_bytes_calldata_slice_bytes32(pos, value0, value1) -> end
            {
                calldatacopy(pos, value0, value1)
                let _1 := add(pos, value1)
                mstore(_1, 0x00)
                mstore(_1, 0x00)
                end := add(_1, 32)
            }
            function abi_encode_packed_bytes_calldata_bytes16(pos, value0, value1) -> end
            {
                calldatacopy(pos, value0, value1)
                let _1 := add(pos, value1)
                mstore(_1, 0)
                mstore(_1, 0)
                end := add(_1, 16)
            }
            function abi_encode_packed_bytes_bytes24_bytes32(pos, value0, value2) -> end
            {
                let length := mload(value0)
                copy_memory_to_memory(add(value0, 0x20), pos, length)
                let end_1 := add(pos, length)
                mstore(end_1, 0)
                mstore(add(end_1, 24), value2)
                end := add(end_1, 56)
            }
            function abi_encode_bool(value0) -> tail
            {
                tail := 160
                mstore(128, iszero(iszero(value0)))
            }
            function abi_encode_bytes32(headStart, value0) -> tail
            {
                tail := add(headStart, 32)
                mstore(headStart, value0)
            }
            function abi_encode_bytes_calldata_bytes_calldata_bytes_bytes_calldata_bytes(headStart, value0, value1, value2, value3, value4, value5, value6, value7) -> tail
            {
                mstore(headStart, 160)
                let tail_1 := abi_encode_bytes_calldata_ptr(value0, value1, add(headStart, 160))
                mstore(add(headStart, 32), sub(tail_1, headStart))
                let tail_2 := abi_encode_bytes_calldata_ptr(value2, value3, tail_1)
                mstore(add(headStart, 64), sub(tail_2, headStart))
                let tail_3 := abi_encode_bytes_to_bytes(value4, tail_2)
                mstore(add(headStart, 96), sub(tail_3, headStart))
                let tail_4 := abi_encode_bytes_calldata_ptr(value5, value6, tail_3)
                mstore(add(headStart, 128), sub(tail_4, headStart))
                tail := abi_encode_bytes_to_bytes(value7, tail_4)
            }
            function abi_encode_bytes(headStart, value0) -> tail
            {
                mstore(headStart, 32)
                tail := abi_encode_bytes_to_bytes(value0, add(headStart, 32))
            }
            function assert_helper()
            {
                mstore(0x00, shl(224, 0x4e487b71))
                mstore(4, 1)
                revert(0x00, 0x24)
            }
            function calldata_array_index_range_access_bytes_calldata(offset, length) -> offsetOut, lengthOut
            {
                if gt(64, length) { revert(0x00, 0x00) }
                offsetOut := offset
                lengthOut := 64
            }
            function calldata_array_index_range_access_bytes_calldata_3133(offset, length, endIndex) -> offsetOut, lengthOut
            {
                if gt(64, endIndex) { revert(0, 0) }
                if gt(endIndex, length) { revert(0, 0) }
                offsetOut := add(offset, 64)
                lengthOut := add(endIndex, not(63))
            }
            function checked_add_uint256(x) -> sum
            {
                if gt(x, not(1)) { panic_error_0x11() }
                sum := add(x, 0x01)
            }
            function convert_uint256_to_uint64(value) -> converted
            {
                converted := and(value, 0xffffffffffffffff)
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
            function extract_from_storage_value_dynamict_bytes32(slot_value, offset) -> value
            {
                value := shr(shl(3, offset), slot_value)
            }
            function finalize_allocation(memPtr, size)
            {
                let newFreePtr := add(memPtr, and(add(size, 31), not(31)))
                if or(gt(newFreePtr, 0xffffffffffffffff), lt(newFreePtr, memPtr)) { panic_error_0x41() }
                mstore(64, newFreePtr)
            }
            function fun_deposit(var_pubkey_offset, var_pubkey_length, var_withdrawal_credentials_offset, var_withdrawal_credentials_length, var_signature_offset, var_signature_length, var_deposit_data_root)
            {
                require_helper_stringliteral_c3b5(eq(var_pubkey_length, 0x30))
                let _1 := 0x20
                require_helper_stringliteral(eq(var_withdrawal_credentials_length, _1))
                require_helper_stringliteral_e15d(eq(var_signature_length, 0x60))
                require_helper_stringliteral_d4d9(iszero(lt(callvalue(), 0x0de0b6b3a7640000)))
                require_helper_stringliteral_7db0(iszero(mod(callvalue(), 0x3b9aca00)))
                let _2 := 0xffffffffffffffff
                require_helper_stringliteral_c445(iszero(gt(div(callvalue(), 0x3b9aca00), _2)))
                let expr_279_mpos := fun_to_little_endian(and(div(callvalue(), 0x3b9aca00), _2))
                let _3 := sload(_1)
                let expr_291_mpos := fun_to_little_endian(and(_3, _2))
                let _4 := mload(64)
                log1(_4, sub(abi_encode_bytes_calldata_bytes_calldata_bytes_bytes_calldata_bytes(_4, var_pubkey_offset, var_pubkey_length, var_withdrawal_credentials_offset, var_withdrawal_credentials_length, expr_279_mpos, var_signature_offset, var_signature_length, expr_291_mpos), _4), 0x649bbc62d0e31342afea4e5cd82d4049e7e1ee912fc0889aa790803be39038c5)
                let expr_304_mpos := mload(64)
                let _5 := sub(abi_encode_packed_bytes_calldata_bytes16(add(expr_304_mpos, _1), var_pubkey_offset, var_pubkey_length), expr_304_mpos)
                let _6 := not(31)
                mstore(expr_304_mpos, add(_5, _6))
                finalize_allocation(expr_304_mpos, _5)
                let _7 := mload(64)
                let _8 := sub(abi_encode_bytes_memory_ptr(expr_304_mpos, _7), _7)
                if iszero(staticcall(gas(), 2, _7, _8, 0x00, _1)) { revert_forward() }
                let _9 := mload(0x00)
                let expr_317_offset, expr_length := calldata_array_index_range_access_bytes_calldata(var_signature_offset, var_signature_length)
                let expr_mpos := mload(64)
                let _10 := sub(abi_encode_bytes_calldata(expr_317_offset, expr_length, add(expr_mpos, _1)), expr_mpos)
                mstore(expr_mpos, add(_10, _6))
                finalize_allocation(expr_mpos, _10)
                let _11 := mload(64)
                if iszero(staticcall(gas(), 2, _11, sub(abi_encode_bytes_memory_ptr(expr_mpos, _11), _11), 0x00, _1)) { revert_forward() }
                let _12 := mload(0x00)
                let expr_offset, expr_325_length := calldata_array_index_range_access_bytes_calldata_3133(var_signature_offset, var_signature_length, var_signature_length)
                let expr_330_mpos := mload(64)
                let _13 := sub(abi_encode_packed_bytes_calldata_slice_bytes32(add(expr_330_mpos, _1), expr_offset, expr_325_length), expr_330_mpos)
                mstore(expr_330_mpos, add(_13, _6))
                finalize_allocation(expr_330_mpos, _13)
                let _14 := mload(64)
                if iszero(staticcall(gas(), 2, _14, sub(abi_encode_bytes_memory_ptr(expr_330_mpos, _14), _14), 0x00, _1)) { revert_forward() }
                let _15 := mload(0x00)
                let expr_332_mpos := mload(64)
                let _16 := sub(abi_encode_packed_bytes32_bytes32(add(expr_332_mpos, _1), _12, _15), expr_332_mpos)
                mstore(expr_332_mpos, add(_16, _6))
                finalize_allocation(expr_332_mpos, _16)
                let _17 := mload(64)
                if iszero(staticcall(gas(), 2, _17, sub(abi_encode_bytes_memory_ptr(expr_332_mpos, _17), _17), 0x00, _1)) { revert_forward() }
                let _18 := mload(0x00)
                let expr_345_mpos := mload(64)
                let _19 := sub(abi_encode_packed_bytes32_bytes_calldata(add(expr_345_mpos, _1), _9, var_withdrawal_credentials_offset, var_withdrawal_credentials_length), expr_345_mpos)
                mstore(expr_345_mpos, add(_19, _6))
                finalize_allocation(expr_345_mpos, _19)
                let _20 := mload(64)
                if iszero(staticcall(gas(), 2, _20, sub(abi_encode_bytes_memory_ptr(expr_345_mpos, _20), _20), 0x00, _1)) { revert_forward() }
                let _21 := mload(0x00)
                let expr_356_mpos := mload(64)
                let _22 := sub(abi_encode_packed_bytes_bytes24_bytes32(add(expr_356_mpos, _1), expr_279_mpos, _18), expr_356_mpos)
                mstore(expr_356_mpos, add(_22, _6))
                finalize_allocation(expr_356_mpos, _22)
                let _23 := mload(64)
                if iszero(staticcall(gas(), 2, _23, sub(abi_encode_bytes_memory_ptr(expr_356_mpos, _23), _23), 0x00, _1)) { revert_forward() }
                let _24 := mload(0x00)
                let expr_358_mpos := mload(64)
                let _25 := sub(abi_encode_packed_bytes32_bytes32(add(expr_358_mpos, _1), _21, _24), expr_358_mpos)
                mstore(expr_358_mpos, add(_25, _6))
                finalize_allocation(expr_358_mpos, _25)
                let _26 := mload(64)
                if iszero(staticcall(gas(), 2, _26, sub(abi_encode_bytes_memory_ptr(expr_358_mpos, _26), _26), 0x00, _1)) { revert_forward() }
                let var_node := mload(0x00)
                require_helper_stringliteral_18a6(eq(var_node, var_deposit_data_root))
                require_helper_stringliteral_122a(lt(_3, 0xffffffff))
                update_storage_value_offsett_uint256_to_uint256(checked_add_uint256(_3))
                let var_size := sload(_1)
                let var_height := 0x00
                for { }
                lt(var_height, _1)
                {
                    var_height := increment_uint256(var_height)
                }
                {
                    if eq(and(var_size, 0x01), 0x01)
                    {
                        let _27, _28 := storage_array_index_access_bytes_3138(var_height)
                        update_storage_value_bytes32_to_bytes32(_27, _28, var_node)
                        leave
                    }
                    let _29, _30 := storage_array_index_access_bytes_3138(var_height)
                    let _31 := extract_from_storage_value_dynamict_bytes32(sload(_29), _30)
                    let expr_416_mpos := mload(64)
                    let _32 := sub(abi_encode_packed_bytes32_bytes32(add(expr_416_mpos, _1), _31, var_node), expr_416_mpos)
                    mstore(expr_416_mpos, add(_32, _6))
                    finalize_allocation(expr_416_mpos, _32)
                    let _33 := mload(64)
                    if iszero(staticcall(gas(), 2, _33, sub(abi_encode_bytes_memory_ptr(expr_416_mpos, _33), _33), 0x00, _1)) { revert_forward() }
                    var_node := mload(0x00)
                    var_size := shr(0x01, var_size)
                }
                assert_helper()
            }
            function fun_get_deposit_root() -> var
            {
                let var_node := 0
                let _1 := 0x20
                let var_size := sload(_1)
                let var_size_1 := var_size
                let var_height := var_node
                for { }
                lt(var_height, _1)
                {
                    var_height := increment_uint256(var_height)
                }
                {
                    let _2 := 0x01
                    switch eq(and(var_size, _2), _2)
                    case 0 {
                        let _3, _4 := storage_array_index_access_bytes_3142(var_height)
                        let _5 := extract_from_storage_value_dynamict_bytes32(sload(_3), _4)
                        let _6 := 64
                        let expr_157_mpos := mload(_6)
                        let _7 := sub(abi_encode_packed_bytes32_bytes32(add(expr_157_mpos, _1), var_node, _5), expr_157_mpos)
                        mstore(expr_157_mpos, add(_7, not(31)))
                        finalize_allocation(expr_157_mpos, _7)
                        let _8 := mload(_6)
                        if iszero(staticcall(gas(), 0x02, _8, sub(abi_encode_bytes_memory_ptr(expr_157_mpos, _8), _8), 0, _1)) { revert_forward() }
                        var_node := mload(0)
                    }
                    default {
                        let _9, _10 := storage_array_index_access_bytes(var_height)
                        let _11 := extract_from_storage_value_dynamict_bytes32(sload(_9), _10)
                        let _12 := 64
                        let expr_145_mpos := mload(_12)
                        let _13 := sub(abi_encode_packed_bytes32_bytes32(add(expr_145_mpos, _1), _11, var_node), expr_145_mpos)
                        mstore(expr_145_mpos, add(_13, not(31)))
                        finalize_allocation(expr_145_mpos, _13)
                        let _14 := mload(_12)
                        if iszero(staticcall(gas(), 0x02, _14, sub(abi_encode_bytes_memory_ptr(expr_145_mpos, _14), _14), 0, _1)) { revert_forward() }
                        var_node := mload(0)
                    }
                    var_size := shr(_2, var_size)
                }
                let expr_177_mpos := fun_to_little_endian(and(var_size_1, 0xffffffffffffffff))
                let expr_182_mpos := mload(64)
                let _15 := sub(abi_encode_packed_bytes32_bytes_bytes24(add(expr_182_mpos, _1), var_node, expr_177_mpos), expr_182_mpos)
                mstore(expr_182_mpos, add(_15, not(31)))
                finalize_allocation(expr_182_mpos, _15)
                let _16 := mload(64)
                if iszero(staticcall(gas(), 0x02, _16, sub(abi_encode_bytes_memory_ptr(expr_182_mpos, _16), _16), 0, _1)) { revert_forward() }
                var := mload(0)
            }
            function fun_supportsInterface(var_interfaceId) -> var_
            {
                let _1 := and(var_interfaceId, shl(224, 0xffffffff))
                let expr := eq(_1, shl(224, 0x01ffc9a7))
                if iszero(expr)
                {
                    expr := eq(_1, shl(224, 0x85640907))
                }
                var_ := expr
            }
            function fun_to_little_endian(var_value) -> var_ret_mpos
            {
                let memPtr := mload(64)
                let newFreePtr := add(memPtr, 64)
                if or(gt(newFreePtr, 0xffffffffffffffff), lt(newFreePtr, memPtr)) { panic_error_0x41() }
                mstore(64, newFreePtr)
                mstore(memPtr, 0x08)
                calldatacopy(add(memPtr, 0x20), calldatasize(), 0x20)
                var_ret_mpos := memPtr
                let expr := and(shl(192, var_value), shl(192, 0xffffffffffffffff))
                mstore8(memory_array_index_access_bytes_3147(memPtr), byte(0x07, expr))
                mstore8(memory_array_index_access_bytes_3148(memPtr), byte(0x06, expr))
                mstore8(memory_array_index_access_bytes_3149(memPtr), byte(0x05, expr))
                mstore8(memory_array_index_access_bytes_3150(memPtr), byte(0x04, expr))
                mstore8(memory_array_index_access_bytes_3151(memPtr), byte(0x03, expr))
                mstore8(memory_array_index_access_bytes_3152(memPtr), byte(0x02, expr))
                mstore8(memory_array_index_access_bytes_3153(memPtr), byte(1, expr))
                mstore8(memory_array_index_access_bytes(memPtr), byte(0, expr))
            }
            function increment_uint256(value) -> ret
            {
                if eq(value, not(0)) { panic_error_0x11() }
                ret := add(value, 1)
            }
            function memory_array_index_access_bytes_3147(baseRef) -> addr
            {
                if iszero(mload(baseRef)) { panic_error_0x32() }
                addr := add(baseRef, 32)
            }
            function memory_array_index_access_bytes_3148(baseRef) -> addr
            {
                if iszero(lt(0x01, mload(baseRef))) { panic_error_0x32() }
                addr := add(baseRef, 33)
            }
            function memory_array_index_access_bytes_3149(baseRef) -> addr
            {
                if iszero(lt(0x02, mload(baseRef))) { panic_error_0x32() }
                addr := add(baseRef, 34)
            }
            function memory_array_index_access_bytes_3150(baseRef) -> addr
            {
                if iszero(lt(0x03, mload(baseRef))) { panic_error_0x32() }
                addr := add(baseRef, 35)
            }
            function memory_array_index_access_bytes_3151(baseRef) -> addr
            {
                if iszero(lt(0x04, mload(baseRef))) { panic_error_0x32() }
                addr := add(baseRef, 36)
            }
            function memory_array_index_access_bytes_3152(baseRef) -> addr
            {
                if iszero(lt(0x05, mload(baseRef))) { panic_error_0x32() }
                addr := add(baseRef, 37)
            }
            function memory_array_index_access_bytes_3153(baseRef) -> addr
            {
                if iszero(lt(0x06, mload(baseRef))) { panic_error_0x32() }
                addr := add(baseRef, 38)
            }
            function memory_array_index_access_bytes(baseRef) -> addr
            {
                if iszero(lt(0x07, mload(baseRef))) { panic_error_0x32() }
                addr := add(baseRef, 39)
            }
            function panic_error_0x11()
            {
                mstore(0, shl(224, 0x4e487b71))
                mstore(4, 0x11)
                revert(0, 0x24)
            }
            function panic_error_0x32()
            {
                mstore(0, shl(224, 0x4e487b71))
                mstore(4, 0x32)
                revert(0, 0x24)
            }
            function panic_error_0x41()
            {
                mstore(0, shl(224, 0x4e487b71))
                mstore(4, 0x41)
                revert(0, 0x24)
            }
            function require_helper_stringliteral_122a(condition)
            {
                if iszero(condition)
                {
                    let memPtr := mload(64)
                    mstore(memPtr, shl(229, 4594637))
                    mstore(add(memPtr, 4), 32)
                    mstore(add(memPtr, 36), 33)
                    mstore(add(memPtr, 68), "DepositContract: merkle tree ful")
                    mstore(add(memPtr, 100), "l")
                    revert(memPtr, 132)
                }
            }
            function require_helper_stringliteral_18a6(condition)
            {
                if iszero(condition)
                {
                    let memPtr := mload(64)
                    mstore(memPtr, shl(229, 4594637))
                    mstore(add(memPtr, 4), 32)
                    mstore(add(memPtr, 36), 84)
                    mstore(add(memPtr, 68), "DepositContract: reconstructed D")
                    mstore(add(memPtr, 100), "epositData does not match suppli")
                    mstore(add(memPtr, 132), "ed deposit_data_root")
                    revert(memPtr, 164)
                }
            }
            function require_helper_stringliteral(condition)
            {
                if iszero(condition)
                {
                    let memPtr := mload(64)
                    mstore(memPtr, shl(229, 4594637))
                    mstore(add(memPtr, 4), 32)
                    mstore(add(memPtr, 36), 54)
                    mstore(add(memPtr, 68), "DepositContract: invalid withdra")
                    mstore(add(memPtr, 100), "wal_credentials length")
                    revert(memPtr, 132)
                }
            }
            function require_helper_stringliteral_7db0(condition)
            {
                if iszero(condition)
                {
                    let memPtr := mload(64)
                    mstore(memPtr, shl(229, 4594637))
                    mstore(add(memPtr, 4), 32)
                    mstore(add(memPtr, 36), 51)
                    mstore(add(memPtr, 68), "DepositContract: deposit value n")
                    mstore(add(memPtr, 100), "ot multiple of gwei")
                    revert(memPtr, 132)
                }
            }
            function require_helper_stringliteral_c3b5(condition)
            {
                if iszero(condition)
                {
                    let memPtr := mload(64)
                    mstore(memPtr, shl(229, 4594637))
                    mstore(add(memPtr, 4), 32)
                    mstore(add(memPtr, 36), 38)
                    mstore(add(memPtr, 68), "DepositContract: invalid pubkey ")
                    mstore(add(memPtr, 100), "length")
                    revert(memPtr, 132)
                }
            }
            function require_helper_stringliteral_c445(condition)
            {
                if iszero(condition)
                {
                    let memPtr := mload(64)
                    mstore(memPtr, shl(229, 4594637))
                    mstore(add(memPtr, 4), 32)
                    mstore(add(memPtr, 36), 39)
                    mstore(add(memPtr, 68), "DepositContract: deposit value t")
                    mstore(add(memPtr, 100), "oo high")
                    revert(memPtr, 132)
                }
            }
            function require_helper_stringliteral_d4d9(condition)
            {
                if iszero(condition)
                {
                    let memPtr := mload(64)
                    mstore(memPtr, shl(229, 4594637))
                    mstore(add(memPtr, 4), 32)
                    mstore(add(memPtr, 36), 38)
                    mstore(add(memPtr, 68), "DepositContract: deposit value t")
                    mstore(add(memPtr, 100), "oo low")
                    revert(memPtr, 132)
                }
            }
            function require_helper_stringliteral_e15d(condition)
            {
                if iszero(condition)
                {
                    let memPtr := mload(64)
                    mstore(memPtr, shl(229, 4594637))
                    mstore(add(memPtr, 4), 32)
                    mstore(add(memPtr, 36), 41)
                    mstore(add(memPtr, 68), "DepositContract: invalid signatu")
                    mstore(add(memPtr, 100), "re length")
                    revert(memPtr, 132)
                }
            }
            function revert_forward()
            {
                let pos := mload(64)
                returndatacopy(pos, 0, returndatasize())
                revert(pos, returndatasize())
            }
            function storage_array_index_access_bytes_3138(index) -> slot, offset
            {
                if iszero(lt(index, 0x20)) { panic_error_0x32() }
                slot := index
                offset := 0x00
            }
            function storage_array_index_access_bytes_3142(index) -> slot, offset
            {
                if iszero(lt(index, 0x20)) { panic_error_0x32() }
                slot := add(0x21, index)
                offset := 0
            }
            function storage_array_index_access_bytes(index) -> slot, offset
            {
                if iszero(lt(index, 0x20)) { panic_error_0x32() }
                slot := index
                offset := 0
            }
            function update_storage_value_offsett_uint256_to_uint256(value)
            { sstore(0x20, value) }
            function update_storage_value_bytes32_to_bytes32(slot, offset, value)
            {
                let _1 := sload(slot)
                let shiftBits := shl(3, offset)
                let mask := shl(shiftBits, not(0))
                sstore(slot, or(and(_1, not(mask)), and(shl(shiftBits, value), mask)))
            }
        }
        }
// ----
