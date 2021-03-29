// https://en.wikipedia.org/wiki/Ackermann_function
// Test to see how FunctionSpecializer deals with functions that are too recursive / resource intensive.
// Unlike the test ackermann_function.yul, this one implements it using `if` and leave
{
    // 5
    sstore(0, A(2, 1))
    // 7
    sstore(1, A(2, 2))

    // Too many unrolling needed. In arbitrary precision, the value is 2**65536 - 3.
    sstore(2, A(4, 2))

    function A(m, n) -> ret {
        if eq(m, 0) {
            ret := add(n, 1)
            leave
        }

        if eq(n, 0) {
            ret := A(sub(m, 1), 1)
            leave
        }

        ret := A(sub(m, 1), A(m, sub(n, 1)))
     }
}
// ----
// step: fullSuite
//
// {
//     {
//         sstore(0, A(2, 1))
//         sstore(1, A(2, 2))
//         sstore(2, A(4, 2))
//     }
//     function A(m, n) -> ret
//     {
//         if iszero(m)
//         {
//             ret := add(n, 1)
//             leave
//         }
//         if iszero(n)
//         {
//             ret := A(add(m, not(0)), 1)
//             leave
//         }
//         let _1 := not(0)
//         ret := A(add(m, _1), A(m, add(n, _1)))
//     }
// }
