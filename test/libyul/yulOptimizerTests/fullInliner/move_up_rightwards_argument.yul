{
	function f(a, b, c) -> x {
		x := add(a, b)
		x := mul(x, c)
	}
	let y := add(mload(1), add(f(mload(2), mload(3), mload(4)), mload(5)))
}
// ----
// fullInliner
// {
//     {
//         let _2 := mload(5)
//         let _4 := mload(4)
//         let _6 := mload(3)
//         let f_a := mload(2)
//         let f_b := _6
//         let f_c := _4
//         let f_x := 0
//         f_x := add(f_a, f_b)
//         f_x := mul(f_x, f_c)
//         let _10 := add(f_x, _2)
//         let y := add(mload(1), _10)
//     }
//     function f(a, b, c) -> x
//     {
//         x := add(a, b)
//         x := mul(x, c)
//     }
// }
