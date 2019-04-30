{
	function f(a) {
		f(1)
	}
	f(mload(0))
}
// ====
// step: fullInliner
// ----
// {
//     {
//         let a_4 := mload(0)
//         f(1)
//     }
//     function f(a)
//     {
//         f(1)
//     }
// }
