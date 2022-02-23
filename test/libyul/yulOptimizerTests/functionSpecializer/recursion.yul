{
    sstore(0, fib(8))
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
// step: functionSpecializer
//
// {
//     sstore(0, fib(8))
//     function fib(i) -> y
//     {
//         y := 1
//         if gt(i, 2)
//         {
//             y := add(fib(sub(i, 1)), fib(sub(i, 2)))
//         }
//     }
// }
