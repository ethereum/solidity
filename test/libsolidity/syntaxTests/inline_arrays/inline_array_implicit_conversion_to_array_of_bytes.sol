contract C {
    function f() external pure {
        bytes[2] memory a1 = ['foo', 'bar'];
        bytes[2] memory a2 = [hex'666f6f', hex'626172'];
        require(keccak256(a1[0]) == keccak256(a2[0]));
        require(keccak256(a1[1]) == keccak256(a2[1]));
    }
}
