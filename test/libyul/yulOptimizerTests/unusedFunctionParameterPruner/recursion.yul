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
//     function f_1(a, b, c) -> x_1, y_1, z_1
//     { x_1, y_1, z_1 := f() }
// }
