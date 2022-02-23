// This checks for a bug that caused a crash because of continued analysis.
contract C {
    mapping (uint => uint) m;
    function f() public {
        m(1) = 2;
    }
}
// ----
// TypeError 5704: (153-157): Type is not callable
