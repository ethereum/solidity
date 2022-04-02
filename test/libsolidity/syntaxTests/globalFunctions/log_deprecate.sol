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
// DeclarationError 7576: (38-42='log0'): Undeclared identifier.
// DeclarationError 7576: (46-50='log1'): Undeclared identifier.
// DeclarationError 7576: (54-58='log2'): Undeclared identifier.
// DeclarationError 7576: (62-66='log3'): Undeclared identifier.
// DeclarationError 7576: (70-74='log4'): Undeclared identifier.
