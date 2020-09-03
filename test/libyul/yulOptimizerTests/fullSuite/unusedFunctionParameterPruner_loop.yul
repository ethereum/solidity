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
//         f()
//         sstore(0, 1)
//         f()
//         sstore(0, 1)
//         f()
//         sstore(0, 1)
//     }
//     function f()
//     {
//         let b := 10
//         let _1 := 0
//         let a := calldataload(_1)
//         for { } iszero(b) { b := add(b, not(0)) }
//         { mstore(a, _1) }
//     }
// }
