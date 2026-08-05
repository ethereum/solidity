{
    mstore(0x40, memoryguard(128))
    // Like ``mutual_recursion_with_spilling_caller``, but the function reached through the cycle
    // needs its slots for excess arguments and return values rather than for local variables:
    // ``h`` has 17 parameters and a return value, so its two deepest parameters are moved to
    // memory. ``p`` calls into the cycle and holds ``b1`` across the call, so its slot has to be
    // allocated above the two argument slots of ``h``.
    //
    // p --> g <==> f --> h(a1..a17) -> r
    f()
    p()
    function f() {
        sstore(2, h(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17))
        g()
    }
    function g() {
        f()
    }
    function h(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17) -> r {
        r := add(a1, add(a2, a17))
    }
    function p() {
        let b1 := calldataload(mul(1,5))
        let b2 := calldataload(mul(2,5))
        let b3 := calldataload(mul(3,5))
        let b4 := calldataload(mul(4,5))
        let b5 := calldataload(mul(5,5))
        let b6 := calldataload(mul(6,5))
        let b7 := calldataload(mul(7,5))
        let b8 := calldataload(mul(8,5))
        let b9 := calldataload(mul(9,5))
        b1 := calldataload(mul(0,5))
        let b10 := calldataload(mul(10,5))
        let b11 := calldataload(mul(11,5))
        let b12 := calldataload(mul(12,5))
        let b13 := calldataload(mul(13,5))
        let b14 := calldataload(mul(14,5))
        let b15 := calldataload(mul(15,5))
        let b16 := calldataload(mul(16,5))
        let b17 := calldataload(mul(17,5))
        g()
        sstore(0, b1)
        sstore(mul(17,5), b17)
        sstore(mul(16,5), b16)
        sstore(mul(15,5), b15)
        sstore(mul(14,5), b14)
        sstore(mul(13,5), b13)
        sstore(mul(12,5), b12)
        sstore(mul(11,5), b11)
        sstore(mul(10,5), b10)
        sstore(mul(9,5), b9)
        sstore(mul(8,5), b8)
        sstore(mul(7,5), b7)
        sstore(mul(6,5), b6)
        sstore(mul(5,5), b5)
        sstore(mul(4,5), b4)
        sstore(mul(3,5), b3)
        sstore(mul(2,5), b2)
        sstore(mul(1,5), b1)
    }
}
// ----
// step: stackLimitEvader
//
// {
//     mstore(0x40, memoryguard(0xe0))
//     f()
//     p()
//     function f()
//     {
//         sstore(2, h(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17))
//         g()
//     }
//     function g()
//     { f() }
//     function h(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17) -> r
//     {
//         mstore(0xc0, a1)
//         mstore(0xa0, a2)
//         mstore(0x80, a17)
//         r := add(mload(0xc0), add(mload(0xa0), mload(0x80)))
//     }
//     function p()
//     {
//         mstore(0xc0, calldataload(mul(1, 5)))
//         let b2 := calldataload(mul(2, 5))
//         let b3 := calldataload(mul(3, 5))
//         let b4 := calldataload(mul(4, 5))
//         let b5 := calldataload(mul(5, 5))
//         let b6 := calldataload(mul(6, 5))
//         let b7 := calldataload(mul(7, 5))
//         let b8 := calldataload(mul(8, 5))
//         let b9 := calldataload(mul(9, 5))
//         mstore(0xc0, calldataload(mul(0, 5)))
//         let b10 := calldataload(mul(10, 5))
//         let b11 := calldataload(mul(11, 5))
//         let b12 := calldataload(mul(12, 5))
//         let b13 := calldataload(mul(13, 5))
//         let b14 := calldataload(mul(14, 5))
//         let b15 := calldataload(mul(15, 5))
//         let b16 := calldataload(mul(16, 5))
//         let b17 := calldataload(mul(17, 5))
//         g()
//         sstore(0, mload(0xc0))
//         sstore(mul(17, 5), b17)
//         sstore(mul(16, 5), b16)
//         sstore(mul(15, 5), b15)
//         sstore(mul(14, 5), b14)
//         sstore(mul(13, 5), b13)
//         sstore(mul(12, 5), b12)
//         sstore(mul(11, 5), b11)
//         sstore(mul(10, 5), b10)
//         sstore(mul(9, 5), b9)
//         sstore(mul(8, 5), b8)
//         sstore(mul(7, 5), b7)
//         sstore(mul(6, 5), b6)
//         sstore(mul(5, 5), b5)
//         sstore(mul(4, 5), b4)
//         sstore(mul(3, 5), b3)
//         sstore(mul(2, 5), b2)
//         sstore(mul(1, 5), mload(0xc0))
//     }
// }
