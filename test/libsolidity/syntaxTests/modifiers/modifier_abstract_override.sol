contract A {
    modifier m() virtual { _; }
}
abstract contract B is A {
    modifier m() virtual override;
}
contract C is B {
    function f() m public {}
}
// ----
// TypeError 4593: (78-108): Overriding an implemented modifier with an unimplemented modifier is not allowed.
