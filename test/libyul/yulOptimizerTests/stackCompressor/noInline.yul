{
  let x := 8
  function f() { let y := 9 }
}
// ====
// step: stackCompressor
// ----
// {
//     let x := 8
//     function f()
//     { let y := 9 }
// }
