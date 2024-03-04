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
// TypeError 9717: (121-124): Invalid mobile type in true expression.
// TypeError 3703: (127-130): Invalid mobile type in false expression.
