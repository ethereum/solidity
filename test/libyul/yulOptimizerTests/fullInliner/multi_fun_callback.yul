{
	// This is a test for an older version where
	// inlining was performed on a function
	// just being called. This is a problem
	// because the statemenst of the original
	// function might be in an invalid state.

	function f(x) {
		mstore(0, x)
		mstore(7, h())
		g(10)
		mstore(1, x)
	}
	function g(x) {
		f(1)
	}
	function h() -> t {
		t := 2

	}
	{
		f(100)
	}
}
// ----
// step: fullInliner
//
// {
//     { f(100) }
//     function f(x)
//     {
//         mstore(0, x)
//         let t_8 := 0
//         t_8 := 2
//         mstore(7, t_8)
//         let x_1_9 := 10
//         f(1)
//         mstore(1, x)
//     }
//     function g(x_1)
//     { f(1) }
//     function h() -> t
//     { t := 2 }
// }
