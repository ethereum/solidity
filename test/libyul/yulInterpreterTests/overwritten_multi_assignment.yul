{
	function f() -> a, b { a := 1 b := 2 }
	let x
	x, x := f()
	mstore(0, x)
}
// ----
// Trace:
// Memory dump:
//      0: 0000000000000000000000000000000000000000000000000000000000000002
// Storage dump:
