{
    function test_ok() -> r1, r2
    {
        r1 := checked_exp_t_uint256_t_uint256(10, 18)
        r2 := checked_exp_t_uint256_t_uint256(shl(127, 1), 2)
    }
    function test_overflow() -> r
    { r := checked_exp_t_uint256_t_uint256(not(0), not(0)) }

    let a1, a2 := test_ok()
   sstore(0, add(a1, a2))
    sstore(1, test_overflow())

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
// ====
// EVMVersion: >=cancun
// ----
// step: fullSuite
//
// {
//     {
//         sstore(0, add(shl(254, 1), 0x0de0b6b3a7640000))
//         let power := 0
//         let _1 := 0
//         _1 := 0
//         _1 := 0
//         _1 := 0
//         let exponent := not(0)
//         let power_1 := 0
//         let base := 0
//         power_1 := 1
//         base := exponent
//         for { } gt(exponent, 1) { }
//         {
//             if gt(base, div(not(0), base)) { panic_error_0x11() }
//             if and(exponent, 1) { power_1 := mul(power_1, base) }
//             base := mul(base, base)
//             exponent := shr(1, exponent)
//         }
//         if gt(power_1, div(not(0), base)) { panic_error_0x11() }
//         power := mul(power_1, base)
//         sstore(1, power)
//     }
//     function panic_error_0x11()
//     {
//         mstore(0, shl(224, 0x4e487b71))
//         mstore(4, 0x11)
//         revert(0, 0x24)
//     }
// }
