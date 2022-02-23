contract C {
	bool a;
	bool b;
	bool c;
	bool d;
	bool e;
	bool f;
	function press_A() public {
		if(e) { a = true; } else { reset(); }
	}
	function press_B() public {
		if(c) { b = true; } else { reset(); }
	}
	function press_C() public {
		if(a) { c = true; } else { reset(); }
	}
	function press_D() public {
		d = true;
	}
	function press_E() public {
		if(d) { e = true; } else { reset(); }
	}
	function press_F() public {
		if(b) { f = true; } else { reset(); }
	}
	function is_not_solved() view public {
		// f = true can be reached by calling the functions
		// press_D()
		// press_E()
		// press_A()
		// press_C()
		// press_B()
		// press_F()
		assert(!f);
	}
	function reset() internal {
		a = false;
		b = false;
		c = false;
		d = false;
		e = false;
		f = false;
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (657-667): CHC: Assertion violation might happen here.
// Warning 4661: (657-667): BMC: Assertion violation happens here.
