contract D {
	function f() external {}
	function g() external {}
}
contract C {
	function f(bool c) public pure {
		(c ? D.f : D.g);
	}
}
// ----
// TypeError 1080: (117-130): True expression's type function D.f() does not match false expression's type function D.g().
