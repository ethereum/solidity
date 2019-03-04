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
// fullInliner
// {
//     {
//         let x_8 := 100
//         mstore(0, x_8)
//         mstore(7, h())
//         g(10)
//         mstore(1, x_8)
//     }
//     function f(x)
//     {
//         mstore(0, x)
//         let t_20 := 0
//         t_20 := 2
//         mstore(7, t_20)
//         let x_1_21 := 10
//         let x_14_23 := 1
//         mstore(0, x_14_23)
//         mstore(7, h())
//         g(10)
//         mstore(1, x_14_23)
//         mstore(1, x)
//     }
//     function g(x_1)
//     {
//         let x_14 := 1
//         mstore(0, x_14)
//         mstore(7, h())
//         g(10)
//         mstore(1, x_14)
//     }
//     function h() -> t
//     {
//         t := 2
//     }
// }
