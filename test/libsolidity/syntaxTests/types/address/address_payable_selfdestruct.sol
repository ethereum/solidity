contract C {
    function f(address payable a) public {
        selfdestruct(a);
    }
}
// ----
// Warning 5159: (64-76): "selfdestruct" has been deprecated. The underlying opcode will eventually undergo breaking changes, and its use is not recommended.
