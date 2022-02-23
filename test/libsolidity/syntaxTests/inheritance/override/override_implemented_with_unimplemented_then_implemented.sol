contract A {
    function f() public virtual {}
}
abstract contract B is A {
    function f() public virtual override;
}
contract C is B {
    function f() public virtual override {}
}
// ----
// TypeError 4593: (81-118): Overriding an implemented function with an unimplemented function is not allowed.
