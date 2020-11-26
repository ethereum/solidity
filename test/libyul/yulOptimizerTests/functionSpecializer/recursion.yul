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
//     sstore(0, fib_1())
//     function fib_1() -> y_2
//     {
//         let i_3 := 8
//         y_2 := 1
//         if gt(i_3, 2)
//         {
//             y_2 := add(fib(sub(i_3, 1)), fib(sub(i_3, 2)))
//         }
//     }
//     function fib(i) -> y
//     {
//         y := 1
//         if gt(i, 2)
//         {
//             y := add(fib(sub(i, 1)), fib(sub(i, 2)))
//         }
//     }
// }
