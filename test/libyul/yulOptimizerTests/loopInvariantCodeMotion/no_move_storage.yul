{
  let b := 1
  // invalidates storage in post
  for { let a := 1 } iszero(eq(a, 10)) { sstore(0x00, 0x01)} {
    let inv := add(b, 42)
    let x := sload(mul(inv, 3))
    a := add(x, 1)
    mstore(a, inv)
  }

  // invalidates storage in body
  for { let a := 1 } iszero(eq(a, 10)) { a := add(a, 1) } {
    let inv := add(b, 42)
    let x := sload(mul(inv, 3))
    a := add(x, 1)
    sstore(a, inv)
  }

  // invalidates state in body
  for { let a := 1 } iszero(eq(a, 10)) { a := add(a, 1) } {
    let inv := add(b, 42)
    pop(callcode(100, 0x010, 10, 0x00, 32, 0x0100, 32))
    let x := sload(mul(inv, 3))
    a := add(x, 1)
  }

}
// ----
// step: loopInvariantCodeMotion
//
// {
//     let b := 1
//     let a := 1
//     let inv := add(b, 42)
//     for { } iszero(eq(a, 10)) { sstore(0x00, 0x01) }
//     {
//         let x := sload(mul(inv, 3))
//         a := add(x, 1)
//         mstore(a, inv)
//     }
//     let a_1 := 1
//     let inv_1 := add(b, 42)
//     for { } iszero(eq(a_1, 10)) { a_1 := add(a_1, 1) }
//     {
//         let x_1 := sload(mul(inv_1, 3))
//         a_1 := add(x_1, 1)
//         sstore(a_1, inv_1)
//     }
//     let a_2 := 1
//     let inv_2 := add(b, 42)
//     for { } iszero(eq(a_2, 10)) { a_2 := add(a_2, 1) }
//     {
//         pop(callcode(100, 0x010, 10, 0x00, 32, 0x0100, 32))
//         let x_2 := sload(mul(inv_2, 3))
//         a_2 := add(x_2, 1)
//     }
// }
