{
    f(1, 2)

    let x := 1
    f(x, 2)

    f(calldataload(0), calldataload(1))

    function f(a, b) {
        sstore(a, b)
    }

}
// ----
// step: functionSpecializer
//
// {
//     f_1()
//     let x := 1
//     f_2(x)
//     f(calldataload(0), calldataload(1))
//     function f_1()
//     {
//         let a_4 := 1
//         let b_3 := 2
//         sstore(a_4, b_3)
//     }
//     function f_2(a_6)
//     {
//         let b_5 := 2
//         sstore(a_6, b_5)
//     }
//     function f(a, b)
//     { sstore(a, b) }
// }
