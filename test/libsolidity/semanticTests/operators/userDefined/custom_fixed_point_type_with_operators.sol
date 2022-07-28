type Fixed is int128;
using {add as +, mul as *} for Fixed;

int constant MULTIPLIER = 10**18;

function add(Fixed a, Fixed b) pure returns (Fixed) {
    return Fixed.wrap(Fixed.unwrap(a) + Fixed.unwrap(b));
}

function mul(Fixed a, Fixed b) pure returns (Fixed) {
    int intermediate = (int(Fixed.unwrap(a)) * int(Fixed.unwrap(b))) / MULTIPLIER;
    if (int128(intermediate) != intermediate) { revert("Overflow"); }
    return Fixed.wrap(int128(intermediate));
}

contract C {
    function applyInterest(Fixed value, Fixed percentage) public pure returns (Fixed result) {
        return value + value * percentage;
    }
}
// ----
// applyInterest(int128,int128): 500000000000000000000, 100000000000000000 -> 550000000000000000000
