contract D {
    constructor(function() external pure returns (uint) ) {
    }
}

library L {
    function f() public pure returns (uint) {
        return 5;
    }
}

contract C {
    function f() public returns (uint r) {
        // An assert used to fail if the function types are not exactly equal (pure, view) v/s
        // default
        // ok
        new D(this.testPure);
        // not okay
        new D(this.testView);
        // not okay
        new D(this.testDefault);
        // not okay
        new D(this.testDifferentSignature);
        // not okay
        new D(gasleft);
        // not okay
        new D(L.f);
        // not okay
        new D(testInternalFunction);
        // not okay
        new D(this.testInternalFunction);
    }

    function testPure() public pure returns (uint) {
    }

    function testView() public view returns (uint) {
        block.timestamp;
    }

    function testDefault() public returns (uint) {
        selfdestruct(payable(address(this)));
    }

    function testDifferentSignature(uint a) public pure returns (uint) {
    }

    function testInternalFunction() internal pure returns (uint) {
        return 10;
    }
}
// ----
// TypeError 9553: (415-428='this.testView'): Invalid type for argument in function call. Invalid implicit conversion from function () view external returns (uint256) to function () pure external returns (uint256) requested.
// TypeError 9553: (465-481='this.testDefault'): Invalid type for argument in function call. Invalid implicit conversion from function () external returns (uint256) to function () pure external returns (uint256) requested.
// TypeError 9553: (518-545='this.testDifferentSignature'): Invalid type for argument in function call. Invalid implicit conversion from function (uint256) pure external returns (uint256) to function () pure external returns (uint256) requested.
// TypeError 9553: (582-589='gasleft'): Invalid type for argument in function call. Invalid implicit conversion from function () view returns (uint256) to function () pure external returns (uint256) requested. Special functions can not be converted to function types.
// TypeError 9553: (626-629='L.f'): Invalid type for argument in function call. Invalid implicit conversion from function () pure returns (uint256) to function () pure external returns (uint256) requested. Special functions can not be converted to function types.
// TypeError 9553: (666-686='testInternalFunction'): Invalid type for argument in function call. Invalid implicit conversion from function () pure returns (uint256) to function () pure external returns (uint256) requested. Special functions can not be converted to function types.
// TypeError 9582: (723-748='this.testInternalFunction'): Member "testInternalFunction" not found or not visible after argument-dependent lookup in contract C.
