abstract contract A {
    function f() public virtual;
    function g() public {
        f();
    }
}
// ----
