function intSuffix(int8 x) pure suffix returns (int8) { return x + 1; }
function uintSuffix(uint y) pure suffix returns (uint) { return y - 1; }
function constantSuffix(int8) pure suffix returns (int) { return 8; }

contract C {
    function overflow() public pure {
        127 intSuffix;
    }

    function underflow() public pure {
        0 uintSuffix;
    }

    function notUnderOverflow() public pure {
        127 constantSuffix;
        0 constantSuffix;
        -127 constantSuffix;
    }
}
// ====
// SMTEngine: all
// ----
// Warning 4984: (63-68): CHC: Overflow (resulting value larger than 127) happens here.
// Warning 3944: (136-141): CHC: Underflow (resulting value less than 0) happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
