{
    // All arguments are constants
    f(1, 2, 3)

    function f(a, b, c) {
        sstore(a, b)
        sstore(b, c)
    }
}
// ----
// step: functionSpecializer
//
// {
//     f_1()
//     function f_1()
//     {
//         let a_1 := 1
//         let b_1 := 2
//         let c_1 := 3
//         sstore(a_1, b_1)
//         sstore(b_1, c_1)
//     }
//     function f(a, b, c)
//     {
//         sstore(a, b)
//         sstore(b, c)
//     }
// }
