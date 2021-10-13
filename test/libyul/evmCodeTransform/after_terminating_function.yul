{
	let b := f(1, 2)
	function f(a, r) -> t { revert(0, 0) }
	b := f(3, 4)
}
// ====
// stackOptimization: true
// ----
//     /* "":17:18   */
//   0x02
//     /* "":14:15   */
//   0x01
//     /* "":12:19   */
//   tag_1
//   jump	// in
//     /* "":21:59   */
// tag_1:
//     /* "":55:56   */
//   0x00
//     /* "":45:57   */
//   dup1
//   revert
