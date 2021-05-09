{
    sstore(0, fib(0))
    sstore(1, fib(2))
    sstore(2, fib(3))
    sstore(3, fib(4))
    sstore(4, fib(5))
    sstore(5, fib(6))
    sstore(7, fib(7))
    sstore(8, fib(8))
    sstore(9, fib(9))
    sstore(10, fib(10))


    function fib(i) -> y
    {
        y := 1
        if gt(i, 2)
        {
            y := add(fib(sub(i, 1)), fib(sub(i, 2)))
        }
    }
}
// ----
// step: fullSuite
//
// {
//     {
//         sstore(0, fib(0))
//         sstore(1, fib(2))
//         sstore(2, fib(3))
//         sstore(3, fib(4))
//         sstore(4, fib(5))
//         sstore(5, fib(6))
//         sstore(7, fib(7))
//         sstore(8, fib(8))
//         sstore(9, fib(9))
//         sstore(10, fib(10))
//     }
//     function fib(i) -> y
//     {
//         y := 1
//         if gt(i, 2)
//         {
//             y := add(fib(add(i, not(0))), fib(add(i, not(1))))
//         }
//     }
// }
