contract C {
    function f() public pure {
        address payable a = address(0);
        a = address(1);
        address payable b = 0x0123456789012345678901234567890123456789;
        b = 0x9876543210987654321098765432109876543210;
    }
}