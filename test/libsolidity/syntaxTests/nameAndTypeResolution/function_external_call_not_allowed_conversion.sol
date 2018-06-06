contract C {}
contract Test {
    function externalCall() public {
        address arg;
        this.g(arg);
    }
    function g (C c) external {}
}
// ----
// TypeError: (103-106): Invalid type for argument in function call. Invalid implicit conversion from address to contract C requested.
