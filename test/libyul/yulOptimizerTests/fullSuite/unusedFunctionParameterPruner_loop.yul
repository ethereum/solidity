{
    sstore(f(1), 1)
    sstore(f(2), 1)
    sstore(f(3), 1)
    function f(a) -> x {
        for {let b := 10} iszero(b) { b := sub(b, 1) }
        {
            a := calldataload(0)
            mstore(a, x)
        }
    }
}
// ----
// step: fullSuite
//
// {
//     {
//         let b := 10
//         let _1 := 0
//         let a := calldataload(_1)
//         for { } iszero(b) { b := add(b, not(0)) }
//         { mstore(a, _1) }
//         sstore(_1, 1)
//         let b_1 := 10
//         for { } iszero(b_1) { b_1 := add(b_1, not(0)) }
//         { mstore(a, _1) }
//         sstore(_1, 1)
//         let b_2 := 10
//         for { } iszero(b_2) { b_2 := add(b_2, not(0)) }
//         { mstore(a, _1) }
//         sstore(_1, 1)
//     }
// }
