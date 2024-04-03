interface testInterface {
    function C(function (string memory) external) external;
    function D(string calldata) external;
    function E(string memory) external;
    function F(address) external;

    error C2(function (string memory) external);
    error F2(address);
}

contract testContract {
    function g(string calldata) external {}
    function h(string memory) external {}
    function i(string calldata str) external {
        this.h(str);
        this.g(str);
    }
    function j(string memory str) external {
        this.h(str);
        this.g(str);
    }
    function k(string memory str) external pure {
        abi.encodeCall(testInterface.D, (str));
    }
    string s;

    function main() external view {
        abi.encodeCall(testInterface.C, (this.g));
        abi.encodeCall(testInterface.C, (this.h));
        abi.encodeCall(testInterface.D, (s));
        abi.encodeCall(testInterface.E, (s));
        abi.encodeCall(testInterface.F, (payable(address(0))));
        abi.encodeCall(this.i, (s));
        abi.encodeCall(this.j, (s));

        abi.encodeError(testInterface.C2, (this.g));
        abi.encodeError(testInterface.C2, (this.h));
        abi.encodeError(testInterface.F2, (payable(address(0))));
    }
}
// ----
// Warning 6133: (933-987): Statement has no effect.
