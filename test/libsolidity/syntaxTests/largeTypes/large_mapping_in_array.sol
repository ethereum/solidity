contract C {
    function f() public {
        mapping (uint => uint[2][2**255])[][] storage A;
        mapping (uint => uint[2][2**255])[2] storage B;
    }
}
// ----
// TypeError 1534: (47-94): Type too large for storage.
// TypeError 1534: (104-150): Type too large for storage.
