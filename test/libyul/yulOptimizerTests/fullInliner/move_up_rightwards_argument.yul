{
	function f(a, b, c) -> x {
		x := add(a, b)
		x := mul(x, c)
	}
	let y := add(mload(1), add(f(mload(2), mload(3), mload(4)), mload(5)))
}
// ----
// step: fullInliner
//
// {
//     {
//         let _2 := mload(5)
//         let _4 := mload(4)
//         let _6 := mload(3)
//         let _8 := mload(2)
//         let c_13 := _4
//         let b_14 := _6
//         let a_15 := _8
//         let x_16 := 0
//         x_16 := add(a_15, b_14)
//         x_16 := mul(x_16, c_13)
//         let _10 := add(x_16, _2)
//         let y := add(mload(1), _10)
//     }
//     function f(a, b, c) -> x
//     {
//         x := add(a, b)
//         x := mul(x, c)
//     }
// }
