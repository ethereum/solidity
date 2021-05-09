contract C {
	function f() public {
		log0;
		log1;
		log2;
		log3;
		log4;
	}
}
// ----
// DeclarationError 7576: (38-42): Undeclared identifier.
// DeclarationError 7576: (46-50): Undeclared identifier.
// DeclarationError 7576: (54-58): Undeclared identifier.
// DeclarationError 7576: (62-66): Undeclared identifier.
// DeclarationError 7576: (70-74): Undeclared identifier.
