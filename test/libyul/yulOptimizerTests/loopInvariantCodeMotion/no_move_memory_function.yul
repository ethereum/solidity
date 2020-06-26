{
  let b := 1
  function f() -> z { z := msize() }
  for { let a := 1 } iszero(eq(a, 10)) { a := add(a, 1) } {
    let inv := add(b, 42)
    let x := keccak256(inv, 32)
	let z := f() // Will prevent moving keccak256
    a := add(x, 1)
    sstore(a, inv)
  }
  for { let a := 1 } iszero(eq(a, 10)) { a := add(a, 1) } {
    let inv := add(b, 42)
    let x := mload(mul(inv, 3))
    a := add(x, 1)
    mstore(a, inv) // Will prevent moving mload
  }
}
// ----
// step: loopInvariantCodeMotion
//
// {
//     let b := 1
//     function f() -> z
//     { z := msize() }
//     let a := 1
//     let inv := add(b, 42)
//     for { } iszero(eq(a, 10)) { a := add(a, 1) }
//     {
//         let x := keccak256(inv, 32)
//         let z_1 := f()
//         a := add(x, 1)
//         sstore(a, inv)
//     }
//     let a_2 := 1
//     let inv_3 := add(b, 42)
//     for { } iszero(eq(a_2, 10)) { a_2 := add(a_2, 1) }
//     {
//         let x_4 := mload(mul(inv_3, 3))
//         a_2 := add(x_4, 1)
//         mstore(a_2, inv_3)
//     }
// }
