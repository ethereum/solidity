type I8 is int8;
using {sub as -, unsub as -} for I8 global;
function sub(I8, I8) pure returns (I8) {}
function unsub(I8) pure returns (I8) {}
contract C {
    function f(I8 a, I8 b) public pure returns (I8) {
        return -a - b;
    }
}

// ----
