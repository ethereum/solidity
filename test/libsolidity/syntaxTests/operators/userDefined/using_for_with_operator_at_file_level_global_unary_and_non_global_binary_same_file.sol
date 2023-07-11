type Int is int16;

using {unsub as -} for Int global;
using {sub as -} for Int;

function sub(Int a, Int b) pure returns (Int) {}

function unsub(Int a) pure returns (Int) {}

contract C {
    function test_sub() public pure returns (Int) {
        return Int.wrap(7) - Int.wrap(2);
    }

    function test_unsub() public pure returns (Int) {
        return -Int.wrap(4);
    }
}
// ----
// TypeError 3320: (62-65): Operators can only be defined in a global 'using for' directive.
