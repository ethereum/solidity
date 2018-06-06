contract C {
    function f() public returns (bool ret) {
        return f == f;
    }
    function g() public returns (bool ret) {
        return f != f;
    }
}
// ----
// Warning: (17-86): Function state mutability can be restricted to pure
// Warning: (91-160): Function state mutability can be restricted to pure
