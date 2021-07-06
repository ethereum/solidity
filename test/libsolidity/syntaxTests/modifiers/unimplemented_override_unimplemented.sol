abstract contract A {
    modifier m() virtual;
}
abstract contract B is A {
    modifier m() virtual override;
}
abstract contract C is B {
    modifier m() virtual override;
    function f() m public {}
}
// ----
