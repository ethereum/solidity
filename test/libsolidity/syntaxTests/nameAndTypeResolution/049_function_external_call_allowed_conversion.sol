contract C {}
contract Test {
    function externalCall() public {
        C arg;
        this.g(arg);
    }
    function g (C c) external {}
}
// ----
