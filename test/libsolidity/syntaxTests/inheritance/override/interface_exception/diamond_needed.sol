interface I {
	function f() external;
	function g() external;
	function h() external;
}
interface J {
	function f() external;
	function g() external;
	function h() external;
}
contract C is I, J {
	function f() external {}
	function g() external override {}
	function h() external override(I) {}
}
// ----
// TypeError 4327: (198-222): Function needs to specify overridden contracts "I" and "J".
// TypeError 4327: (246-254): Function needs to specify overridden contracts "I" and "J".
// TypeError 4327: (281-292): Function needs to specify overridden contract "J".
