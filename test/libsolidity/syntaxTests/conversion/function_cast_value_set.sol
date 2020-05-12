contract C {
	function f() public payable {
		function() external payable x = this.f{value: 7};
	}
}
// ----
// TypeError: (46-94): Type function () payable external is not implicitly convertible to expected type function () payable external.
