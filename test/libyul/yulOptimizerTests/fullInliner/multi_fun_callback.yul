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
//         {
//             let f_x := 100
//             mstore(0, f_x)
//             mstore(7, h())
//             g(10)
//             mstore(1, f_x)
//         }
//     }
//     function f(x)
//     {
//         mstore(0, x)
//         let h_t
//         h_t := 2
//         mstore(7, h_t)
//         let g_x_1 := 10
//         f(1)
//         mstore(1, x)
//     }
//     function g(x_1)
//     {
//         let f_x_8 := 1
//         mstore(0, f_x_8)
//         let f_h_t
//         f_h_t := 2
//         mstore(7, f_h_t)
//         let f_g_x_1 := 10
//         f(1)
//         mstore(1, f_x_8)
//     }
//     function h() -> t
//     {
//         t := 2
//     }
// }
