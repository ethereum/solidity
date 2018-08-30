contract IFoo {
    function Approve(address a, address b) public;
}
contract Foo is IFoo {
    event Approve(address a, address b);
}
// ----
// DeclarationError: (96-132): Identifier already declared.
// TypeError: (96-132): Override changes function to event.
