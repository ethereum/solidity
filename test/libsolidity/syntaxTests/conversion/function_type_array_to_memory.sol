contract C {
    function externalDefault() external returns(uint) { return 11; }
    function externalView() external view returns(uint) { return 12; }
    function externalPure() external pure returns(uint) { return 13; }

    function internalDefault() internal returns(uint) { return 21; }
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
// TypeError 7407: (760-779): Type function () view external returns (uint256)[1] memory is not implicitly convertible to expected type function () external returns (uint256)[1] memory.
// TypeError 7407: (812-826): Type function () view returns (uint256)[1] memory is not implicitly convertible to expected type function () returns (uint256)[1] memory.
// TypeError 7407: (1230-1249): Type function () pure external returns (uint256)[1] memory is not implicitly convertible to expected type function () external returns (uint256)[1] memory.
// TypeError 7407: (1282-1296): Type function () pure returns (uint256)[1] memory is not implicitly convertible to expected type function () returns (uint256)[1] memory.
// TypeError 7407: (1688-1707): Type function () pure external returns (uint256)[1] memory is not implicitly convertible to expected type function () external returns (uint256)[1] memory.
// TypeError 7407: (1737-1751): Type function () pure returns (uint256)[1] memory is not implicitly convertible to expected type function () returns (uint256)[1] memory.
