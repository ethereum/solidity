{
  let b := 1
  for { let a := 1 } iszero(eq(a, 10)) { a := add(a, 1) } {
    let inv := add(b, 42)
    let x := keccak256(inv, 32)
	let z := msize() // Will prevent moving of keccak256
    a := add(x, 1)
    sstore(a, inv)
  }
  for { let a := 1 } iszero(eq(a, 10)) { a := add(a, 1) } {
    let inv := add(b, 42)
    let x := mload(mul(inv, 3))
    a := add(x, 1)
    mstore(a, inv) // Will prevent moving of mload
  }

  // Condition will prevent moving
  for { let a := 1 } iszero(msize()) { a := add(a, 1) } {
    let inv := add(b, 42)
    let x := mload(mul(inv, 3))
    a := add(x, 1)
    sstore(a, inv)
  }

  // for-post statement will prevent moving
  for { let a := 1 } iszero(a) { a := msize() } {
    let inv := add(b, 42)
    let x := mload(mul(inv, 3))
    a := add(x, 1)
    sstore(a, inv)
  }

}
// ----
// step: loopInvariantCodeMotion
//
// {
//     let b := 1
//     let a := 1
//     let inv := add(b, 42)
//     for { } iszero(eq(a, 10)) { a := add(a, 1) }
//     {
//         let x := keccak256(inv, 32)
//         let z := msize()
//         a := add(x, 1)
//         sstore(a, inv)
//     }
//     let a_1 := 1
//     let inv_2 := add(b, 42)
//     for { } iszero(eq(a_1, 10)) { a_1 := add(a_1, 1) }
//     {
//         let x_3 := mload(mul(inv_2, 3))
//         a_1 := add(x_3, 1)
//         mstore(a_1, inv_2)
//     }
//     let a_4 := 1
//     let inv_5 := add(b, 42)
//     for { } iszero(msize()) { a_4 := add(a_4, 1) }
//     {
//         let x_6 := mload(mul(inv_5, 3))
//         a_4 := add(x_6, 1)
//         sstore(a_4, inv_5)
//     }
//     let a_7 := 1
//     let inv_8 := add(b, 42)
//     for { } iszero(a_7) { a_7 := msize() }
//     {
//         let x_9 := mload(mul(inv_8, 3))
//         a_7 := add(x_9, 1)
//         sstore(a_7, inv_8)
//     }
// }
