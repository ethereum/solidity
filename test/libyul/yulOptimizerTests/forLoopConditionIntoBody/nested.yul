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
// ====
// step: forLoopConditionIntoBody
// ----
// {
//     let random := 42
//     for {
//         for { let a := 1 } 1 { }
//         {
//             if iszero(iszero(eq(a, 10))) { break }
//             a := add(a, 1)
//         }
//         let b := 1
//     }
//     1
//     {
//         for { let c := 1 } 1 { c := add(c, 1) }
//         {
//             if iszero(iszero(eq(c, 2))) { break }
//             b := add(b, 1)
//         }
//     }
//     {
//         if iszero(iszero(eq(b, 10))) { break }
//         mstore(b, b)
//     }
// }
