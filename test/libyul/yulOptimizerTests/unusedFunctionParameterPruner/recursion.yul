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
//     let a1, b1, c1 := f_12(1, 2, 3)
//     function f() -> x, y, z
//     {
//         let x_1, y_2, z_3 := f_12(1, 2, 3)
//         x := x_1
//         y := y_2
//         z := z_3
//         x := add(x_1, 1)
//     }
//     function f_12(a_13, b_14, c_15) -> x_16, y_17, z_18
//     { x_16, y_17, z_18 := f() }
// }
