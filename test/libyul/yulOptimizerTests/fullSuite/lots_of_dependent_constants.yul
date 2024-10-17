/*
pragma solidity ^0.8.26;

contract Test {
    uint256 public constant A0 = 8;
    uint256 public constant A1 = A0 * 2;
    uint256 public constant A2 = A1 * 2;
    uint256 public constant A3 = A2 * 2;
    uint256 public constant A4 = A3 * 2;
    uint256 public constant A5 = A4 * 2;
    uint256 public constant A6 = A5 * 2;
    uint256 public constant A7 = A6 * 2;
    uint256 public constant A8 = A7 * 2;
    uint256 public constant A9 = A8 * 2;
    uint256 public constant A10 = A9 * 2;

    function test(uint256 x) public pure returns (uint256) {
        return x / A10;
    }
}
*/

{
    /// @src 0:26:582  "contract Test {..."
    mstore(64, memoryguard(128))

    if iszero(lt(calldatasize(), 4))
    {
        let selector := shift_right_224_unsigned(calldataload(0))
        switch selector

        case 0x29e99f07
        {
            // test(uint256)

            external_fun_test_66()
        }

        case 0x3c823131
        {
            // A6()

            external_fun_A6_34()
        }

        case 0x4a23d454
        {
            // A7()

            external_fun_A7_39()
        }

        case 0x5902c17a
        {
            // A2()

            external_fun_A2_14()
        }

        case 0x5cc23f7d
        {
            // A0()

            external_fun_A0_4()
        }

        case 0x7d6d1f39
        {
            // A4()

            external_fun_A4_24()
        }

        case 0x97a28744
        {
            // A10()

            external_fun_A10_54()
        }

        case 0x9f08ce4f
        {
            // A5()

            external_fun_A5_29()
        }

        case 0xa33eadce
        {
            // A3()

            external_fun_A3_19()
        }

        case 0xca035e93
        {
            // A9()

            external_fun_A9_49()
        }

        case 0xd173ee61
        {
            // A8()

            external_fun_A8_44()
        }

        case 0xf8341309
        {
            // A1()

            external_fun_A1_9()
        }

        default {}
    }

    revert_error_42b3090547df1d2001c96683413b8cf91c1b902ef5e3cb8d9f6f304cf7446f74()

    function shift_right_224_unsigned(value) -> newValue {
        newValue :=

        shr(224, value)

    }

    function allocate_unbounded() -> memPtr {
        memPtr := mload(64)
    }

    function revert_error_ca66f745a3ce8ff40e2ccaf1ad45db7774001b90d25810abd9040049be7bf4bb() {
        revert(0, 0)
    }

    function revert_error_dbdddcbe895c83990c08b3492a0e83918d802a52331272ac6fdb6a7c4aea3b1b() {
        revert(0, 0)
    }

    function revert_error_c1322bf8034eace5e0b5c7295db60986aa89aae5e0ea0873e4689e076861a5db() {
        revert(0, 0)
    }

    function cleanup_t_uint256(value) -> cleaned {
        cleaned := value
    }

    function validator_revert_t_uint256(value) {
        if iszero(eq(value, cleanup_t_uint256(value))) { revert(0, 0) }
    }

    function abi_decode_t_uint256(offset, end) -> value {
        value := calldataload(offset)
        validator_revert_t_uint256(value)
    }

    function abi_decode_tuple_t_uint256(headStart, dataEnd) -> value0 {
        if slt(sub(dataEnd, headStart), 32) { revert_error_dbdddcbe895c83990c08b3492a0e83918d802a52331272ac6fdb6a7c4aea3b1b() }

        {

            let offset := 0

            value0 := abi_decode_t_uint256(add(headStart, offset), dataEnd)
        }

    }

    function abi_encode_t_uint256_to_t_uint256_fromStack(value, pos) {
        mstore(pos, cleanup_t_uint256(value))
    }

    function abi_encode_tuple_t_uint256__to_t_uint256__fromStack(headStart , value0) -> tail {
        tail := add(headStart, 32)

        abi_encode_t_uint256_to_t_uint256_fromStack(value0,  add(headStart, 0))

    }

    function external_fun_test_66() {

        if callvalue() { revert_error_ca66f745a3ce8ff40e2ccaf1ad45db7774001b90d25810abd9040049be7bf4bb() }
        let param_0 :=  abi_decode_tuple_t_uint256(4, calldatasize())
        let ret_0 :=  fun_test_66(param_0)
        let memPos := allocate_unbounded()
        let memEnd := abi_encode_tuple_t_uint256__to_t_uint256__fromStack(memPos , ret_0)
        return(memPos, sub(memEnd, memPos))

    }

    function abi_decode_tuple_(headStart, dataEnd)   {
        if slt(sub(dataEnd, headStart), 0) { revert_error_dbdddcbe895c83990c08b3492a0e83918d802a52331272ac6fdb6a7c4aea3b1b() }

    }

    function cleanup_t_rational_8_by_1(value) -> cleaned {
        cleaned := value
    }

    function identity(value) -> ret {
        ret := value
    }

    function convert_t_rational_8_by_1_to_t_uint256(value) -> converted {
        converted := cleanup_t_uint256(identity(cleanup_t_rational_8_by_1(value)))
    }

    /// @src 0:46:76  "uint256 public constant A0 = 8"
    function constant_A0_4() -> ret {
        /// @src 0:75:76  "8"
        let expr_3 := 0x08
        let _1 := convert_t_rational_8_by_1_to_t_uint256(expr_3)

        ret := _1
    }

    function cleanup_t_rational_2_by_1(value) -> cleaned {
        cleaned := value
    }

    function convert_t_rational_2_by_1_to_t_uint256(value) -> converted {
        converted := cleanup_t_uint256(identity(cleanup_t_rational_2_by_1(value)))
    }

    function panic_error_0x11() {
        mstore(0, 35408467139433450592217433187231851964531694900788300625387963629091585785856)
        mstore(4, 0x11)
        revert(0, 0x24)
    }

    function checked_mul_t_uint256(x, y) -> product {
        x := cleanup_t_uint256(x)
        y := cleanup_t_uint256(y)
        let product_raw := mul(x, y)
        product := cleanup_t_uint256(product_raw)

        // overflow, if x != 0 and y != product/x
        if iszero(
            or(
                iszero(x),
                eq(y, div(product, x))
            )
        ) { panic_error_0x11() }

    }

    /// @src 0:82:117  "uint256 public constant A1 = A0 * 2"
    function constant_A1_9() -> ret {
        /// @src 0:111:113  "A0"
        let expr_6 := constant_A0_4()
        /// @src 0:116:117  "2"
        let expr_7 := 0x02
        /// @src 0:111:117  "A0 * 2"
        let expr_8 := checked_mul_t_uint256(expr_6, convert_t_rational_2_by_1_to_t_uint256(expr_7))

        let _2 := expr_8

        ret := _2
    }

    /// @src 0:123:158  "uint256 public constant A2 = A1 * 2"
    function constant_A2_14() -> ret {
        /// @src 0:152:154  "A1"
        let expr_11 := constant_A1_9()
        /// @src 0:157:158  "2"
        let expr_12 := 0x02
        /// @src 0:152:158  "A1 * 2"
        let expr_13 := checked_mul_t_uint256(expr_11, convert_t_rational_2_by_1_to_t_uint256(expr_12))

        let _3 := expr_13

        ret := _3
    }

    /// @src 0:164:199  "uint256 public constant A3 = A2 * 2"
    function constant_A3_19() -> ret {
        /// @src 0:193:195  "A2"
        let expr_16 := constant_A2_14()
        /// @src 0:198:199  "2"
        let expr_17 := 0x02
        /// @src 0:193:199  "A2 * 2"
        let expr_18 := checked_mul_t_uint256(expr_16, convert_t_rational_2_by_1_to_t_uint256(expr_17))

        let _4 := expr_18

        ret := _4
    }

    /// @src 0:205:240  "uint256 public constant A4 = A3 * 2"
    function constant_A4_24() -> ret {
        /// @src 0:234:236  "A3"
        let expr_21 := constant_A3_19()
        /// @src 0:239:240  "2"
        let expr_22 := 0x02
        /// @src 0:234:240  "A3 * 2"
        let expr_23 := checked_mul_t_uint256(expr_21, convert_t_rational_2_by_1_to_t_uint256(expr_22))

        let _5 := expr_23

        ret := _5
    }

    /// @src 0:246:281  "uint256 public constant A5 = A4 * 2"
    function constant_A5_29() -> ret {
        /// @src 0:275:277  "A4"
        let expr_26 := constant_A4_24()
        /// @src 0:280:281  "2"
        let expr_27 := 0x02
        /// @src 0:275:281  "A4 * 2"
        let expr_28 := checked_mul_t_uint256(expr_26, convert_t_rational_2_by_1_to_t_uint256(expr_27))

        let _6 := expr_28

        ret := _6
    }

    /// @src 0:287:322  "uint256 public constant A6 = A5 * 2"
    function constant_A6_34() -> ret {
        /// @src 0:316:318  "A5"
        let expr_31 := constant_A5_29()
        /// @src 0:321:322  "2"
        let expr_32 := 0x02
        /// @src 0:316:322  "A5 * 2"
        let expr_33 := checked_mul_t_uint256(expr_31, convert_t_rational_2_by_1_to_t_uint256(expr_32))

        let _7 := expr_33

        ret := _7
    }

    /// @ast-id 34
    /// @src 0:287:322  "uint256 public constant A6 = A5 * 2"
    function getter_fun_A6_34() -> ret_0 {
        ret_0 := constant_A6_34()
    }
    /// @src 0:26:582  "contract Test {..."

    function external_fun_A6_34() {

        if callvalue() { revert_error_ca66f745a3ce8ff40e2ccaf1ad45db7774001b90d25810abd9040049be7bf4bb() }
        abi_decode_tuple_(4, calldatasize())
        let ret_0 :=  getter_fun_A6_34()
        let memPos := allocate_unbounded()
        let memEnd := abi_encode_tuple_t_uint256__to_t_uint256__fromStack(memPos , ret_0)
        return(memPos, sub(memEnd, memPos))

    }

    /// @src 0:328:363  "uint256 public constant A7 = A6 * 2"
    function constant_A7_39() -> ret {
        /// @src 0:357:359  "A6"
        let expr_36 := constant_A6_34()
        /// @src 0:362:363  "2"
        let expr_37 := 0x02
        /// @src 0:357:363  "A6 * 2"
        let expr_38 := checked_mul_t_uint256(expr_36, convert_t_rational_2_by_1_to_t_uint256(expr_37))

        let _8 := expr_38

        ret := _8
    }

    /// @ast-id 39
    /// @src 0:328:363  "uint256 public constant A7 = A6 * 2"
    function getter_fun_A7_39() -> ret_0 {
        ret_0 := constant_A7_39()
    }
    /// @src 0:26:582  "contract Test {..."

    function external_fun_A7_39() {

        if callvalue() { revert_error_ca66f745a3ce8ff40e2ccaf1ad45db7774001b90d25810abd9040049be7bf4bb() }
        abi_decode_tuple_(4, calldatasize())
        let ret_0 :=  getter_fun_A7_39()
        let memPos := allocate_unbounded()
        let memEnd := abi_encode_tuple_t_uint256__to_t_uint256__fromStack(memPos , ret_0)
        return(memPos, sub(memEnd, memPos))

    }

    /// @ast-id 14
    /// @src 0:123:158  "uint256 public constant A2 = A1 * 2"
    function getter_fun_A2_14() -> ret_0 {
        ret_0 := constant_A2_14()
    }
    /// @src 0:26:582  "contract Test {..."

    function external_fun_A2_14() {

        if callvalue() { revert_error_ca66f745a3ce8ff40e2ccaf1ad45db7774001b90d25810abd9040049be7bf4bb() }
        abi_decode_tuple_(4, calldatasize())
        let ret_0 :=  getter_fun_A2_14()
        let memPos := allocate_unbounded()
        let memEnd := abi_encode_tuple_t_uint256__to_t_uint256__fromStack(memPos , ret_0)
        return(memPos, sub(memEnd, memPos))

    }

    /// @ast-id 4
    /// @src 0:46:76  "uint256 public constant A0 = 8"
    function getter_fun_A0_4() -> ret_0 {
        ret_0 := constant_A0_4()
    }
    /// @src 0:26:582  "contract Test {..."

    function external_fun_A0_4() {

        if callvalue() { revert_error_ca66f745a3ce8ff40e2ccaf1ad45db7774001b90d25810abd9040049be7bf4bb() }
        abi_decode_tuple_(4, calldatasize())
        let ret_0 :=  getter_fun_A0_4()
        let memPos := allocate_unbounded()
        let memEnd := abi_encode_tuple_t_uint256__to_t_uint256__fromStack(memPos , ret_0)
        return(memPos, sub(memEnd, memPos))

    }

    /// @ast-id 24
    /// @src 0:205:240  "uint256 public constant A4 = A3 * 2"
    function getter_fun_A4_24() -> ret_0 {
        ret_0 := constant_A4_24()
    }
    /// @src 0:26:582  "contract Test {..."

    function external_fun_A4_24() {

        if callvalue() { revert_error_ca66f745a3ce8ff40e2ccaf1ad45db7774001b90d25810abd9040049be7bf4bb() }
        abi_decode_tuple_(4, calldatasize())
        let ret_0 :=  getter_fun_A4_24()
        let memPos := allocate_unbounded()
        let memEnd := abi_encode_tuple_t_uint256__to_t_uint256__fromStack(memPos , ret_0)
        return(memPos, sub(memEnd, memPos))

    }

    /// @src 0:369:404  "uint256 public constant A8 = A7 * 2"
    function constant_A8_44() -> ret {
        /// @src 0:398:400  "A7"
        let expr_41 := constant_A7_39()
        /// @src 0:403:404  "2"
        let expr_42 := 0x02
        /// @src 0:398:404  "A7 * 2"
        let expr_43 := checked_mul_t_uint256(expr_41, convert_t_rational_2_by_1_to_t_uint256(expr_42))

        let _9 := expr_43

        ret := _9
    }

    /// @src 0:410:445  "uint256 public constant A9 = A8 * 2"
    function constant_A9_49() -> ret {
        /// @src 0:439:441  "A8"
        let expr_46 := constant_A8_44()
        /// @src 0:444:445  "2"
        let expr_47 := 0x02
        /// @src 0:439:445  "A8 * 2"
        let expr_48 := checked_mul_t_uint256(expr_46, convert_t_rational_2_by_1_to_t_uint256(expr_47))

        let _10 := expr_48

        ret := _10
    }

    /// @src 0:451:487  "uint256 public constant A10 = A9 * 2"
    function constant_A10_54() -> ret {
        /// @src 0:481:483  "A9"
        let expr_51 := constant_A9_49()
        /// @src 0:486:487  "2"
        let expr_52 := 0x02
        /// @src 0:481:487  "A9 * 2"
        let expr_53 := checked_mul_t_uint256(expr_51, convert_t_rational_2_by_1_to_t_uint256(expr_52))

        let _11 := expr_53

        ret := _11
    }

    /// @ast-id 54
    /// @src 0:451:487  "uint256 public constant A10 = A9 * 2"
    function getter_fun_A10_54() -> ret_0 {
        ret_0 := constant_A10_54()
    }
    /// @src 0:26:582  "contract Test {..."

    function external_fun_A10_54() {

        if callvalue() { revert_error_ca66f745a3ce8ff40e2ccaf1ad45db7774001b90d25810abd9040049be7bf4bb() }
        abi_decode_tuple_(4, calldatasize())
        let ret_0 :=  getter_fun_A10_54()
        let memPos := allocate_unbounded()
        let memEnd := abi_encode_tuple_t_uint256__to_t_uint256__fromStack(memPos , ret_0)
        return(memPos, sub(memEnd, memPos))

    }

    /// @ast-id 29
    /// @src 0:246:281  "uint256 public constant A5 = A4 * 2"
    function getter_fun_A5_29() -> ret_0 {
        ret_0 := constant_A5_29()
    }
    /// @src 0:26:582  "contract Test {..."

    function external_fun_A5_29() {

        if callvalue() { revert_error_ca66f745a3ce8ff40e2ccaf1ad45db7774001b90d25810abd9040049be7bf4bb() }
        abi_decode_tuple_(4, calldatasize())
        let ret_0 :=  getter_fun_A5_29()
        let memPos := allocate_unbounded()
        let memEnd := abi_encode_tuple_t_uint256__to_t_uint256__fromStack(memPos , ret_0)
        return(memPos, sub(memEnd, memPos))

    }

    /// @ast-id 19
    /// @src 0:164:199  "uint256 public constant A3 = A2 * 2"
    function getter_fun_A3_19() -> ret_0 {
        ret_0 := constant_A3_19()
    }
    /// @src 0:26:582  "contract Test {..."

    function external_fun_A3_19() {

        if callvalue() { revert_error_ca66f745a3ce8ff40e2ccaf1ad45db7774001b90d25810abd9040049be7bf4bb() }
        abi_decode_tuple_(4, calldatasize())
        let ret_0 :=  getter_fun_A3_19()
        let memPos := allocate_unbounded()
        let memEnd := abi_encode_tuple_t_uint256__to_t_uint256__fromStack(memPos , ret_0)
        return(memPos, sub(memEnd, memPos))

    }

    /// @ast-id 49
    /// @src 0:410:445  "uint256 public constant A9 = A8 * 2"
    function getter_fun_A9_49() -> ret_0 {
        ret_0 := constant_A9_49()
    }
    /// @src 0:26:582  "contract Test {..."

    function external_fun_A9_49() {

        if callvalue() { revert_error_ca66f745a3ce8ff40e2ccaf1ad45db7774001b90d25810abd9040049be7bf4bb() }
        abi_decode_tuple_(4, calldatasize())
        let ret_0 :=  getter_fun_A9_49()
        let memPos := allocate_unbounded()
        let memEnd := abi_encode_tuple_t_uint256__to_t_uint256__fromStack(memPos , ret_0)
        return(memPos, sub(memEnd, memPos))

    }

    /// @ast-id 44
    /// @src 0:369:404  "uint256 public constant A8 = A7 * 2"
    function getter_fun_A8_44() -> ret_0 {
        ret_0 := constant_A8_44()
    }
    /// @src 0:26:582  "contract Test {..."

    function external_fun_A8_44() {

        if callvalue() { revert_error_ca66f745a3ce8ff40e2ccaf1ad45db7774001b90d25810abd9040049be7bf4bb() }
        abi_decode_tuple_(4, calldatasize())
        let ret_0 :=  getter_fun_A8_44()
        let memPos := allocate_unbounded()
        let memEnd := abi_encode_tuple_t_uint256__to_t_uint256__fromStack(memPos , ret_0)
        return(memPos, sub(memEnd, memPos))

    }

    /// @ast-id 9
    /// @src 0:82:117  "uint256 public constant A1 = A0 * 2"
    function getter_fun_A1_9() -> ret_0 {
        ret_0 := constant_A1_9()
    }
    /// @src 0:26:582  "contract Test {..."

    function external_fun_A1_9() {

        if callvalue() { revert_error_ca66f745a3ce8ff40e2ccaf1ad45db7774001b90d25810abd9040049be7bf4bb() }
        abi_decode_tuple_(4, calldatasize())
        let ret_0 :=  getter_fun_A1_9()
        let memPos := allocate_unbounded()
        let memEnd := abi_encode_tuple_t_uint256__to_t_uint256__fromStack(memPos , ret_0)
        return(memPos, sub(memEnd, memPos))

    }

    function revert_error_42b3090547df1d2001c96683413b8cf91c1b902ef5e3cb8d9f6f304cf7446f74() {
        revert(0, 0)
    }

    function zero_value_for_split_t_uint256() -> ret {
        ret := 0
    }

    function panic_error_0x12() {
        mstore(0, 35408467139433450592217433187231851964531694900788300625387963629091585785856)
        mstore(4, 0x12)
        revert(0, 0x24)
    }

    function checked_div_t_uint256(x, y) -> r {
        x := cleanup_t_uint256(x)
        y := cleanup_t_uint256(y)
        if iszero(y) { panic_error_0x12() }

        r := div(x, y)
    }

    /// @ast-id 66
    /// @src 0:494:580  "function test(uint256 x) public pure returns (uint256) {..."
    function fun_test_66(var_x_56) -> var__59 {
        /// @src 0:540:547  "uint256"
        let zero_t_uint256_12 := zero_value_for_split_t_uint256()
        var__59 := zero_t_uint256_12

        /// @src 0:566:567  "x"
        let _13 := var_x_56
        let expr_61 := _13
        /// @src 0:570:573  "A10"
        let expr_62 := constant_A10_54()
        /// @src 0:566:573  "x / A10"
        let expr_63 := checked_div_t_uint256(expr_61, expr_62)

        /// @src 0:559:573  "return x / A10"
        var__59 := expr_63
        leave

    }
    /// @src 0:26:582  "contract Test {..."

}

// ----
// step: fullSuite
//
// {
//     {
//         let _1 := memoryguard(0x80)
//         mstore(64, _1)
//         if iszero(lt(calldatasize(), 4))
//         {
//             switch shr(224, calldataload(0))
//             case 0x29e99f07 {
//                 if callvalue() { revert(0, 0) }
//                 if slt(add(calldatasize(), not(3)), 32) { revert(0, 0) }
//                 mstore(_1, shr(13, calldataload(4)))
//                 return(_1, 32)
//             }
//             case 0x3c823131 {
//                 if callvalue() { revert(0, 0) }
//                 if slt(add(calldatasize(), not(3)), 0) { revert(0, 0) }
//                 let memPos := mload(64)
//                 mstore(memPos, 512)
//                 return(memPos, 32)
//             }
//             case 0x4a23d454 {
//                 if callvalue() { revert(0, 0) }
//                 if slt(add(calldatasize(), not(3)), 0) { revert(0, 0) }
//                 let memPos_1 := mload(64)
//                 mstore(memPos_1, 1024)
//                 return(memPos_1, 32)
//             }
//             case 0x5902c17a {
//                 if callvalue() { revert(0, 0) }
//                 if slt(add(calldatasize(), not(3)), 0) { revert(0, 0) }
//                 let memPos_2 := mload(64)
//                 mstore(memPos_2, 32)
//                 return(memPos_2, 32)
//             }
//             case 0x5cc23f7d {
//                 if callvalue() { revert(0, 0) }
//                 if slt(add(calldatasize(), not(3)), 0) { revert(0, 0) }
//                 let memPos_3 := mload(64)
//                 mstore(memPos_3, 8)
//                 return(memPos_3, 32)
//             }
//             case 0x7d6d1f39 {
//                 if callvalue() { revert(0, 0) }
//                 if slt(add(calldatasize(), not(3)), 0) { revert(0, 0) }
//                 let memPos_4 := mload(64)
//                 mstore(memPos_4, 128)
//                 return(memPos_4, 32)
//             }
//             case 0x97a28744 {
//                 if callvalue() { revert(0, 0) }
//                 if slt(add(calldatasize(), not(3)), 0) { revert(0, 0) }
//                 let memPos_5 := mload(64)
//                 mstore(memPos_5, 8192)
//                 return(memPos_5, 32)
//             }
//             case 0x9f08ce4f {
//                 if callvalue() { revert(0, 0) }
//                 if slt(add(calldatasize(), not(3)), 0) { revert(0, 0) }
//                 let memPos_6 := mload(64)
//                 mstore(memPos_6, 256)
//                 return(memPos_6, 32)
//             }
//             case 0xa33eadce {
//                 if callvalue() { revert(0, 0) }
//                 if slt(add(calldatasize(), not(3)), 0) { revert(0, 0) }
//                 let memPos_7 := mload(64)
//                 mstore(memPos_7, 64)
//                 return(memPos_7, 32)
//             }
//             case 0xca035e93 {
//                 if callvalue() { revert(0, 0) }
//                 if slt(add(calldatasize(), not(3)), 0) { revert(0, 0) }
//                 let memPos_8 := mload(64)
//                 mstore(memPos_8, 4096)
//                 return(memPos_8, 32)
//             }
//             case 0xd173ee61 {
//                 if callvalue() { revert(0, 0) }
//                 if slt(add(calldatasize(), not(3)), 0) { revert(0, 0) }
//                 let memPos_9 := mload(64)
//                 mstore(memPos_9, 2048)
//                 return(memPos_9, 32)
//             }
//             case 0xf8341309 {
//                 if callvalue() { revert(0, 0) }
//                 if slt(add(calldatasize(), not(3)), 0) { revert(0, 0) }
//                 let memPos_10 := mload(64)
//                 mstore(memPos_10, 16)
//                 return(memPos_10, 32)
//             }
//         }
//         revert(0, 0)
//     }
// }
