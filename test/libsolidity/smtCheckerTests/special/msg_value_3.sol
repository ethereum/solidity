contract C {
	bool lock = true;
	function f() public {
		lock = false;
		g();
		lock = true;
	}
	function g() public payable {
		require(lock == false);
		assert(msg.value == 0);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1180: Contract invariant(s) for :C:\nlock\n
