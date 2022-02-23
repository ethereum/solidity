contract A {
    uint public foo;
}
contract X is A {
	uint public override foo;
}
// ----
// DeclarationError 9097: (55-79): Identifier already declared.
// TypeError 1452: (17-32): Cannot override public state variable.
