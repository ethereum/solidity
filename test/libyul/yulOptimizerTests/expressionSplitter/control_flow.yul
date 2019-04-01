{
	let x := calldataload(0)
	if mul(add(x, 2), 3) {
		for { let a := 2 } lt(a, mload(a)) { a := add(a, mul(a, 2)) } {
			let b := mul(add(a, 2), 4)
			sstore(b, mul(b, 2))
		}
	}
}
// ====
// step: expressionSplitter
// ----
// {
//     let _1 := 0
//     let x := calldataload(_1)
//     let _2 := 3
//     let _3 := 2
//     let _4 := add(x, _3)
//     let _5 := mul(_4, _2)
//     if _5
//     {
//         for {
//             let a := 2
//         }
//         lt(a, mload(a))
//         {
//             let _6 := 2
//             let _7 := mul(a, _6)
//             a := add(a, _7)
//         }
//         {
//             let _8 := 4
//             let _9 := 2
//             let _10 := add(a, _9)
//             let b := mul(_10, _8)
//             let _11 := 2
//             let _12 := mul(b, _11)
//             sstore(b, _12)
//         }
//     }
// }
