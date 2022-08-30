contract C {
	string log;
	function() external[] fs;
	function() external[] gs;

	function a() external {
		log = string.concat(log, "[a called]");
	}
	function b() external {
		log = string.concat(log, "[b called]");
	}

	function f(function() external[] calldata x) external {
		fs = x;
	}
	function g(function() external[] memory x) public {
		fs = x;
	}
	function test() external returns (string memory) {
		log = "";
		function() external[] memory x = new function() external[](2);
		x[0] = this.a;
		x[1] = this.b;
		this.f(x);
		fs[0]();
		fs[1]();
		return log;
	}
	function test2() external returns (string memory) {
		log = "";
		function() external[] memory x = new function() external[](2);
		x[0] = this.b;
		x[1] = this.a;
		g(x);
		fs[0]();
		fs[1]();
		return log;
	}
	function test3() external returns (string memory) {
		log = "";
		gs = fs;
		gs[0]();
		gs[1]();
		return log;
	}
}
// ----
// test() -> 0x20, 0x14, "[a called][b called]"
// gas irOptimized: 116673
// gas legacy: 119030
// gas legacyOptimized: 117021
// test2() -> 0x20, 0x14, "[b called][a called]"
// test3() -> 0x20, 0x14, "[b called][a called]"
// gas irOptimized: 103256
// gas legacy: 102814
// gas legacyOptimized: 101706
