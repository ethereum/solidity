contract IFoo {
    event Approve(address a, address b);
}
contract Foo is IFoo {
    modifier Approve { _; }
}
// ----
// DeclarationError: (86-109): Identifier already declared.
// TypeError: (86-109): Override changes event to modifier.
