contract C {
    uint immutable public x = 42;

    function g() external view returns (uint) {}

    function f() public view {
        this.x = this.g;
    }
}
// ----
// TypeError 4247: (137-143): Expression has to be an lvalue.
