{
	let a := mload(3)
	let b := sload(a)
	let c := mload(7)
	let d := add(c, b)
	switch d
	case 3 {
		let x := mload(3)
		let y := add(x, 3)
	}
	default {
		sstore(1, 0)
	}
	let z := 3
	let t := add(z, 9)
}
// ====
// step: expressionJoiner
// ----
// {
//     switch add(mload(7), sload(mload(3)))
//     case 3 { let y := add(mload(3), 3) }
//     default { sstore(1, 0) }
//     let t := add(3, 9)
// }
