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
//         let _1 := mload(5)
//         let _2 := mload(4)
//         let _3 := mload(3)
//         let _4 := mload(2)
//         let c_1 := _2
//         let b_1 := _3
//         let a_1 := _4
//         let x_1 := 0
//         x_1 := add(a_1, b_1)
//         x_1 := mul(x_1, c_1)
//         let _5 := add(x_1, _1)
//         let y := add(mload(1), _5)
//     }
//     function f(a, b, c) -> x
//     {
//         x := add(a, b)
//         x := mul(x, c)
//     }
// }
