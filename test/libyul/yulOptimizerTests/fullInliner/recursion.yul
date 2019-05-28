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
//     { f(mload(0)) }
//     function f(a)
//     { f(1) }
// }
