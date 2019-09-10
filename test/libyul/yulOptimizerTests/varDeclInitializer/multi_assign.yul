{
  function f() -> x, y {
    let a, b := f()
	let u
  }
  let r
  let s := 3
  let t
}
// ====
// step: varDeclInitializer
// ----
// {
//     function f() -> x, y
//     {
//         let a, b := f()
//         let u := 0
//     }
//     let r := 0
//     let s := 3
//     let t := 0
// }
