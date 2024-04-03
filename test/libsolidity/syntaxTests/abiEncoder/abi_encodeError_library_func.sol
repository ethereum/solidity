library L {
	function g(uint) public {}
	function h(uint) internal {}
}

function f() {
    abi.encodeError(L.g, (1));
    abi.encodeError(L.h, (1));
}
// ----
// TypeError 3510: (108-111): Expected an error type. Cannot use functions for abi.encodeError.
// TypeError 3510: (139-142): Expected an error type. Cannot use functions for abi.encodeError.

