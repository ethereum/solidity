{
    sstore(f(1), 1)
    sstore(f(2), 1)
    sstore(f(3), 1)
    function f(a) -> x {
        // The usage of a is redundant
        a := calldataload(0)
        mstore(a, x)
        // to prevent getting fully inlined
        sstore(1, 1)
        sstore(2, 2)
        sstore(3, 3)
        sstore(3, 3)
    }
}
// ----
// step: fullSuite
//
// {
//     {
//         let _1 := 0
//         let _2 := calldataload(_1)
//         mstore(_2, _1)
//         let _3 := 1
//         sstore(_3, _3)
//         let _4 := 2
//         sstore(_4, _4)
//         let _5 := 3
//         sstore(_5, _5)
//         sstore(_5, _5)
//         sstore(_1, _3)
//         mstore(_2, _1)
//         sstore(_3, _3)
//         sstore(_4, _4)
//         sstore(_5, _5)
//         sstore(_5, _5)
//         sstore(_1, _3)
//         mstore(_2, _1)
//         sstore(_3, _3)
//         sstore(_4, _4)
//         sstore(_5, _5)
//         sstore(_5, _5)
//         sstore(_1, _3)
//     }
// }
