abstract contract A {
    modifier m() virtual;
    function f() m public {}
}
contract B is A {
    modifier m() virtual override { _; }
}
// ----
