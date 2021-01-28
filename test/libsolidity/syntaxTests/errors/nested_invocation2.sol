error E1(uint);
error E2();
function f() pure {
    revert(E1(E2));
}
// ----
// TypeError 9553: (62-64): Invalid type for argument in function call. Invalid implicit conversion from function () pure to uint256 requested.
