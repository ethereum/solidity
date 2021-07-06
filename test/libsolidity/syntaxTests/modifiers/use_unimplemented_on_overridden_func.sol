abstract contract A {
    modifier m() virtual;
    function f() m public virtual {}
}
abstract contract B is A {
    function f() public override {}
}
// ----
