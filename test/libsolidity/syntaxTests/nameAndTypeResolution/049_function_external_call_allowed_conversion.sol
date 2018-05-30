contract C {}
contract Test {
    function externalCall() public {
        C arg;
        this.g(arg);
    }
    function g (C c) external {}
}
// ----
// Warning: (125-128): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (113-141): Function state mutability can be restricted to pure
