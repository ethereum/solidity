Optimized IR:
/// @use-src 0:"CoqOfSolidity/contracts/scl/mulmuladdX_fullgen_b4/contract.sol"
object "Contract_91" {
    code {
        {
            /// @src 0:2011:14164  "contract Contract {..."
            let _1 := memoryguard(0x80)
            mstore(64, _1)
            if callvalue() { revert(0, 0) }
            let _2 := datasize("Contract_91_deployed")
            codecopy(_1, dataoffset("Contract_91_deployed"), _2)
            return(_1, _2)
        }
    }
    /// @use-src 0:"CoqOfSolidity/contracts/scl/mulmuladdX_fullgen_b4/contract.sol"
    object "Contract_91_deployed" {
        code {
            {
                /// @src 0:2011:14164  "contract Contract {..."
                let _1 := memoryguard(0x0200)
                if iszero(lt(calldatasize(), 4))
                {
                    if eq(0x81a379ec, shr(224, calldataload(0)))
                    {
                        if callvalue() { revert(0, 0) }
                        if slt(add(calldatasize(), not(3)), 384) { revert(0, 0) }
                        if iszero(slt(35, calldatasize())) { revert(0, 0) }
                        let newFreePtr := add(_1, 320)
                        if or(gt(newFreePtr, 0xffffffffffffffff), lt(newFreePtr, _1))
                        {
                            mstore(0, shl(224, 0x4e487b71))
                            mstore(4, 0x41)
                            revert(0, 0x24)
                        }
                        mstore(64, newFreePtr)
                        let dst := _1
                        if gt(324, calldatasize()) { revert(0, 0) }
                        let src := 4
                        for { } lt(src, 324) { src := add(src, 0x20) }
                        {
                            mstore(dst, calldataload(src))
                            dst := add(dst, 0x20)
                        }
                        let ret := fun_ecGenMulmuladdX_store(_1, calldataload(324), calldataload(356))
                        let memPos := mload(64)
                        mstore(memPos, ret)
                        return(memPos, 0x20)
                    }
                }
                revert(0, 0)
            }
            /// @src 0:2864:14142  "assembly (\"memory-safe\") {..."
            function usr$ecAddn2_2189(usr$x1, usr$y1, usr$x2, usr$y2, usr_p) -> usr_x, usr_y, usr_zz, usr_zzz
            {
                let usr$y1_1 := sub(usr_p, usr$y1)
                let usr$y2_1 := addmod(mulmod(usr$y2, 1, usr_p), usr$y1_1, usr_p)
                let usr$x2_1 := addmod(mulmod(usr$x2, 1, usr_p), sub(usr_p, usr$x1), usr_p)
                let usr_x_1 := mulmod(usr$x2_1, usr$x2_1, usr_p)
                let usr_y_1 := mulmod(usr_x_1, usr$x2_1, usr_p)
                usr_zz := mulmod(1, usr_x_1, usr_p)
                usr_zzz := mulmod(1, usr_y_1, usr_p)
                let usr$zz1 := mulmod(usr$x1, usr_x_1, usr_p)
                usr_x := addmod(addmod(mulmod(usr$y2_1, usr$y2_1, usr_p), sub(usr_p, usr_y_1), usr_p), mulmod(add(usr_p, not(/** @src 0:2011:14164  "contract Contract {..." */ 1)), /** @src 0:2864:14142  "assembly (\"memory-safe\") {..." */ usr$zz1, usr_p), usr_p)
                usr_y := addmod(mulmod(addmod(usr$zz1, sub(usr_p, usr_x), usr_p), usr$y2_1, usr_p), mulmod(usr$y1_1, usr_y_1, usr_p), usr_p)
            }
            function usr$ecAddn2(usr$x1, usr$y1, usr$zz1, usr$zzz1, usr$x2, usr$y2, usr$_p) -> usr_x, usr_y, usr_zz, usr_zzz
            {
                let usr$y2_1 := addmod(mulmod(usr$y2, usr$zzz1, usr$_p), sub(usr$_p, usr$y1), usr$_p)
                let usr$x2_1 := addmod(mulmod(usr$x2, usr$zz1, usr$_p), sub(usr$_p, usr$x1), usr$_p)
                let usr_x_1 := mulmod(usr$x2_1, usr$x2_1, usr$_p)
                let usr_y_1 := mulmod(usr_x_1, usr$x2_1, usr$_p)
                usr_zz := mulmod(usr$zz1, usr_x_1, usr$_p)
                usr_zzz := mulmod(usr$zzz1, usr_y_1, usr$_p)
                let usr$zz1_1 := mulmod(usr$x1, usr_x_1, usr$_p)
                usr_x := addmod(addmod(mulmod(usr$y2_1, usr$y2_1, usr$_p), sub(usr$_p, usr_y_1), usr$_p), mulmod(add(usr$_p, not(/** @src 0:2011:14164  "contract Contract {..." */ 1)), /** @src 0:2864:14142  "assembly (\"memory-safe\") {..." */ usr$zz1_1, usr$_p), usr$_p)
                usr_y := addmod(mulmod(addmod(usr$zz1_1, sub(usr$_p, usr_x), usr$_p), usr$y2_1, usr$_p), mulmod(sub(usr$_p, usr$y1), usr_y_1, usr$_p), usr$_p)
            }
            /// @ast-id 90 @src 0:2383:14162  "function ecGenMulmuladdX_store(..."
            function fun_ecGenMulmuladdX_store(var_Q_mpos, var_scalar_u, var_scalar_v) -> var_X
            /// @ast-id 90
            {
                mstore(0x0140, var_Q_mpos)
                mstore(0x0120, var_scalar_u)
                mstore(0x0160, var_scalar_v)
                mstore(0xa0, 0)
                /// @ast-id 90
                /** @ast-id 90 */ /** @ast-id 90 */ fun_ecGenMulmuladdX_store_2814()
                /// @ast-id 90
                /** @ast-id 90 */ var_X := /** @ast-id 90 */ /** @ast-id 90 */ mload(/** @ast-id 90 */ 0xa0)
            }
            /// @ast-id 90
            function fun_ecGenMulmuladdX_store_2814()
            {
                /// @src 0:2586:2595  "uint256 X"
                mstore(0xa0, /** @src 0:2011:14164  "contract Contract {..." */ 0)
                /// @src 0:2607:2626  "uint256 mask=1<<127"
                mstore(0x01a0, /** @src 0:2620:2626  "1<<127" */ shl(127, /** @src 0:2011:14164  "contract Contract {..." */ 1))
                /// @src 0:2679:2703  "scalar_u==0&&scalar_v==0"
                let expr := /** @src 0:2679:2690  "scalar_u==0" */ iszero(mload(0x0120))
                /// @src 0:2679:2703  "scalar_u==0&&scalar_v==0"
                if expr
                {
                    expr := /** @src 0:2692:2703  "scalar_v==0" */ iszero(mload(0x0160))
                }
                /// @src 0:2676:2737  "if(scalar_u==0&&scalar_v==0){..."
                if expr
                {
                    /// @src 0:2718:2726  "return 0"
                    mstore(0xa0, /** @src 0:2011:14164  "contract Contract {..." */ 0)
                    /// @src 0:2718:2726  "return 0"
                    leave
                }
                /// @src 0:2746:2755  "uint256 Y"
                mstore(0x0100, /** @src 0:2011:14164  "contract Contract {..." */ 0)
                /// @src 0:2765:2776  "uint256 ZZZ"
                mstore(0xe0, /** @src 0:2011:14164  "contract Contract {..." */ 0)
                /// @src 0:2786:2796  "uint256 ZZ"
                mstore(0x01c0, /** @src 0:2011:14164  "contract Contract {..." */ 0)
                /// @src 0:2864:14142  "assembly (\"memory-safe\") {..."
                let _1 := mload(0x40)
                mstore(0x40, add(_1, 2048))
                mstore(add(_1, 4128), mload(add(mload(0x0140), 128)))
                let _2 := mload(0x40)
                let usr_modulusp := mload(add(_2, 2080))
                let _3 := add(mload(0x0140), 224)
                let _4 := mload(_3)
                let _5 := add(mload(0x0140), 192)
                mstore(add(128, _2), mload(_5))
                mstore(add(_2, 160), _4)
                mstore(add(_2, 192), 1)
                mstore(add(_2, 224), 1)
                let _6 := add(mload(0x0140), 288)
                let _7 := mload(_6)
                let _8 := add(mload(0x0140), 256)
                let _9 := mload(_8)
                let _10 := mload(0x40)
                mstore(add(256, _10), _9)
                mstore(add(_10, 288), _7)
                mstore(add(_10, 320), 1)
                mstore(add(_10, 352), 1)
                let var_X := mload(_8)
                let var_Y := mload(_6)
                let var_X_1, var_Y_1, var_ZZ, var_ZZZ := usr$ecAddn2_2189(var_X, var_Y, mload(_5), mload(_3), usr_modulusp)
                let _11 := mload(0x40)
                mstore(add(384, _11), var_X_1)
                mstore(add(_11, 416), var_Y_1)
                mstore(add(_11, 448), var_ZZ)
                mstore(add(_11, 480), var_ZZZ)
                let _12 := mload(add(32, mload(0x0140)))
                let _13 := mload(mload(0x0140))
                let _14 := mload(0x40)
                mstore(add(512, _14), _13)
                mstore(add(_14, 544), _12)
                mstore(add(_14, 576), 1)
                mstore(add(_14, 608), 1)
                let _15 := mload(_3)
                let _16 := mload(_5)
                let _17 := add(mload(0x0140), 32)
                let var_X_2, var_Y_2, var_ZZ_1, var_ZZZ_1 := usr$ecAddn2_2189(mload(mload(0x0140)), mload(_17), _16, _15, usr_modulusp)
                let _18 := mload(0x40)
                mstore(add(640, _18), var_X_2)
                mstore(add(_18, 672), var_Y_2)
                mstore(add(_18, 704), var_ZZ_1)
                mstore(add(_18, 736), var_ZZZ_1)
                let var_X_3 := mload(_8)
                let var_Y_3 := mload(_6)
                let var_X_4, var_Y_4, var_ZZ_2, var_ZZZ_2 := usr$ecAddn2_2189(var_X_3, var_Y_3, mload(mload(0x0140)), mload(_17), usr_modulusp)
                let _19 := mload(0x40)
                mstore(add(768, _19), var_X_4)
                mstore(add(_19, 800), var_Y_4)
                mstore(add(_19, 832), var_ZZ_2)
                mstore(add(_19, 864), var_ZZZ_2)
                let var_X_5, var_Y_5, var_ZZ_3, var_ZZZ_3 := usr$ecAddn2(var_X_4, var_Y_4, var_ZZ_2, var_ZZZ_2, mload(_5), mload(_3), usr_modulusp)
                let _20 := mload(0x40)
                mstore(add(896, _20), var_X_5)
                mstore(add(_20, 928), var_Y_5)
                mstore(add(_20, 960), var_ZZ_3)
                mstore(add(_20, 992), var_ZZZ_3)
                let _21 := add(mload(0x0140), 96)
                let _22 := mload(_21)
                let _23 := add(mload(0x0140), 0x40)
                let _24 := mload(_23)
                let _25 := mload(0x40)
                mstore(add(1024, _25), _24)
                mstore(add(_25, 1056), _22)
                mstore(add(_25, 1088), 1)
                mstore(add(_25, 1120), 1)
                let _26 := mload(0x40)
                let var_X_6, var_Y_6, var_ZZ_4, var_ZZZ_4 := usr$ecAddn2_2189(mload(_23), mload(_21), mload(_5), mload(_3), mload(add(_26, 2080)))
                mstore(add(1152, _26), var_X_6)
                mstore(add(_26, 1184), var_Y_6)
                mstore(add(_26, 1216), var_ZZ_4)
                mstore(add(_26, 1248), var_ZZZ_4)
                let var_X_7 := mload(_8)
                let var_Y_7 := mload(_6)
                let _27 := mload(0x40)
                let var_X_8, var_Y_8, var_ZZ_5, var_ZZZ_5 := usr$ecAddn2_2189(mload(_23), mload(_21), var_X_7, var_Y_7, mload(add(_27, 2080)))
                mstore(add(1280, _27), var_X_8)
                mstore(add(_27, 1312), var_Y_8)
                mstore(add(_27, 1344), var_ZZ_5)
                mstore(add(_27, 1376), var_ZZZ_5)
                let _28 := mload(0x40)
                let var_X_9, var_Y_9, var_ZZ_6, var_ZZZ_6 := usr$ecAddn2(var_X_8, var_Y_8, var_ZZ_5, var_ZZZ_5, mload(_5), mload(_3), mload(add(_28, 2080)))
                mstore(add(1408, _28), var_X_9)
                mstore(add(_28, 1440), var_Y_9)
                mstore(add(_28, 1472), var_ZZ_6)
                mstore(add(_28, 1504), var_ZZZ_6)
                let _29 := mload(0x40)
                let var_X_10, var_Y_10, var_ZZ_7, var_ZZZ_7 := usr$ecAddn2_2189(mload(mload(0x0140)), mload(_17), mload(_23), mload(_21), mload(add(_29, 2080)))
                mstore(add(1536, _29), var_X_10)
                mstore(add(_29, 1568), var_Y_10)
                mstore(add(_29, 1600), var_ZZ_7)
                mstore(add(_29, 1632), var_ZZZ_7)
                let _30 := mload(0x40)
                let var_X_11, var_Y_11, var_ZZ_8, var_ZZZ_8 := usr$ecAddn2(var_X_10, var_Y_10, var_ZZ_7, var_ZZZ_7, mload(_5), mload(_3), mload(add(_30, 2080)))
                mstore(add(1664, _30), var_X_11)
                mstore(add(_30, 1696), var_Y_11)
                mstore(add(_30, 1728), var_ZZ_8)
                mstore(add(_30, 1760), var_ZZZ_8)
                let _31 := mload(0x40)
                let var_X_12 := mload(add(768, _31))
                let var_Y_12 := mload(add(800, _31))
                let var_ZZ_9 := mload(add(832, _31))
                let var_ZZZ_9 := mload(add(864, _31))
                let var_X_13, var_Y_13, var_ZZ_10, var_ZZZ_10 := usr$ecAddn2(var_X_12, var_Y_12, var_ZZ_9, var_ZZZ_9, mload(_23), mload(_21), mload(add(_31, 2080)))
                mstore(add(1792, _31), var_X_13)
                mstore(add(_31, 1824), var_Y_13)
                mstore(add(_31, 1856), var_ZZ_10)
                mstore(add(_31, 1888), var_ZZZ_10)
                let _32 := mload(0x40)
                let var_X_14, var_Y_14, var_ZZ_11, var_ZZZ_11 := usr$ecAddn2(var_X_13, var_Y_13, var_ZZ_10, var_ZZZ_10, mload(_5), mload(_3), mload(add(_32, 2080)))
                mstore(add(1920, _32), var_X_14)
                mstore(add(_32, 1952), var_Y_14)
                mstore(add(_32, 1984), var_ZZ_11)
                mstore(add(_32, 2016), var_ZZZ_11)
                mstore(0xe0, /** @src 0:2011:14164  "contract Contract {..." */ 0)
                /// @src 0:2864:14142  "assembly (\"memory-safe\") {..."
                for { }
                iszero(mload(0xe0))
                {
                    mstore(0x01a0, shr(1, mload(0x01a0)))
                }
                {
                    mstore(0xe0, add(add(sub(1, iszero(and(mload(0x0120), mload(0x01a0)))), shl(1, sub(1, iszero(and(shr(128, mload(0x0120)), mload(0x01a0)))))), add(shl(2, sub(1, iszero(and(mload(0x0160), mload(0x01a0))))), shl(3, sub(1, iszero(and(shr(128, mload(0x0160)), mload(0x01a0))))))))
                }
                mstore(0x0180, mload(0x40))
                let _33 := add(mload(0x0180), shl(7, mload(0xe0)))
                mstore(0xa0, mload(_33))
                mstore(0x0100, mload(add(_33, 32)))
                mstore(0x01c0, mload(add(_33, 0x40)))
                mstore(0xe0, mload(add(_33, 96)))
                mstore(0x01e0, mload(add(mload(0x0180), 2080)))
                for { }
                mload(0x01a0)
                {
                    mstore(0x01a0, shr(1, mload(0x01a0)))
                }
                {
                    mstore(0xc0, mulmod(mload(0xa0), mulmod(mulmod(2, mload(0x0100), mload(0x01e0)), mulmod(2, mload(0x0100), mload(0x01e0)), mload(0x01e0)), mload(0x01e0)))
                    let usr$T4 := addmod(mulmod(3, mulmod(mload(0xa0), mload(0xa0), mload(0x01e0)), mload(0x01e0)), mulmod(mload(add(mload(0x0140), 160)), mulmod(mload(0x01c0), mload(0x01c0), mload(0x01e0)), mload(0x01e0)), mload(0x01e0))
                    mstore(0xe0, mulmod(mulmod(mulmod(2, mload(0x0100), mload(0x01e0)), mulmod(mulmod(2, mload(0x0100), mload(0x01e0)), mulmod(2, mload(0x0100), mload(0x01e0)), mload(0x01e0)), mload(0x01e0)), mload(0xe0), mload(0x01e0)))
                    mstore(0x01c0, mulmod(mulmod(mulmod(2, mload(0x0100), mload(0x01e0)), mulmod(2, mload(0x0100), mload(0x01e0)), mload(0x01e0)), mload(0x01c0), mload(0x01e0)))
                    mstore(0xa0, addmod(mulmod(usr$T4, usr$T4, mload(0x01e0)), mulmod(add(mload(0x01e0), not(/** @src 0:2011:14164  "contract Contract {..." */ 1)), /** @src 0:2864:14142  "assembly (\"memory-safe\") {..." */ mload(0xc0), mload(0x01e0)), mload(0x01e0)))
                    mstore(0x0100, addmod(mulmod(mulmod(mulmod(2, mload(0x0100), mload(0x01e0)), mulmod(mulmod(2, mload(0x0100), mload(0x01e0)), mulmod(2, mload(0x0100), mload(0x01e0)), mload(0x01e0)), mload(0x01e0)), mload(0x0100), mload(0x01e0)), mulmod(usr$T4, addmod(mload(0xa0), sub(mload(0x01e0), mload(0xc0)), mload(0x01e0)), mload(0x01e0)), mload(0x01e0)))
                    let usr$T1 := add(add(sub(1, iszero(and(mload(0x0120), mload(0x01a0)))), shl(1, sub(1, iszero(and(shr(128, mload(0x0120)), mload(0x01a0)))))), add(shl(2, sub(1, iszero(and(mload(0x0160), mload(0x01a0))))), shl(3, sub(1, iszero(and(shr(128, mload(0x0160)), mload(0x01a0)))))))
                    if iszero(usr$T1)
                    {
                        mstore(0x0100, sub(mload(0x01e0), mload(0x0100)))
                        continue
                    }
                    let usr$T4_1 := mload(add(mload(0x0180), shl(7, usr$T1)))
                    let _34 := add(add(mload(0x0180), shl(7, usr$T1)), 96)
                    let _35 := mload(_34)
                    mstore(add(mload(0x0180), 2144), _35)
                    if iszero(mload(0x01c0))
                    {
                        mstore(0xa0, usr$T4_1)
                        mstore(0x0100, mload(add(add(mload(0x0180), shl(7, usr$T1)), 32)))
                        mstore(0x01c0, mload(add(add(mload(0x0180), shl(7, usr$T1)), 0x40)))
                        mstore(0xe0, mload(_34))
                        continue
                    }
                    let _36 := addmod(mulmod(mload(add(add(mload(0x0180), shl(7, usr$T1)), 32)), mload(0xe0), mload(0x01e0)), mulmod(mload(0x0100), _35, mload(0x01e0)), mload(0x01e0))
                    mstore(add(mload(0x0180), 2112), _36)
                    let usr$T1_1 := mload(add(add(mload(0x0180), shl(7, usr$T1)), 0x40))
                    let usr$T2 := addmod(mulmod(usr$T4_1, mload(0x01c0), mload(0x01e0)), sub(mload(0x01e0), mulmod(mload(0xa0), usr$T1_1, mload(0x01e0))), mload(0x01e0))
                    if iszero(_36)
                    {
                        if iszero(usr$T2)
                        {
                            mstore(0x80, mulmod(mload(0xa0), mulmod(mulmod(add(mload(0x01e0), not(/** @src 0:2011:14164  "contract Contract {..." */ 1)), /** @src 0:2864:14142  "assembly (\"memory-safe\") {..." */ mload(0x0100), mload(0x01e0)), mulmod(add(mload(0x01e0), not(/** @src 0:2011:14164  "contract Contract {..." */ 1)), /** @src 0:2864:14142  "assembly (\"memory-safe\") {..." */ mload(0x0100), mload(0x01e0)), mload(0x01e0)), mload(0x01e0)))
                            mstore(add(mload(0x0180), 2112), mload(0x80))
                            usr$T4_1 := addmod(mulmod(3, mulmod(mload(0xa0), mload(0xa0), mload(0x01e0)), mload(0x01e0)), mulmod(mload(add(mload(0x0140), 160)), mulmod(mload(0x01c0), mload(0x01c0), mload(0x01e0)), mload(0x01e0)), mload(0x01e0))
                            mstore(0xe0, mulmod(mulmod(mulmod(add(mload(0x01e0), not(/** @src 0:2011:14164  "contract Contract {..." */ 1)), /** @src 0:2864:14142  "assembly (\"memory-safe\") {..." */ mload(0x0100), mload(0x01e0)), mulmod(mulmod(add(mload(0x01e0), not(/** @src 0:2011:14164  "contract Contract {..." */ 1)), /** @src 0:2864:14142  "assembly (\"memory-safe\") {..." */ mload(0x0100), mload(0x01e0)), mulmod(add(mload(0x01e0), not(/** @src 0:2011:14164  "contract Contract {..." */ 1)), /** @src 0:2864:14142  "assembly (\"memory-safe\") {..." */ mload(0x0100), mload(0x01e0)), mload(0x01e0)), mload(0x01e0)), mload(0xe0), mload(0x01e0)))
                            mstore(0x01c0, mulmod(mulmod(mulmod(add(mload(0x01e0), not(/** @src 0:2011:14164  "contract Contract {..." */ 1)), /** @src 0:2864:14142  "assembly (\"memory-safe\") {..." */ mload(0x0100), mload(0x01e0)), mulmod(add(mload(0x01e0), not(/** @src 0:2011:14164  "contract Contract {..." */ 1)), /** @src 0:2864:14142  "assembly (\"memory-safe\") {..." */ mload(0x0100), mload(0x01e0)), mload(0x01e0)), mload(0x01c0), mload(0x01e0)))
                            mstore(0xa0, addmod(mulmod(usr$T4_1, usr$T4_1, mload(0x01e0)), mulmod(add(mload(0x01e0), not(/** @src 0:2011:14164  "contract Contract {..." */ 1)), /** @src 0:2864:14142  "assembly (\"memory-safe\") {..." */ mload(0x80), mload(0x01e0)), mload(0x01e0)))
                            usr$T2 := mulmod(usr$T4_1, addmod(mload(0x80), sub(mload(0x01e0), mload(0xa0)), mload(0x01e0)), mload(0x01e0))
                            mstore(0x0100, addmod(usr$T2, mulmod(mulmod(mulmod(add(mload(0x01e0), not(/** @src 0:2011:14164  "contract Contract {..." */ 1)), /** @src 0:2864:14142  "assembly (\"memory-safe\") {..." */ mload(0x0100), mload(0x01e0)), mulmod(mulmod(add(mload(0x01e0), not(/** @src 0:2011:14164  "contract Contract {..." */ 1)), /** @src 0:2864:14142  "assembly (\"memory-safe\") {..." */ mload(0x0100), mload(0x01e0)), mulmod(add(mload(0x01e0), not(/** @src 0:2011:14164  "contract Contract {..." */ 1)), /** @src 0:2864:14142  "assembly (\"memory-safe\") {..." */ mload(0x0100), mload(0x01e0)), mload(0x01e0)), mload(0x01e0)), mload(0x0100), mload(0x01e0)), mload(0x01e0)))
                            continue
                        }
                    }
                    mstore(0x01c0, mulmod(mulmod(mload(0x01c0), mulmod(usr$T2, usr$T2, mload(0x01e0)), mload(0x01e0)), usr$T1_1, mload(0x01e0)))
                    let usr$T1_2 := mulmod(mload(0xa0), usr$T1_1, mload(0x01e0))
                    mstore(0xe0, mulmod(mulmod(mload(0xe0), mulmod(mulmod(usr$T2, usr$T2, mload(0x01e0)), usr$T2, mload(0x01e0)), mload(0x01e0)), _35, mload(0x01e0)))
                    let _37 := mload(add(mload(0x0180), 2112))
                    mstore(0xa0, addmod(addmod(mulmod(_37, _37, mload(0x01e0)), sub(mload(0x01e0), mulmod(mulmod(usr$T2, usr$T2, mload(0x01e0)), usr$T2, mload(0x01e0))), mload(0x01e0)), mulmod(usr$T1_2, mulmod(add(mload(0x01e0), not(/** @src 0:2011:14164  "contract Contract {..." */ 1)), /** @src 0:2864:14142  "assembly (\"memory-safe\") {..." */ mulmod(usr$T2, usr$T2, mload(0x01e0)), mload(0x01e0)), mload(0x01e0)), mload(0x01e0)))
                    mstore(0x0100, addmod(mulmod(addmod(mulmod(usr$T1_2, mulmod(usr$T2, usr$T2, mload(0x01e0)), mload(0x01e0)), sub(mload(0x01e0), mload(0xa0)), mload(0x01e0)), _37, mload(0x01e0)), mulmod(mulmod(mload(0x0100), _35, mload(0x01e0)), mulmod(mulmod(usr$T2, usr$T2, mload(0x01e0)), usr$T2, mload(0x01e0)), mload(0x01e0)), mload(0x01e0)))
                }
                mstore(0x40, 2176)
                mstore(2272, mload(0x01c0))
                mstore(2176, 32)
                mstore(2208, 32)
                mstore(2240, 32)
                mstore(2304, add(mload(0x01e0), not(/** @src 0:2011:14164  "contract Contract {..." */ 1)))
                /// @src 0:2864:14142  "assembly (\"memory-safe\") {..."
                mstore(2336, mload(0x01e0))
                if iszero(staticcall(not(0), 0x05, 2176, 192, 2176, 32))
                {
                    mstore(0x40, 2047)
                    revert(0x40, 32)
                }
                mstore(0xa0, mulmod(mload(0xa0), mload(2176), mload(0x01e0)))
            }
        }
        data ".metadata" hex"a26469706673582212206730a111b8db8822867b5c4ebd5f13275a06ffcd338937b7f32169a8773f4d6d64736f6c634300081b0033"
    }
}

