{
	function f(a) -> x {
		let r := mul(a, a)
		x := add(r, r)
	}
	let y := add(f(sload(mload(2))), mload(7))
}
// ----
// fullInliner
// {
//     {
//         let _1 := mload(7)
//         let f_a := sload(mload(2))
//         let f_x
//         {
//             let f_r := mul(f_a, f_a)
//             f_x := add(f_r, f_r)
//         }
//         let y := add(f_x, _1)
//     }
//     function f(a) -> x
//     {
//         let r := mul(a, a)
//         x := add(r, r)
//     }
// }
