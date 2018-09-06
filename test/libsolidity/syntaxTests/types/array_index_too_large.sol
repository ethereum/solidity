contract C {
    function f() public returns (string memory) {
        // this used to cause an internal error
        return (["zeppelin"][123456789012345678901234567890123456789012345678901234567890123456789012345678]);
    }
}
// ----
// TypeError: (140-218): Type int_const 1234...(70 digits omitted)...5678 is not implicitly convertible to expected type uint256.