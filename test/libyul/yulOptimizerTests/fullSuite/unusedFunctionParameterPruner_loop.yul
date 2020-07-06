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
//         sstore(f_23_88(1), 1)
//         sstore(f_23_88(2), 1)
//         sstore(f_23_88(3), 1)
//     }
//     function f_23_88(a) -> x
//     {
//         let b := 10
//         let a_1 := calldataload(x)
//         for { } iszero(b) { b := add(b, not(0)) }
//         { mstore(a_1, x) }
//     }
// }
