{
	function f(a) {
		sstore(a, a)
	}
	f(mload(0))
}
// ----
// fullInliner
// {
//     {
//         let f_a := mload(0)
//         sstore(f_a, f_a)
//     }
//     function f(a)
//     {
//         sstore(a, a)
//     }
// }
