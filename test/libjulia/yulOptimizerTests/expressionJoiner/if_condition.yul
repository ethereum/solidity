{
	let a := mload(3)
	let b := sload(a)
	let c := mload(7)
	let d := add(c, b)
	if d {
		let x := mload(3)
		let y := add(x, 3)
	}
	let z := 3
	let t := add(z, 9)
}
// ----
// expressionJoiner
// {
//     if add(mload(7), sload(mload(3)))
//     {
//         let y := add(mload(3), 3)
//     }
//     let t := add(3, 9)
// }
