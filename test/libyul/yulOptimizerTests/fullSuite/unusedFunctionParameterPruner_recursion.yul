{
    let j, k, l := f(1, 2, 3)
    sstore(0, j)
    sstore(1, k)
    sstore(1, l)
    function f(a, b, c) -> x, y, z
    {
        x, y, z := f(1, 2, 3)
        x := add(x, 1)
    }
}
// ----
// step: fullSuite
//
// {
//     {
//         let x, y, z := f_15()
//         sstore(0, add(x, 1))
//         sstore(1, y)
//         sstore(1, z)
//     }
//     function f_15() -> x, y, z
//     {
//         let x_1, y_1, z_1 := f_15()
//         x := add(x_1, 1)
//         y := y_1
//         z := z_1
//     }
// }
