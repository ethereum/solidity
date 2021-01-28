error E(uint,string);
function f() pure {
    revert(E(2, 3));
}
// ----
// TypeError 9553: (58-59): Invalid type for argument in function call. Invalid implicit conversion from int_const 3 to string memory requested.
