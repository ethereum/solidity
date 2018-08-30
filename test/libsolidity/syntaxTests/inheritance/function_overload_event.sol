contract IFoo {
    event Approve(address a, address b);
}
contract Foo is IFoo {
    function Approve(address a, address b) public;
}
// ----
// DeclarationError: (86-132): Identifier already declared.
// TypeError: (86-132): Override changes event to function.
