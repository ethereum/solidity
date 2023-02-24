function szabo(uint x) pure suffix returns (uint) { return x; }
function finney(uint x) pure suffix returns (uint) { return x; }

contract C {
    function f() public pure {
        1 szabo;
        1 finney;
    }
}
