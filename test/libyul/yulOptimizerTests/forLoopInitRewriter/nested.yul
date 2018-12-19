{
  let random := 42
  for {
    for { let a := 1} iszero(eq(a,10)) {} {
      a := add(a, 1)
    }
	let b := 1
  } iszero(eq(b, 10)) {
    for { let c := 1 } iszero(eq(c,2)) { c := add(c, 1) } {
      b := add(b, 1)
    }
  } {
    mstore(b,b)
  }
}
// ----
// forLoopInitRewriter
// {
//     let random := 42
//     let a := 1
//     for {
//     }
//     iszero(eq(a, 10))
//     {
//     }
//     {
//         a := add(a, 1)
//     }
//     let b := 1
//     for {
//     }
//     iszero(eq(b, 10))
//     {
//         let c := 1
//         for {
//         }
//         iszero(eq(c, 2))
//         {
//             c := add(c, 1)
//         }
//         {
//             b := add(b, 1)
//         }
//     }
//     {
//         mstore(b, b)
//     }
// }
