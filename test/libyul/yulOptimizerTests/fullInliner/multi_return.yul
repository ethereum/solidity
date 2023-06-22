{
	function f(a) -> x, y {
		x := mul(a, a)
		y := add(a, x)
	}
	let r, s := f(mload(0))
	mstore(r, s)
}
// ----
// step: fullInliner
//
// {
//     {
//         let a_2 := mload(0)
//         let x_3 := 0
//         let y_4 := 0
//         x_3 := mul(a_2, a_2)
//         y_4 := add(a_2, x_3)
//         let r := x_3
//         mstore(r, y_4)
//     }
//     function f(a) -> x, y
//     {
//         x := mul(a, a)
//         y := add(a, x)
//     }
// }
