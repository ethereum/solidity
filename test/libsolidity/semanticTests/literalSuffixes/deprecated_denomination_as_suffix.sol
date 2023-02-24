function szabo(uint x) pure suffix returns (uint) { return 2 * x; }
function finney(uint x) pure suffix returns (uint) { return 4 * x; }

contract C {
    uint public s = 2 szabo;
    uint public f = 2 finney;
}
// ----
// s() -> 4
// f() -> 8
