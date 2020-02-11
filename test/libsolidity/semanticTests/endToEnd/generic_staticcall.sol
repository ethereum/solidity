contract A {
    uint public x;
    constructor() public {
        x = 42;
    }

    function pureFunction(uint256 p) public pure returns(uint256) {
        return p;
    }

    function viewFunction(uint256 p) public view returns(uint256) {
        return p + x;
    }

    function nonpayableFunction(uint256 p) public returns(uint256) {
        x = p;
        return x;
    }

    function assertFunction(uint256 p) public view returns(uint256) {
        assert(x == p);
        return x;
    }
}
contract C {
    function f(address a) public view returns(bool, bytes memory) {
        return a.staticcall(abi.encodeWithSignature("pureFunction(uint256)", 23));
    }

    function g(address a) public view returns(bool, bytes memory) {
        return a.staticcall(abi.encodeWithSignature("viewFunction(uint256)", 23));
    }

    function h(address a) public view returns(bool, bytes memory) {
        return a.staticcall(abi.encodeWithSignature("nonpayableFunction(uint256)", 23));
    }

    function i(address a, uint256 v) public view returns(bool, bytes memory) {
        return a.staticcall(abi.encodeWithSignature("assertFunction(uint256)", v));
    }
}

// ----

contract A {
    uint public x;
    constructor() public {
        x = 42;
    }

    function pureFunction(uint256 p) public pure returns(uint256) {
        return p;
    }

    function viewFunction(uint256 p) public view returns(uint256) {
        return p + x;
    }

    function nonpayableFunction(uint256 p) public returns(uint256) {
        x = p;
        return x;
    }

    function assertFunction(uint256 p) public view returns(uint256) {
        assert(x == p);
        return x;
    }
}
contract C {
    function f(address a) public view returns(bool, bytes memory) {
        return a.staticcall(abi.encodeWithSignature("pureFunction(uint256)", 23));
    }

    function g(address a) public view returns(bool, bytes memory) {
        return a.staticcall(abi.encodeWithSignature("viewFunction(uint256)", 23));
    }

    function h(address a) public view returns(bool, bytes memory) {
        return a.staticcall(abi.encodeWithSignature("nonpayableFunction(uint256)", 23));
    }

    function i(address a, uint256 v) public view returns(bool, bytes memory) {
        return a.staticcall(abi.encodeWithSignature("assertFunction(uint256)", v));
    }
}

// ----
// f(address): c_addressA -> true, 0x40, 0x20, 23
// f(address):"0" -> "1, 64, 32, 23"
// g(address): c_addressA -> true, 0x40, 0x20, 23 + 42
// g(address):"0" -> "1, 64, 32, 65"
// h(address): c_addressA -> false, 0x40, 0x00
// h(address):"0" -> "0, 64, 0"
// i(address,uint256): c_addressA, 42 -> true, 0x40, 0x20, 42
// i(address,uint256):"0, 42" -> "1, 64, 32, 42"
// i(address,uint256): c_addressA, 23 -> false, 0x40, 0x00
// i(address,uint256):"0, 23" -> "0, 64, 0"
