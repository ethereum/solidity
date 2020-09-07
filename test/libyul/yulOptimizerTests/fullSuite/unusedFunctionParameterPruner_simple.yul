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
//         f()
//         sstore(0, 1)
//         f()
//         sstore(0, 1)
//         f()
//         sstore(0, 1)
//     }
//     function f()
//     {
//         mstore(calldataload(0), 0)
//         sstore(1, 1)
//         sstore(2, 2)
//         sstore(3, 3)
//         sstore(3, 3)
//     }
// }
