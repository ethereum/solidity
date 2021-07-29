contract C {
    function test() public pure {
        bool fine1 = (ufixed(1/3) == 1/2);
        bool fine2 = (1/2 == 1/2);
        bool invalid1 = (1/3 + 1/(10**300) == 1/3 - 1/(10**300));
        bool invalid2 = (1/3 == 1/3);
        bool invalid3 = (ufixed(1/3) == 1/3);
    }
}
// ----
// TypeError 2271: (150-188): Operator == not compatible with types rational_const 1000...(293 digits omitted)...0003 / 3000...(293 digits omitted)...0000 and rational_const 9999...(292 digits omitted)...9997 / 3000...(293 digits omitted)...0000
// TypeError 2271: (216-226): Operator == not compatible with types rational_const 1 / 3 and rational_const 1 / 3
// TypeError 2271: (254-272): Operator == not compatible with types ufixed128x18 and rational_const 1 / 3
