{
	function f(a) {
		f(1)
	}
	f(mload(0))
}
// ----
// fullInliner
// {
//     {
//         let f_a := mload(0)
//         f(1)
//     }
//     function f(a)
//     {
//         f(1)
//     }
// }
