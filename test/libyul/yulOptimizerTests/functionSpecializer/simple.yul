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
//         let a_4 := 1
//         let b_3 := 2
//         let c_2 := 3
//         sstore(a_4, b_3)
//         sstore(b_3, c_2)
//     }
//     function f(a, b, c)
//     {
//         sstore(a, b)
//         sstore(b, c)
//     }
// }
