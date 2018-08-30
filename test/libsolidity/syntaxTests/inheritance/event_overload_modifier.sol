contract IFoo {
    modifier Approve { _; }
}
contract Foo is IFoo {
    event Approve(address a, address b);
}
// ----
// DeclarationError: (73-109): Identifier already declared.
// TypeError: (73-109): Override changes modifier to event.
