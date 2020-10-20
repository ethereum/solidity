{
    // The third and the last return value is unused
    let a, b, c, d, e := f(sload(0))
    sstore(a, b)
    let a1, b1, c1, d1, e1 := f(sload(1))
    sstore(a1, d1)
    let a2, b2, c2, d2, e2 := f(sload(2))
    sstore(d2, b2)
    function f(a_1) -> v, w, x, y, z
    {
       w := mload(a_1)
       y := mload(w)
       z := mload(y)
       sstore(y, z)
    }
}
// ----
// step: unusedFunctionReturnParameterPruner
//
// {
//     let a, b, c, d, e := f_1(sload(0))
//     sstore(a, b)
//     let a1, b1, c1, d1, e1 := f_1(sload(1))
//     sstore(a1, d1)
//     let a2, b2, c2, d2, e2 := f_1(sload(2))
//     sstore(d2, b2)
//     function f(a_1) -> v, w, y
//     {
//         let x
//         let z
//         w := mload(a_1)
//         y := mload(w)
//         z := mload(y)
//         sstore(y, z)
//     }
//     function f_1(a_1_2) -> v_3, w_4, x_5, y_6, z_7
//     { v_3, w_4, y_6 := f(a_1_2) }
// }
