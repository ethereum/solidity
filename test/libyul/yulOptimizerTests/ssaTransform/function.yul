{
  function f(a, b) -> c, d {
    b := add(b, a)
    c := add(c, b)
    d := add(d, c)
    a := add(a, d)
  }
}
// ====
// step: ssaTransform
// ----
// {
//     function f(a, b) -> c, d
//     {
//         let b_1 := add(b, a)
//         b := b_1
//         let c_2 := add(c, b_1)
//         c := c_2
//         let d_3 := add(d, c_2)
//         d := d_3
//         let a_4 := add(a, d_3)
//         a := a_4
//     }
// }
