contract C {
	event Test(function() external indexed);
	function f() public {
		emit Test(this.f);
	}
}
// ----
