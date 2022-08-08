using {
    addC as +,
    addM as +,
    addS as +
} for S;

struct S {
    uint v;
}

function addC(S calldata _s, S calldata) pure returns (S calldata) {
    return _s;
}

function addM(S memory _s, S memory) pure returns (S memory) {
    _s.v = 7;
    return _s;
}

function addS(S storage _s, S storage) returns (S storage) {
    _s.v = 13;
    return _s;
}

contract C {
    S s;

    function testC(S calldata _s) public returns (S calldata) {
        return _s + _s;
    }

    function testM() public returns (S memory) {
        S memory sTmp;
        return sTmp + sTmp;
    }

    function testS() public returns (uint) {
        s + s;
        return s.v;
    }

    function testSTmp() public returns (uint) {
        S storage sTmp = s;
        sTmp + sTmp;
        return sTmp.v;
    }
}

// ----
// testC((uint256)): 3 -> 3
// testM()-> 7
// testS() -> 13
// testSTmp() -> 13
