contract C {
    function f() public returns(bool) {
        bytes32 x = bytes32(uint256(1));
        assembly {
            x: = not(x)
        }
        if (x != ~bytes32(uint256(1))) return false;
        assembly {
            x: = iszero(x)
        }
        if (x != bytes32(0)) return false;
        return true;
    }
}

// ----
// f() -> true
// f():"" -> "1"
