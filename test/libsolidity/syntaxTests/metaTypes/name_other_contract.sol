contract Test {
    function f() public pure returns (string memory) {
        return type(C).name;
    }
}

contract C {
    function f() pure public {
    }
}
