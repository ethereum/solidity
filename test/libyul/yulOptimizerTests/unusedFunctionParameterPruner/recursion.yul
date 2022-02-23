{
    let a1, b1, c1 := f(1, 2, 3)
    function f(a, b, c) -> x, y, z
    {
        x, y, z := f(1, 2, 3)
        x := add(x, 1)
    }
}
// ----
// step: unusedFunctionParameterPruner
//
// {
//     let a1, b1, c1 := f_1(1, 2, 3)
//     function f() -> x, y, z
//     {
//         x, y, z := f_1(1, 2, 3)
//         x := add(x, 1)
//     }
//     function f_1(a_2, b_3, c_4) -> x_5, y_6, z_7
//     { x_5, y_6, z_7 := f() }
// }
