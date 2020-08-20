{
    f(1, 2)
    function f(a, b)
    {
        sstore(a, 0)
        a := add(a, 1)
    }
}
// ----
// step: unusedFunctionParameterPruner
//
// {
//     f_7(1, 2)
//     function f(a)
//     {
//         let a_2 := a
//         sstore(a_2, 0)
//         a := add(a_2, 1)
//     }
//     function f_7(a_8, b_9)
//     { f(a_8) }
// }
