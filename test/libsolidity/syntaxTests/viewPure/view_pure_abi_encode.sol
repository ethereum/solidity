contract C {
    function f() pure public returns (bytes memory r) {
        r = abi.encode(1, 2);
        r = abi.encodePacked(f());
        r = abi.encodeWithSelector(0x12345678, 1);
        r = abi.encodeWithSignature("f(uint256)", 4);
    }
}
