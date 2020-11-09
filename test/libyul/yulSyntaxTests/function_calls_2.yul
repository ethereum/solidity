{
	{ function f() {} }
	{ function f() { let y := 2 } }
	{ function f() -> z { let y := 2 } }
	{ function f(a) { let y := 2 } }
	{ function f(a) { let y := a } }
	{ function f() -> x, y, z {} }
	{ function f(x, y, z) {} }
	{ function f(a, b) -> x, y, z { y := a } }
	{ function f() {} f() }
	{ function f() -> x, y { x := 1 y := 2} let a, b := f() }
	{ function f(a, b) -> x, y { x := b y := a } let a, b := f(2, 3) }
	{ function rec(a) { rec(sub(a, 1)) } rec(2) }
	{ let r := 2 function f() -> x, y { x := 1 y := 2} let a, b := f() b := r }
	{ function f() { g() } function g() { f() } }
	{ function f(a) -> b {} function g(a, b, c) {} function x() { g(1, 2, f(mul(2, 3))) x() } }
}
