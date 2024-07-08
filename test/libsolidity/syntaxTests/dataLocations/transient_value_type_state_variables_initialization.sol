contract D { }

contract C {
	int transient x = -99;
	address transient a = address(0xABC);
	bool transient b = x > 0 ? false : true;
}
// ====
// EVMVersion: >=cancun
// ----
// DeclarationError 9825: (30-51): Initialization of transient storage state variables is not supported.
// DeclarationError 9825: (54-90): Initialization of transient storage state variables is not supported.
// DeclarationError 9825: (93-132): Initialization of transient storage state variables is not supported.
