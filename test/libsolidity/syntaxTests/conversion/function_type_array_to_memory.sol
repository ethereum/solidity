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

        externalDefaultArray = [this.externalView];
        internalDefaultArray = [internalView];

        return (externalDefaultArray[0](), internalDefaultArray[0]());
    }

    function testPureToDefault() public returns (uint, uint) {
        function () external returns(uint)[1] memory externalDefaultArray;
        function () internal returns(uint)[1] memory internalDefaultArray;

        externalDefaultArray = [this.externalPure];
        internalDefaultArray = [internalPure];

        return (externalDefaultArray[0](), internalDefaultArray[0]());
    }

    function testPureToView() public returns (uint, uint) {
        function () external returns(uint)[1] memory externalViewArray;
        function () internal returns(uint)[1] memory internalViewArray;

        externalViewArray = [this.externalPure];
        internalViewArray = [internalPure];

        return (externalViewArray[0](), internalViewArray[0]());
    }
}
// ----
// Warning 2018: (17-81): Function state mutability can be restricted to pure
// Warning 2018: (86-152): Function state mutability can be restricted to pure
// Warning 2018: (229-293): Function state mutability can be restricted to pure
// Warning 2018: (298-364): Function state mutability can be restricted to pure
