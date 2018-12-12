contract C {}
contract Test {
    function externalCall() public {
        C arg;
        this.g(arg);
    }
    function g (C c) external {}
}
// ----
// Warning: (113-141): Function state mutability can be restricted to pure
