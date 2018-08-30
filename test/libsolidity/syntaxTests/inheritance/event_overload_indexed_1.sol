contract IFoo {
    event Approve(address a, address b);
}
contract Foo is IFoo {
    event Approve(address indexed a, address b);
}
// ----
// TypeError: (86-130): Event overload must extend parameter list or have different types.
