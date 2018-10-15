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
//         let _1 := mload(5)
//         let f_c := mload(4)
//         let f_b := mload(3)
//         let f_a := mload(2)
//         let f_x
//         {
//             f_x := add(f_a, f_b)
//             f_x := mul(f_x, f_c)
//         }
//         let y := add(mload(1), add(f_x, _1))
//     }
//     function f(a, b, c) -> x
//     {
//         x := add(a, b)
//         x := mul(x, c)
//     }
// }
