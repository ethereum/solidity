contract C {
	function f() pure public {
		assembly {
			pop(pc())
		}
	}
}
// ----
// Warning 2450: (61-63): The "pc" instruction is deprecated and will be removed in the next breaking release.
