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
//         let a_1 := mload(0)
//         let x_1 := 0
//         let y_1 := 0
//         x_1 := mul(a_1, a_1)
//         y_1 := add(a_1, x_1)
//         let r := x_1
//         mstore(r, y_1)
//     }
//     function f(a) -> x, y
//     {
//         x := mul(a, a)
//         y := add(a, x)
//     }
// }
