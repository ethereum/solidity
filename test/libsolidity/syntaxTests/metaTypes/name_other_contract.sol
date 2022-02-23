contract Test {
    function f() public pure returns (string memory) {
        return type(C).name;
    }
    function g() public pure returns (string memory) {
        return type(A).name;
    }
    function h() public pure returns (string memory) {
        return type(I).name;
    }
}

abstract contract A {
	function f() virtual public pure;
}

interface I {
	function f() external pure;
}

contract C {
    function f() pure public {
    }
}
// ----
