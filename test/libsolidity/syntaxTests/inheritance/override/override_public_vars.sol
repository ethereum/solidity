abstract contract A {
	int public testvar;
}
abstract contract X is A {
	int public override testvar;
}
// ----
// DeclarationError 9097: (73-100='int public override testvar'): Identifier already declared.
// TypeError 1452: (23-41='int public testvar'): Cannot override public state variable.
