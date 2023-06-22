{
	let x := calldataload(0)
	if mul(add(x, 2), 3) {
		for { let a := 2 } lt(a, mload(a)) { a := add(a, mul(a, 2)) } {
			let b := mul(add(a, 2), 4)
			sstore(b, mul(b, 2))
		}
	}
}
// ----
// step: expressionSplitter
//
// {
//     let x := calldataload(0)
//     let _1 := 3
//     let _2 := 2
//     let _3 := add(x, _2)
//     let _4 := mul(_3, _1)
//     if _4
//     {
//         for { let a := 2 }
//         lt(a, mload(a))
//         {
//             let _5 := 2
//             let _6 := mul(a, _5)
//             a := add(a, _6)
//         }
//         {
//             let _7 := 4
//             let _8 := 2
//             let _9 := add(a, _8)
//             let b := mul(_9, _7)
//             let _10 := 2
//             let _11 := mul(b, _10)
//             sstore(b, _11)
//         }
//     }
// }
