library L { function l() public {} }
contract test {
    function f() public {
        L.l.value;
    }
}
// ----
// TypeError: (87-96): Member "value" not found or not visible after argument-dependent lookup in function () - did you forget the "payable" modifier?
