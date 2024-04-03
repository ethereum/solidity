contract C {
    event MyCustomEvent(uint);

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
        abi.encodeCall(this.f1, (MyCustomEvent, MyCustomEvent));
        abi.encodeCall(this.f2, (MyCustomEvent));
        abi.encodeCall(this.f3, (MyCustomEvent));
        abi.encodeCall(this.g1, (MyCustomEvent));
        abi.encodeCall(this.g2, (MyCustomEvent));
        abi.encodeCall(this.h, (MyCustomEvent));
        abi.encodeCall(this.i, (MyCustomEvent));
        abi.encodeCall(this.j, (MyCustomEvent));
        abi.encodeCall(this.k, (MyCustomEvent));
        abi.encodeCall(this.l, (MyCustomEvent));
    }
}
// ----
// TypeError 5407: (542-555): Cannot implicitly convert component at position 0 from "event MyCustomEvent(uint256)" to "uint8".
// TypeError 5407: (557-570): Cannot implicitly convert component at position 1 from "event MyCustomEvent(uint256)" to "uint8".
// TypeError 5407: (606-621): Cannot implicitly convert component at position 0 from "event MyCustomEvent(uint256)" to "uint32".
// TypeError 5407: (656-671): Cannot implicitly convert component at position 0 from "event MyCustomEvent(uint256)" to "uint256".
// TypeError 5407: (706-721): Cannot implicitly convert component at position 0 from "event MyCustomEvent(uint256)" to "bytes memory".
// TypeError 5407: (756-771): Cannot implicitly convert component at position 0 from "event MyCustomEvent(uint256)" to "bytes32".
// TypeError 5407: (805-820): Cannot implicitly convert component at position 0 from "event MyCustomEvent(uint256)" to "string memory".
// TypeError 5407: (854-869): Cannot implicitly convert component at position 0 from "event MyCustomEvent(uint256)" to "bool".
// TypeError 5407: (903-918): Cannot implicitly convert component at position 0 from "event MyCustomEvent(uint256)" to "address".
// TypeError 5407: (952-967): Cannot implicitly convert component at position 0 from "event MyCustomEvent(uint256)" to "address payable".
// TypeError 5407: (1001-1016): Cannot implicitly convert component at position 0 from "event MyCustomEvent(uint256)" to "enum C.testEnum".
