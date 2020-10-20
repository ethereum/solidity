{
    // second return paramter is unused
    let j, k, l := f(1, 2, 3)
    sstore(0, j)
    sstore(1, l)
    function f(a, b, c) -> x, y, z
    {
        // second return parameter is unused
        let x_1, y_1, z_1 := f(1, 2, 3)
        x := x_1
        z := z_1
        x := add(x, 1)
    }
}
// ----
// step: fullSuite
//
// {
//     {
//         let x, z := f()
//         sstore(0, x)
//         sstore(1, z)
//     }
//     function f() -> x, z
//     {
//         let x_1, z_1 := f()
//         z := z_1
//         x := add(x_1, 1)
//     }
// }
