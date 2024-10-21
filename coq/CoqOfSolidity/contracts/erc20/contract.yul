Optimized IR:
/// @use-src 0:"CoqOfSolidity/contracts/erc20/contract.sol"
object "Erc20_403" {
    code {
        {
            /// @src 0:65:3516  "contract Erc20 {..."
            let _1 := memoryguard(0x80)
            mstore(64, _1)
            if callvalue() { revert(0, 0) }
            if /** @src 0:2363:2384  "account != address(0)" */ iszero(/** @src 0:439:449  "msg.sender" */ caller())
            /// @src 0:65:3516  "contract Erc20 {..."
            {
                mstore(_1, shl(229, 4594637))
                mstore(add(_1, 4), 32)
                mstore(add(_1, 36), 31)
                mstore(add(_1, 68), "Erc20: mint to the zero address")
                revert(_1, 100)
            }
            sstore(/** @src 0:2501:2513  "_totalSupply" */ 0x02, /** @src 0:2501:2521  "_totalSupply + value" */ checked_add_uint256(/** @src 0:65:3516  "contract Erc20 {..." */ sload(/** @src 0:2501:2513  "_totalSupply" */ 0x02)))
            /// @src 0:65:3516  "contract Erc20 {..."
            mstore(/** @src 0:2382:2383  "0" */ 0x00, /** @src 0:439:449  "msg.sender" */ caller())
            /// @src 0:65:3516  "contract Erc20 {..."
            mstore(0x20, /** @src 0:2382:2383  "0" */ 0x00)
            /// @src 0:2552:2578  "_balances[account] + value"
            let _2 := checked_add_uint256(/** @src 0:65:3516  "contract Erc20 {..." */ sload(keccak256(/** @src 0:2382:2383  "0" */ 0x00, /** @src 0:65:3516  "contract Erc20 {..." */ 64)))
            mstore(/** @src 0:2382:2383  "0" */ 0x00, /** @src 0:439:449  "msg.sender" */ caller())
            /// @src 0:65:3516  "contract Erc20 {..."
            mstore(0x20, /** @src 0:2382:2383  "0" */ 0x00)
            /// @src 0:65:3516  "contract Erc20 {..."
            sstore(keccak256(/** @src 0:2382:2383  "0" */ 0x00, /** @src 0:65:3516  "contract Erc20 {..." */ 64), _2)
            /// @src 0:2593:2629  "Transfer(address(0), account, value)"
            let _3 := /** @src 0:65:3516  "contract Erc20 {..." */ mload(64)
            mstore(_3, /** @src 0:451:453  "20" */ 0x14)
            /// @src 0:2593:2629  "Transfer(address(0), account, value)"
            log3(_3, /** @src 0:65:3516  "contract Erc20 {..." */ 0x20, /** @src 0:2593:2629  "Transfer(address(0), account, value)" */ 0xddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef, /** @src 0:2382:2383  "0" */ 0x00, /** @src 0:439:449  "msg.sender" */ caller())
            /// @src 0:65:3516  "contract Erc20 {..."
            let _4 := mload(64)
            let _5 := datasize("Erc20_403_deployed")
            codecopy(_4, dataoffset("Erc20_403_deployed"), _5)
            return(_4, _5)
        }
        function checked_add_uint256(x) -> sum
        {
            sum := add(x, /** @src 0:451:453  "20" */ 0x14)
            /// @src 0:65:3516  "contract Erc20 {..."
            if gt(x, sum)
            {
                mstore(0, shl(224, 0x4e487b71))
                mstore(4, 0x11)
                revert(0, 0x24)
            }
        }
    }
    /// @use-src 0:"CoqOfSolidity/contracts/erc20/contract.sol"
    object "Erc20_403_deployed" {
        code {
            {
                /// @src 0:65:3516  "contract Erc20 {..."
                mstore(64, memoryguard(0x80))
                if iszero(lt(calldatasize(), 4))
                {
                    switch shr(224, calldataload(0))
                    case 0x095ea7b3 {
                        if callvalue() { revert(0, 0) }
                        if slt(add(calldatasize(), not(3)), 64) { revert(0, 0) }
                        let value0 := abi_decode_address()
                        /// @src 0:1064:1069  "value"
                        fun_approve(/** @src 0:1043:1053  "msg.sender" */ caller(), /** @src 0:1064:1069  "value" */ value0, /** @src 0:65:3516  "contract Erc20 {..." */ calldataload(36))
                        let memPos := mload(64)
                        mstore(memPos, 1)
                        return(memPos, 32)
                    }
                    case 0x18160ddd {
                        if callvalue() { revert(0, 0) }
                        if slt(add(calldatasize(), not(3)), 0) { revert(0, 0) }
                        let _1 := sload(/** @src 0:537:549  "_totalSupply" */ 0x02)
                        /// @src 0:65:3516  "contract Erc20 {..."
                        let memPos_1 := mload(64)
                        mstore(memPos_1, _1)
                        return(memPos_1, 32)
                    }
                    case 0x23b872dd {
                        if callvalue() { revert(0, 0) }
                        if slt(add(calldatasize(), not(3)), 96) { revert(0, 0) }
                        let value0_1 := abi_decode_address()
                        let value1 := abi_decode_address_2305()
                        let value := calldataload(68)
                        /// @src 0:1219:1224  "value"
                        fun_transfer(value0_1, value1, value)
                        /// @src 0:65:3516  "contract Erc20 {..."
                        mstore(0, and(value0_1, sub(shl(160, 1), 1)))
                        mstore(32, 1)
                        let dataSlot := keccak256(0, 64)
                        /// @src 0:1319:1348  "_allowances[from][msg.sender]"
                        let dataSlot_1 := /** @src -1:-1:-1 */ 0
                        /// @src 0:65:3516  "contract Erc20 {..."
                        mstore(/** @src -1:-1:-1 */ 0, /** @src 0:65:3516  "contract Erc20 {..." */ and(/** @src 0:1307:1317  "msg.sender" */ caller(), /** @src 0:65:3516  "contract Erc20 {..." */ sub(shl(160, 1), 1)))
                        mstore(0x20, /** @src 0:1319:1336  "_allowances[from]" */ dataSlot)
                        /// @src 0:65:3516  "contract Erc20 {..."
                        dataSlot_1 := keccak256(/** @src -1:-1:-1 */ 0, /** @src 0:65:3516  "contract Erc20 {..." */ 0x40)
                        /// @src 0:1319:1356  "_allowances[from][msg.sender] - value"
                        fun_approve(value0_1, /** @src 0:1307:1317  "msg.sender" */ caller(), /** @src 0:1319:1356  "_allowances[from][msg.sender] - value" */ checked_sub_uint256(/** @src 0:65:3516  "contract Erc20 {..." */ sload(/** @src 0:1319:1348  "_allowances[from][msg.sender]" */ dataSlot_1), /** @src 0:1319:1356  "_allowances[from][msg.sender] - value" */ value))
                        /// @src 0:65:3516  "contract Erc20 {..."
                        let memPos_2 := mload(64)
                        mstore(memPos_2, 1)
                        return(memPos_2, 32)
                    }
                    case 0x39509351 {
                        if callvalue() { revert(0, 0) }
                        if slt(add(calldatasize(), not(3)), 64) { revert(0, 0) }
                        let value0_2 := abi_decode_address()
                        mstore(0, /** @src 0:1550:1560  "msg.sender" */ caller())
                        /// @src 0:65:3516  "contract Erc20 {..."
                        mstore(32, 1)
                        let dataSlot_2 := keccak256(0, 64)
                        /// @src 0:1571:1603  "_allowances[msg.sender][spender]"
                        let dataSlot_3 := /** @src -1:-1:-1 */ 0
                        /// @src 0:65:3516  "contract Erc20 {..."
                        mstore(/** @src -1:-1:-1 */ 0, /** @src 0:65:3516  "contract Erc20 {..." */ and(/** @src 0:1571:1603  "_allowances[msg.sender][spender]" */ value0_2, /** @src 0:65:3516  "contract Erc20 {..." */ sub(shl(160, 1), 1)))
                        mstore(0x20, /** @src 0:1571:1594  "_allowances[msg.sender]" */ dataSlot_2)
                        /// @src 0:65:3516  "contract Erc20 {..."
                        dataSlot_3 := keccak256(/** @src -1:-1:-1 */ 0, /** @src 0:65:3516  "contract Erc20 {..." */ 0x40)
                        /// @src 0:1571:1616  "_allowances[msg.sender][spender] + addedValue"
                        fun_approve(/** @src 0:1550:1560  "msg.sender" */ caller(), /** @src 0:1571:1616  "_allowances[msg.sender][spender] + addedValue" */ value0_2, checked_add_uint256(/** @src 0:65:3516  "contract Erc20 {..." */ sload(/** @src 0:1571:1603  "_allowances[msg.sender][spender]" */ dataSlot_3), /** @src 0:65:3516  "contract Erc20 {..." */ calldataload(36)))
                        let memPos_3 := mload(64)
                        mstore(memPos_3, 1)
                        return(memPos_3, 32)
                    }
                    case 0x70a08231 {
                        if callvalue() { revert(0, 0) }
                        if slt(add(calldatasize(), not(3)), 32) { revert(0, 0) }
                        mstore(0, and(abi_decode_address(), sub(shl(160, 1), 1)))
                        mstore(32, 0)
                        let _2 := sload(keccak256(0, 64))
                        let memPos_4 := mload(64)
                        mstore(memPos_4, _2)
                        return(memPos_4, 32)
                    }
                    case 0xa457c2d7 {
                        if callvalue() { revert(0, 0) }
                        if slt(add(calldatasize(), not(3)), 64) { revert(0, 0) }
                        let value0_3 := abi_decode_address()
                        mstore(0, /** @src 0:1818:1828  "msg.sender" */ caller())
                        /// @src 0:65:3516  "contract Erc20 {..."
                        mstore(32, 1)
                        let dataSlot_4 := keccak256(0, 64)
                        /// @src 0:1839:1871  "_allowances[msg.sender][spender]"
                        let dataSlot_5 := /** @src -1:-1:-1 */ 0
                        /// @src 0:65:3516  "contract Erc20 {..."
                        mstore(/** @src -1:-1:-1 */ 0, /** @src 0:65:3516  "contract Erc20 {..." */ and(/** @src 0:1839:1871  "_allowances[msg.sender][spender]" */ value0_3, /** @src 0:65:3516  "contract Erc20 {..." */ sub(shl(160, 1), 1)))
                        mstore(0x20, /** @src 0:1839:1862  "_allowances[msg.sender]" */ dataSlot_4)
                        /// @src 0:65:3516  "contract Erc20 {..."
                        dataSlot_5 := keccak256(/** @src -1:-1:-1 */ 0, /** @src 0:65:3516  "contract Erc20 {..." */ 0x40)
                        /// @src 0:1839:1889  "_allowances[msg.sender][spender] - subtractedValue"
                        fun_approve(/** @src 0:1818:1828  "msg.sender" */ caller(), /** @src 0:1839:1889  "_allowances[msg.sender][spender] - subtractedValue" */ value0_3, checked_sub_uint256(/** @src 0:65:3516  "contract Erc20 {..." */ sload(/** @src 0:1839:1871  "_allowances[msg.sender][spender]" */ dataSlot_5), /** @src 0:65:3516  "contract Erc20 {..." */ calldataload(36)))
                        let memPos_5 := mload(64)
                        mstore(memPos_5, 1)
                        return(memPos_5, 32)
                    }
                    case 0xa9059cbb {
                        if callvalue() { revert(0, 0) }
                        if slt(add(calldatasize(), not(3)), 64) { revert(0, 0) }
                        let value0_4 := abi_decode_address()
                        /// @src 0:913:918  "value"
                        fun_transfer(/** @src 0:897:907  "msg.sender" */ caller(), /** @src 0:913:918  "value" */ value0_4, /** @src 0:65:3516  "contract Erc20 {..." */ calldataload(36))
                        let memPos_6 := mload(64)
                        mstore(memPos_6, 1)
                        return(memPos_6, 32)
                    }
                    case 0xdd62ed3e {
                        if callvalue() { revert(0, 0) }
                        if slt(add(calldatasize(), not(3)), 64) { revert(0, 0) }
                        let value0_5 := abi_decode_address()
                        let value1_1 := abi_decode_address_2305()
                        mstore(0, and(value0_5, sub(shl(160, 1), 1)))
                        mstore(32, /** @src 0:770:781  "_allowances" */ 0x01)
                        /// @src 0:65:3516  "contract Erc20 {..."
                        let dataSlot_6 := keccak256(0, 64)
                        /// @src 0:770:797  "_allowances[owner][spender]"
                        let dataSlot_7 := /** @src -1:-1:-1 */ 0
                        /// @src 0:65:3516  "contract Erc20 {..."
                        mstore(/** @src -1:-1:-1 */ 0, /** @src 0:65:3516  "contract Erc20 {..." */ and(value1_1, sub(shl(160, 1), 1)))
                        mstore(0x20, /** @src 0:770:788  "_allowances[owner]" */ dataSlot_6)
                        /// @src 0:65:3516  "contract Erc20 {..."
                        dataSlot_7 := keccak256(/** @src -1:-1:-1 */ 0, /** @src 0:65:3516  "contract Erc20 {..." */ 0x40)
                        let _3 := sload(/** @src 0:770:797  "_allowances[owner][spender]" */ dataSlot_7)
                        /// @src 0:65:3516  "contract Erc20 {..."
                        let memPos_7 := mload(64)
                        mstore(memPos_7, _3)
                        return(memPos_7, 32)
                    }
                }
                revert(0, 0)
            }
            function abi_decode_address() -> value
            {
                value := calldataload(4)
                if iszero(eq(value, and(value, sub(shl(160, 1), 1)))) { revert(0, 0) }
            }
            function abi_decode_address_2305() -> value
            {
                value := calldataload(36)
                if iszero(eq(value, and(value, sub(shl(160, 1), 1)))) { revert(0, 0) }
            }
            function checked_sub_uint256(x, y) -> diff
            {
                diff := sub(x, y)
                if gt(diff, x)
                {
                    mstore(0, shl(224, 0x4e487b71))
                    mstore(4, 0x11)
                    revert(0, 0x24)
                }
            }
            function checked_add_uint256(x, y) -> sum
            {
                sum := add(x, y)
                if gt(x, sum)
                {
                    mstore(0, shl(224, 0x4e487b71))
                    mstore(4, 0x11)
                    revert(0, 0x24)
                }
            }
            /// @ast-id 375 @src 0:3000:3329  "function _approve(address owner, address spender, uint256 value) internal {..."
            function fun_approve(var_owner, var_spender, var_value)
            {
                /// @src 0:65:3516  "contract Erc20 {..."
                let _1 := and(/** @src 0:3092:3111  "owner != address(0)" */ var_owner, /** @src 0:65:3516  "contract Erc20 {..." */ sub(shl(160, 1), 1))
                if /** @src 0:3092:3111  "owner != address(0)" */ iszero(/** @src 0:65:3516  "contract Erc20 {..." */ _1)
                {
                    let memPtr := mload(64)
                    mstore(memPtr, shl(229, 4594637))
                    mstore(add(memPtr, 4), 32)
                    mstore(add(memPtr, 36), 36)
                    mstore(add(memPtr, 68), "Erc20: approve from the zero add")
                    mstore(add(memPtr, 100), "ress")
                    revert(memPtr, 132)
                }
                let _2 := and(/** @src 0:3170:3191  "spender != address(0)" */ var_spender, /** @src 0:65:3516  "contract Erc20 {..." */ sub(shl(160, 1), 1))
                if /** @src 0:3170:3191  "spender != address(0)" */ iszero(/** @src 0:65:3516  "contract Erc20 {..." */ _2)
                {
                    let memPtr_1 := mload(64)
                    mstore(memPtr_1, shl(229, 4594637))
                    mstore(add(memPtr_1, 4), 32)
                    mstore(add(memPtr_1, 36), 34)
                    mstore(add(memPtr_1, 68), "Erc20: approve to the zero addre")
                    mstore(add(memPtr_1, 100), "ss")
                    revert(memPtr_1, 132)
                }
                mstore(/** @src 0:3109:3110  "0" */ 0x00, /** @src 0:65:3516  "contract Erc20 {..." */ _1)
                mstore(0x20, /** @src 0:3241:3252  "_allowances" */ 0x01)
                /// @src 0:65:3516  "contract Erc20 {..."
                let dataSlot := keccak256(/** @src 0:3109:3110  "0" */ 0x00, /** @src 0:65:3516  "contract Erc20 {..." */ 0x40)
                /// @src 0:3241:3268  "_allowances[owner][spender]"
                let dataSlot_1 := /** @src -1:-1:-1 */ 0
                /// @src 0:65:3516  "contract Erc20 {..."
                mstore(/** @src -1:-1:-1 */ 0, /** @src 0:65:3516  "contract Erc20 {..." */ _2)
                mstore(0x20, /** @src 0:3241:3259  "_allowances[owner]" */ dataSlot)
                /// @src 0:65:3516  "contract Erc20 {..."
                dataSlot_1 := keccak256(/** @src -1:-1:-1 */ 0, /** @src 0:65:3516  "contract Erc20 {..." */ 0x40)
                sstore(/** @src 0:3241:3268  "_allowances[owner][spender]" */ dataSlot_1, /** @src 0:65:3516  "contract Erc20 {..." */ var_value)
                /// @src 0:3291:3322  "Approval(owner, spender, value)"
                let _3 := /** @src 0:65:3516  "contract Erc20 {..." */ mload(0x40)
                mstore(_3, var_value)
                /// @src 0:3291:3322  "Approval(owner, spender, value)"
                log3(_3, /** @src 0:65:3516  "contract Erc20 {..." */ 0x20, /** @src 0:3291:3322  "Approval(owner, spender, value)" */ 0x8c5be1e5ebec7d5bd14f71427d1e84f3dd0314c0f7b2291e5b200ac8c7c3b925, _1, _2)
            }
            /// @ast-id 245 @src 0:1924:2283  "function _transfer(address from, address to, uint256 value) internal {..."
            function fun_transfer(var_from, var_to, var_value)
            {
                /// @src 0:65:3516  "contract Erc20 {..."
                let _1 := and(/** @src 0:2011:2027  "to != address(0)" */ var_to, /** @src 0:65:3516  "contract Erc20 {..." */ sub(shl(160, 1), 1))
                if /** @src 0:2011:2027  "to != address(0)" */ iszero(/** @src 0:65:3516  "contract Erc20 {..." */ _1)
                {
                    let memPtr := mload(64)
                    mstore(memPtr, shl(229, 4594637))
                    mstore(add(memPtr, 4), 32)
                    mstore(add(memPtr, 36), 35)
                    mstore(add(memPtr, 68), "Erc20: transfer to the zero addr")
                    mstore(add(memPtr, 100), "ess")
                    revert(memPtr, 132)
                }
                let _2 := and(var_from, sub(shl(160, 1), 1))
                mstore(/** @src 0:2025:2026  "0" */ 0x00, /** @src 0:65:3516  "contract Erc20 {..." */ _2)
                mstore(0x20, /** @src 0:2025:2026  "0" */ 0x00)
                /// @src 0:2166:2189  "_balances[from] - value"
                let _3 := checked_sub_uint256(/** @src 0:65:3516  "contract Erc20 {..." */ sload(keccak256(/** @src 0:2025:2026  "0" */ 0x00, /** @src 0:65:3516  "contract Erc20 {..." */ 0x40)), /** @src 0:2166:2189  "_balances[from] - value" */ var_value)
                /// @src 0:65:3516  "contract Erc20 {..."
                mstore(/** @src 0:2025:2026  "0" */ 0x00, /** @src 0:65:3516  "contract Erc20 {..." */ _2)
                mstore(0x20, /** @src 0:2025:2026  "0" */ 0x00)
                /// @src 0:65:3516  "contract Erc20 {..."
                sstore(keccak256(/** @src 0:2025:2026  "0" */ 0x00, /** @src 0:65:3516  "contract Erc20 {..." */ 0x40), _3)
                mstore(/** @src 0:2025:2026  "0" */ 0x00, /** @src 0:65:3516  "contract Erc20 {..." */ _1)
                mstore(0x20, /** @src 0:2025:2026  "0" */ 0x00)
                /// @src 0:2215:2236  "_balances[to] + value"
                let _4 := checked_add_uint256(/** @src 0:65:3516  "contract Erc20 {..." */ sload(keccak256(/** @src 0:2025:2026  "0" */ 0x00, /** @src 0:65:3516  "contract Erc20 {..." */ 0x40)), /** @src 0:2215:2236  "_balances[to] + value" */ var_value)
                /// @src 0:65:3516  "contract Erc20 {..."
                mstore(/** @src 0:2025:2026  "0" */ 0x00, /** @src 0:65:3516  "contract Erc20 {..." */ _1)
                mstore(0x20, /** @src 0:2025:2026  "0" */ 0x00)
                /// @src 0:65:3516  "contract Erc20 {..."
                sstore(keccak256(/** @src 0:2025:2026  "0" */ 0x00, /** @src 0:65:3516  "contract Erc20 {..." */ 0x40), _4)
                /// @src 0:2251:2276  "Transfer(from, to, value)"
                let _5 := /** @src 0:65:3516  "contract Erc20 {..." */ mload(0x40)
                mstore(_5, var_value)
                /// @src 0:2251:2276  "Transfer(from, to, value)"
                log3(_5, /** @src 0:65:3516  "contract Erc20 {..." */ 0x20, /** @src 0:2251:2276  "Transfer(from, to, value)" */ 0xddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef, _2, _1)
            }
        }
        data ".metadata" hex"a26469706673582212201a1fe65a5b6e9a889cfa2e0e3e08138191bebe93076641e37824cf6a4c5c784264736f6c634300081b0033"
    }
}

