contract C {
    function () external returns(uint)[1] externalDefaultArray;
    function () external view returns(uint)[1] externalViewArray;
    function () external pure returns(uint)[1] externalPureArray;

    function () internal returns(uint)[1] internalDefaultArray;
    function () internal view returns(uint)[1] internalViewArray;
    function () internal pure returns(uint)[1] internalPureArray;

    function externalDefault() external returns(uint) { return 11; }
    function externalView() external view returns(uint) { return 12; }
    function externalPure() external pure returns(uint) { return 13; }

    function internalDefault() internal returns(uint) { return 21; }
    function internalView() internal view returns(uint) { return 22; }
    function internalPure() internal pure returns(uint) { return 23; }

    function testViewToDefault() public returns (uint, uint) {
        externalDefaultArray = [this.externalView];
        internalDefaultArray = [internalView];

        return (externalDefaultArray[0](), internalDefaultArray[0]());
    }

    function testPureToDefault() public returns (uint, uint) {
        externalDefaultArray = [this.externalPure];
        internalDefaultArray = [internalPure];

        return (externalDefaultArray[0](), internalDefaultArray[0]());
    }

    function testPureToView() public returns (uint, uint) {
        externalViewArray = [this.externalPure];
        internalViewArray = [internalPure];

        return (externalViewArray[0](), internalViewArray[0]());
    }
}
// ----
// testViewToDefault() -> 12, 22
// testPureToDefault() -> 13, 23
// testPureToView() -> 13, 23
