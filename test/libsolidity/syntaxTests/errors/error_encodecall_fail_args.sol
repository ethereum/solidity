error MyCustomError(uint, bool);
contract C {
    enum testEnum { choice1, choice2, choice3 }

    function f1(uint8, uint8) external {}
    function f2(uint32) external {}
    function f3(uint) external {}
    function g1(bytes memory) external {}
    function g2(bytes32) external {}
    function h(string memory) external {}
    function i(bool) external {}
    function j(address) external {}
    function k(address payable) external {}
    function l(testEnum) external {}

    function f() pure public {
        abi.encodeCall(this.f1, (MyCustomError, MyCustomError));
        abi.encodeCall(this.f2, (MyCustomError));
        abi.encodeCall(this.f3, (MyCustomError));
        abi.encodeCall(this.g1, (MyCustomError));
        abi.encodeCall(this.g2, (MyCustomError));
        abi.encodeCall(this.h, (MyCustomError));
        abi.encodeCall(this.i, (MyCustomError));
        abi.encodeCall(this.j, (MyCustomError));
        abi.encodeCall(this.k, (MyCustomError));
        abi.encodeCall(this.l, (MyCustomError));
    }
}
// ----
// TypeError 5407: (543-556): Cannot implicitly convert component at position 0 from "error MyCustomError(uint256,bool)" to "uint8".
// TypeError 5407: (558-571): Cannot implicitly convert component at position 1 from "error MyCustomError(uint256,bool)" to "uint8".
// TypeError 5407: (607-622): Cannot implicitly convert component at position 0 from "error MyCustomError(uint256,bool)" to "uint32".
// TypeError 5407: (657-672): Cannot implicitly convert component at position 0 from "error MyCustomError(uint256,bool)" to "uint256".
// TypeError 5407: (707-722): Cannot implicitly convert component at position 0 from "error MyCustomError(uint256,bool)" to "bytes memory".
// TypeError 5407: (757-772): Cannot implicitly convert component at position 0 from "error MyCustomError(uint256,bool)" to "bytes32".
// TypeError 5407: (806-821): Cannot implicitly convert component at position 0 from "error MyCustomError(uint256,bool)" to "string memory".
// TypeError 5407: (855-870): Cannot implicitly convert component at position 0 from "error MyCustomError(uint256,bool)" to "bool".
// TypeError 5407: (904-919): Cannot implicitly convert component at position 0 from "error MyCustomError(uint256,bool)" to "address".
// TypeError 5407: (953-968): Cannot implicitly convert component at position 0 from "error MyCustomError(uint256,bool)" to "address payable".
// TypeError 5407: (1002-1017): Cannot implicitly convert component at position 0 from "error MyCustomError(uint256,bool)" to "enum C.testEnum".
