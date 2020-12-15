contract C {
    function f() public pure {
        // 0 is okay, because it's an exception
        address payable a = payable(0);

        // address literals have type address
        address payable b = payable(0x00000000219ab540356cBB839Cbe05303d7705Fa);

        address payable c = payable(address(2));

        a; b; c;
    }
}
