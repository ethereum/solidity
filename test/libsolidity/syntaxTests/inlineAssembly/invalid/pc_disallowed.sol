contract C {
	function f() pure public {
		assembly {
			pop(pc())
		}
	}
}
// ----
// SyntaxError 2450: (61-63='pc'): PC instruction is a low-level EVM feature. Because of that PC is disallowed in strict assembly.
