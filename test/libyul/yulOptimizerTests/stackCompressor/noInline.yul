{
  let x := 8
  function f() { let y := 9 }
}
// ----
// stackCompressor
// {
//     let x := 8
//     function f()
//     {
//         let y := 9
//     }
// }
