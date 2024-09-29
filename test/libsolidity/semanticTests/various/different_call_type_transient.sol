contract B {
    uint256 transient public value;

    function setValue(uint256 v) public {
        value += v;
    }
}

contract A {
    uint256 transient public value;

    function delegateSetValue(address otherContract, uint256 v) public {
        (bool success, ) = otherContract.delegatecall(abi.encodeWithSignature("setValue(uint256)", v));
        require(success);
    }
    function callSetValue(address otherContract, uint256 v) public {
        (bool success, ) = otherContract.call(abi.encodeWithSignature("setValue(uint256)", v));
        require(success);
    }
    function staticSetValue(address otherContract, uint256 v) view public returns (bool) {
        (bool success, ) = otherContract.staticcall(abi.encodeWithSignature("setValue(uint256)", v));
        return success;
    }
}

contract Test {
    A a = new A();
    B b = new B();

    function testDelegate() public returns (uint256, uint256) {
        a.delegateSetValue(address(b), 7);
        return (a.value(), b.value());
    }
    function testCall() public returns (uint256, uint256) {
        a.callSetValue(address(b), 8);
        return (a.value(), b.value());
    }
    function testStatic() view public returns (bool) {
        return a.staticSetValue(address(b), 0);
    }
}

// ====
// EVMVersion: >=cancun
// ----
// testDelegate() -> 7, 0
// testCall() -> 0, 8
// testStatic() -> false
// gas irOptimized: 96900694
// gas legacy: 96901136
// gas legacyOptimized: 96900725
