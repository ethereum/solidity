contract C {
    function f(address a) public {
        selfdestruct(a);
    }
}
// ----
// TypeError 9553: (69-70='a'): Invalid type for argument in function call. Invalid implicit conversion from address to address payable requested.
