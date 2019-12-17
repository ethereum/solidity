contract A {
    uint public foo;
}
contract X is A {
	uint public override foo;
}
// ----
// DeclarationError: (55-79): Identifier already declared.
// TypeError: (17-32): Cannot override public state variable.
