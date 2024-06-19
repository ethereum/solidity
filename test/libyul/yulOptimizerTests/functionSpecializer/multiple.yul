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
//         let a_1 := 1
//         let b_1 := 2
//         sstore(a_1, b_1)
//     }
//     function f_2(a_2)
//     {
//         let b_2 := 2
//         sstore(a_2, b_2)
//     }
//     function f(a, b)
//     { sstore(a, b) }
// }
