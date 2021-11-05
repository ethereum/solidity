interface Identity {
    function selectorAndAppendValue(uint value) external pure returns (uint);
}
contract C {
    Identity constant i = Identity(address(0x0004));
    function testHighLevel() external pure returns (bool) {
        // Should fail because `extcodesize(4) = 0`
        i.selectorAndAppendValue(5);
        return true;
    }
    function testLowLevel() external view returns (uint value) {
        (bool success, bytes memory ret) =
            address(4).staticcall(
                abi.encodeWithSelector(Identity.selectorAndAppendValue.selector, uint(5))
            );
        value = abi.decode(ret, (uint));
    }

}
// ====
// compileViaYul: also
// EVMVersion: >=constantinople
// ----
// testHighLevel() -> FAILURE
// testLowLevel() -> 0xc76596d400000000000000000000000000000000000000000000000000000000
