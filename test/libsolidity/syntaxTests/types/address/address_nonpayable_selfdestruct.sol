contract C {
    function f(address a) public {
        selfdestruct(a);
    }
}
// ----
// Warning 5159: (56-68): "selfdestruct" has been deprecated. The underlying opcode will eventually undergo breaking changes, and its use is not recommended.
// TypeError 9553: (69-70): Invalid type for argument in function call. Invalid implicit conversion from address to address payable requested.
