{
    let c, d := f(1, 2)
    sstore(c, 1)
    function f(x, y) -> a, b
    {
        a := sload(x)
        b := sload(y)
    }
}
// ----
// step: unusedFunctionReturnParameterPruner
//
// {
//     let c, d := f_1(1, 2)
//     sstore(c, 1)
//     function f(x, y) -> a
//     {
//         let b
//         a := sload(x)
//         b := sload(y)
//     }
//     function f_1(x_2, y_3) -> a_4, b_5
//     { a_4 := f(x_2, y_3) }
// }
