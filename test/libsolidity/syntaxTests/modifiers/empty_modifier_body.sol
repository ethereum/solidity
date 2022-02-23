abstract contract A { modifier mod(uint a) virtual;}
contract B is A { modifier mod(uint a) override { _; } }

abstract contract C {
	modifier m virtual;
	function f() m public {

	}
}
contract D is C {
	modifier m override {
		_;
	}
}
// ----
