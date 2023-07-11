pragma abicoder               v2;

contract C {
    struct S { function () external x; }
    function f(S memory) public pure returns (uint r) { r = 1; }
    function g(S calldata) external pure returns (uint r) { r = 2; }
    function h(S calldata s) external pure returns (uint r) { s.x; r = 3; }
}
// ----
// f((function)): "01234567890123456789abcd" -> 1
// f((function)): "01234567890123456789abcdX" -> FAILURE
// g((function)): "01234567890123456789abcd" -> 2
// g((function)): "01234567890123456789abcdX" -> 2
// h((function)): "01234567890123456789abcd" -> 3
// h((function)): "01234567890123456789abcdX" -> FAILURE
