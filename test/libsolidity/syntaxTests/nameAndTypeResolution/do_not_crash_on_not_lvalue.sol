// This checks for a bug that caused a crash because of continued analysis.
contract C {
    mapping (uint => uint) m;
    function f() public {
        m(1) = 2;
    }
}
// ----
// TypeError: (153-157): Type is not callable
// TypeError: (153-157): Expression has to be an lvalue.
// TypeError: (160-161): Type int_const 2 is not implicitly convertible to expected type tuple().
