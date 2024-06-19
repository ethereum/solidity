{
  function f(a, b) -> c, d {
    b := add(b, a)
    c := add(c, b)
    d := add(d, c)
    a := add(a, d)
  }
}
// ----
// step: ssaTransform
//
// {
//     function f(a, b) -> c, d
//     {
//         let a_2 := a
//         let b_2 := b
//         let b_1 := add(b_2, a_2)
//         b := b_1
//         let c_1 := add(c, b_1)
//         c := c_1
//         let d_1 := add(d, c_1)
//         d := d_1
//         let a_1 := add(a_2, d_1)
//         a := a_1
//     }
// }
