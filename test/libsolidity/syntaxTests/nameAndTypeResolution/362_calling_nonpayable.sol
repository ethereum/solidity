contract receiver { function nopay() public {} }
contract test {
    function f() public { (new receiver()).nopay.value(10)(); }
}
// ----
// TypeError: (91-119): Member "value" not found or not visible after argument-dependent lookup in function () external - did you forget the "payable" modifier?
