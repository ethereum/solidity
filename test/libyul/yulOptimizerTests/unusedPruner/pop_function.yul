{
	let a := f()
	pop(a)
	function f() -> y {
		sstore(1, sload(1))
	}
}
// ----
// step: unusedPruner
//
// {
//     pop(f())
//     function f() -> y
//     { sstore(1, sload(1)) }
// }
