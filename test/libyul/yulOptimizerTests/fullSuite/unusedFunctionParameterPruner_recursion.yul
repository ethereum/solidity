{
    let j, k, l := f(1, 2, 3)
    sstore(0, j)
    sstore(1, k)
    sstore(1, l)
    function f(a, b, c) -> x, y, z
    {
        if calldataload(0) {
            x, y, z := f(1, 2, 3)
        }
        x := add(x, 1)
    }
}
// ----
// step: fullSuite
//
// {
//     {
//         let x, y, z := f()
//         sstore(0, x)
//         sstore(1, z)
//     }
//     function f() -> x, y, z
//     {
//         if calldataload(0)
//         {
//             let x_1, y_1, z_1 := f()
//             x := x_1
//             y := y_1
//             z := z_1
//         }
//         x := add(x, 1)
//     }
// }
