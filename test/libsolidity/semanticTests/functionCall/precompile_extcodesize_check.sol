interface Identity {
    function selectorAndAppendValue(uint value) external pure returns (uint);
}
interface ReturnMoreData {
    function f(uint value) external pure returns (uint, uint, uint);
}
contract C {
    Identity constant i = Identity(address(0x0004));
    function testHighLevel() external pure returns (bool) {
        // Works because the extcodesize check is skipped
        // and the precompiled contract returns actual data.
        i.selectorAndAppendValue(5);
        return true;
    }
    function testHighLevel2() external pure returns (uint, uint, uint) {
        // Fails because the identity contract does not return enough data.
        return ReturnMoreData(address(4)).f(2);
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
// EVMVersion: >=constantinople
// ----
// testHighLevel() -> true
// testLowLevel() -> 0xc76596d400000000000000000000000000000000000000000000000000000000
// testHighLevel2() -> FAILURE
