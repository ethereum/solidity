contract C {
    function externalView() external view returns(uint) { return 12; }
    function externalPure() external pure returns(uint) { return 13; }

    function internalView() internal view returns(uint) { return 22; }
    function internalPure() internal pure returns(uint) { return 23; }

    function testViewToDefault() public returns (uint, uint) {
        function () external returns(uint)[1] memory externalDefaultArray;
        function () internal returns(uint)[1] memory internalDefaultArray;

        // This would work if we were assigning to storage rather than memory
        externalDefaultArray = [this.externalView];
        internalDefaultArray = [internalView];

        return (externalDefaultArray[0](), internalDefaultArray[0]());
    }

    function testPureToDefault() public returns (uint, uint) {
        function () external returns(uint)[1] memory externalDefaultArray;
        function () internal returns(uint)[1] memory internalDefaultArray;

        // This would work if we were assigning to storage rather than memory
        externalDefaultArray = [this.externalPure];
        internalDefaultArray = [internalPure];

        return (externalDefaultArray[0](), internalDefaultArray[0]());
    }

    function testPureToView() public returns (uint, uint) {
        function () external returns(uint)[1] memory externalViewArray;
        function () internal returns(uint)[1] memory internalViewArray;

        // This would work if we were assigning to storage rather than memory
        externalViewArray = [this.externalPure];
        internalViewArray = [internalPure];

        return (externalViewArray[0](), internalViewArray[0]());
    }
}
// ----
// testViewToDefault() -> 12, 22
// testPureToDefault() -> 13, 23
// testPureToView() -> 13, 23
