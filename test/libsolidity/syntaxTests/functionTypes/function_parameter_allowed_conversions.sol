contract D {
    constructor(function() external returns (uint)) {
    }
}

contract E {
    function test(function() external returns(uint) f) public returns (uint) {
        return f();
    }
}

library L {
    function test(function() external returns(uint) f) public returns (uint) {
        return f();
    }
}

contract C {
    uint x;
    // tests for usage as constructor parameter
    function f() public {
        // An assert used to fail in ABIFunction.cpp that the function types are not exactly equal
        // that is pure or view v/s default even though they could be converted.
        new D(this.testPure);
        new D(this.testView);
        new D(this.testDefault);
    }

    // tests for usage as contract function parameter
    function g() public {
        E e = E(address(0));

        e.test(this.testPure);
        e.test(this.testView);
        e.test(this.testDefault);
    }

    // tests for usage as library function parameter
    function h() public {
        L.test(this.testPure);
        L.test(this.testView);
        L.test(this.testDefault);
    }

    // tests for usage as return parameter
    function i() public view returns (function() external returns(uint)) {
        uint value = block.number % 3;

        if (value == 0) {
            return this.testPure;
        }
        else if (value == 1) {
            return this.testView;
        }
        else {
            return this.testDefault;
        }
    }

    modifier mod(function() external returns(uint) fun) {
        if (fun() == 0) {
            _;
        }
    }

    // tests for usage as modifier parameter
    function j(function() external pure returns(uint) fun) mod(fun) public {
    }

    function testPure() public pure returns (uint) {
        return 0;
    }

    function testView() public view returns (uint) {
        return x;
    }

    function testDefault() public returns (uint) {
        x = 5;
        return x;
    }
}
// ----
