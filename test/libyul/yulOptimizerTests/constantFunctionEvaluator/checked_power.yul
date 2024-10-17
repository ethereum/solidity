{
    function test_ok() -> r1, r2, r3, r4, r5, r6, r7, r8, r9
    {
        r9 := checked_exp_t_uint256_t_uint256(10, 18)
        r8 := checked_exp_t_uint256_t_uint256(2, 255)
        r7 := checked_exp_t_uint256_t_uint256(1, not(0))
        r6 := checked_exp_t_uint256_t_uint256(0, 0)
        r5 := checked_exp_t_uint256_t_uint256(not(0), 0)
        r4 := checked_exp_t_uint256_t_uint256(not(0), 1)
        r3 := checked_exp_t_uint256_t_uint256(10, 77)
        r2 := checked_exp_t_uint256_t_uint256(306, 31)
        r1 := checked_exp_t_uint256_t_uint256(shl(127, 1), 2)
    }
    function test_overflow1() -> r
    { r := checked_exp_t_uint256_t_uint256(not(0), not(0)) }
    function test_overflow2() -> r
    { r := checked_exp_t_uint256_t_uint256(2, 256) }
    function test_overflow3() -> r
    { r := checked_exp_t_uint256_t_uint256(11, 78) }
    function test_overflow4() -> r
    { r := checked_exp_t_uint256_t_uint256(307, 32) }
    function test_overflow5() -> r
    { r := checked_exp_t_uint256_t_uint256(10, 78) }
    function test_overflow6() -> r
    { r := checked_exp_t_uint256_t_uint256(50859009, 10) }

    function panic_error_0x11() {
        mstore(0, 35408467139433450592217433187231851964531694900788300625387963629091585785856)
        mstore(4, 0x11)
        revert(0, 0x24)
    }
    function shift_right_1_unsigned(value) -> newValue {
        newValue := shr(1, value)
    }
    function checked_exp_helper(_power, _base, exponent, max) -> power, base {
        power := _power
        base  := _base
        for { } gt(exponent, 1) {}
        {
            // overflow check for base * base
            if gt(base, div(max, base)) { panic_error_0x11() }
            if and(exponent, 1)
            {
                // No checks for power := mul(power, base) needed, because the check
                // for base * base above is sufficient, since:
                // |power| <= base (proof by induction) and thus:
                // |power * base| <= base * base <= max <= |min| (for signed)
                // (this is equally true for signed and unsigned exp)
                power := mul(power, base)
            }
            base := mul(base, base)
            exponent := shift_right_1_unsigned(exponent)
        }
    }
    function checked_exp_unsigned(base, exponent, max) -> power {
        // This function currently cannot be inlined because of the
        // "leave" statements. We have to improve the optimizer.

        // Note that 0**0 == 1
        if iszero(exponent) { power := 1 leave }
        if iszero(base) { power := 0 leave }

        // Specializations for small bases
        switch base
        // 0 is handled above
        case 1 { power := 1 leave }
        case 2
        {
            if gt(exponent, 255) { panic_error_0x11() }
            power := exp(2, exponent)
            if gt(power, max) { panic_error_0x11() }
            leave
        }
        if or(
            and(lt(base, 11), lt(exponent, 78)),
            and(lt(base, 307), lt(exponent, 32))
        )
        {
            power := exp(base, exponent)
            if gt(power, max) { panic_error_0x11() }
            leave
        }

        power, base := checked_exp_helper(1, base, exponent, max)

        if gt(power, div(max, base)) { panic_error_0x11() }
        power := mul(power, base)
    }
    function cleanup_t_uint256(value) -> cleaned {
        cleaned := value
    }
    function checked_exp_t_uint256_t_uint256(base, exponent) -> power {
        base := cleanup_t_uint256(base)
        exponent := cleanup_t_uint256(exponent)
        power := checked_exp_unsigned(base, exponent, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    }
}
// ----
// step: constantFunctionEvaluator
//
// {
//     function test_ok() -> r1, r2, r3, r4, r5, r6, r7, r8, r9
//     {
//         r1 := 28948022309329048855892746252171976963317496166410141009864396001978282409984
//         r2 := 114120645878241659292862104872151745131076900847207153599812066301423392915456
//         r3 := 100000000000000000000000000000000000000000000000000000000000000000000000000000
//         r4 := 115792089237316195423570985008687907853269984665640564039457584007913129639935
//         r5 := 1
//         r6 := 1
//         r7 := 1
//         r8 := 57896044618658097711785492504343953926634992332820282019728792003956564819968
//         r9 := 1000000000000000000
//     }
//     function test_overflow1() -> r
//     {
//         r := checked_exp_t_uint256_t_uint256(not(0), not(0))
//     }
//     function test_overflow2() -> r_1
//     {
//         r_1 := checked_exp_t_uint256_t_uint256(2, 256)
//     }
//     function test_overflow3() -> r_2
//     {
//         r_2 := checked_exp_t_uint256_t_uint256(11, 78)
//     }
//     function test_overflow4() -> r_3
//     {
//         r_3 := checked_exp_t_uint256_t_uint256(307, 32)
//     }
//     function test_overflow5() -> r_4
//     {
//         r_4 := checked_exp_t_uint256_t_uint256(10, 78)
//     }
//     function test_overflow6() -> r_5
//     {
//         r_5 := checked_exp_t_uint256_t_uint256(50859009, 10)
//     }
//     function panic_error_0x11()
//     {
//         mstore(0, 35408467139433450592217433187231851964531694900788300625387963629091585785856)
//         mstore(4, 0x11)
//         revert(0, 0x24)
//     }
//     function shift_right_1_unsigned(value) -> newValue
//     { newValue := shr(1, value) }
//     function checked_exp_helper(_power, _base, exponent, max) -> power, base
//     {
//         power := _power
//         base := _base
//         for { } gt(exponent, 1) { }
//         {
//             if gt(base, div(max, base)) { panic_error_0x11() }
//             if and(exponent, 1) { power := mul(power, base) }
//             base := mul(base, base)
//             exponent := shift_right_1_unsigned(exponent)
//         }
//     }
//     function checked_exp_unsigned(base_6, exponent_7, max_8) -> power_9
//     {
//         if iszero(exponent_7)
//         {
//             power_9 := 1
//             leave
//         }
//         if iszero(base_6)
//         {
//             power_9 := 0
//             leave
//         }
//         switch base_6
//         case 1 {
//             power_9 := 1
//             leave
//         }
//         case 2 {
//             if gt(exponent_7, 255) { panic_error_0x11() }
//             power_9 := exp(2, exponent_7)
//             if gt(power_9, max_8) { panic_error_0x11() }
//             leave
//         }
//         if or(and(lt(base_6, 11), lt(exponent_7, 78)), and(lt(base_6, 307), lt(exponent_7, 32)))
//         {
//             power_9 := exp(base_6, exponent_7)
//             if gt(power_9, max_8) { panic_error_0x11() }
//             leave
//         }
//         power_9, base_6 := checked_exp_helper(1, base_6, exponent_7, max_8)
//         if gt(power_9, div(max_8, base_6)) { panic_error_0x11() }
//         power_9 := mul(power_9, base_6)
//     }
//     function cleanup_t_uint256(value_10) -> cleaned
//     { cleaned := value_10 }
//     function checked_exp_t_uint256_t_uint256(base_11, exponent_12) -> power_13
//     {
//         base_11 := cleanup_t_uint256(base_11)
//         exponent_12 := cleanup_t_uint256(exponent_12)
//         power_13 := checked_exp_unsigned(base_11, exponent_12, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
//     }
// }
