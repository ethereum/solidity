contract C {
    function externalView() external view returns(uint) { return 12; }
    function externalPure() external pure returns(uint) { return 13; }

    function internalView() internal view returns(uint) { return 22; }
    function internalPure() internal pure returns(uint) { return 23; }

    function testViewToDefault() public returns (uint, uint) {
        function () external returns(uint) externalDefault;
        function () internal returns(uint) internalDefault;

        externalDefault = this.externalView;
        internalDefault = internalView;

        return (externalDefault(), internalDefault());
    }

    function testPureToDefault() public returns (uint, uint) {
        function () external returns(uint) externalDefault;
        function () internal returns(uint) internalDefault;

        externalDefault = this.externalPure;
        internalDefault = internalPure;

        return (externalDefault(), internalDefault());
    }

    function testPureToView() public returns (uint, uint) {
        function () external view returns(uint) externalView;
        function () internal view returns(uint) internalView;

        externalView = this.externalPure;
        internalView = internalPure;

        return (externalView(), internalView());
    }
}
// ----
// testViewToDefault() -> 12, 22
// testPureToDefault() -> 13, 23
// testPureToView() -> 13, 23
