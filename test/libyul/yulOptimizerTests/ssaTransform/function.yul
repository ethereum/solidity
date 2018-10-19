{
  function f(a, b) -> c, d {
    b := add(b, a)
    c := add(c, b)
    d := add(d, c)
    a := add(a, d)
  }
}
// ----
// ssaTransform
// {
//     function f(a, b) -> c, d
//     {
//         let b_1 := add(b, a)
//         b := b_1
//         let c_1 := add(c, b_1)
//         c := c_1
//         let d_1 := add(d, c_1)
//         d := d_1
//         let a_1 := add(a, d_1)
//         a := a_1
//     }
// }
