contract C {
    function f(mapping(uint => uint)[] storage) public pure {
    }
}
// ----
// TypeError: (28-51): Location has to be memory for publicly visible functions (remove the "storage" or "calldata" keyword).
