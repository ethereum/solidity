abstract contract A {
	int public testvar;
}
abstract contract X is A {
	int public override testvar;
}
// ----
// DeclarationError 9097: (73-100): Identifier already declared.
// TypeError 1452: (23-41): Cannot override public state variable.
