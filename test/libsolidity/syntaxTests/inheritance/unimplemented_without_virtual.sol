abstract contract C {
	function f() external;
}
// ----
// TypeError 5424: (23-45='function f() external;'): Functions without implementation must be marked virtual.
