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
// expressionSplitter
// {
//     let x := calldataload(0)
//     let _1 := add(x, 2)
//     let _2 := mul(_1, 3)
//     if _2
//     {
//         for {
//             let a := 2
//         }
//         lt(a, mload(a))
//         {
//             let _3 := mul(a, 2)
//             a := add(a, _3)
//         }
//         {
//             let _4 := add(a, 2)
//             let b := mul(_4, 4)
//             let _5 := mul(b, 2)
//             sstore(b, _5)
//         }
//     }
// }
