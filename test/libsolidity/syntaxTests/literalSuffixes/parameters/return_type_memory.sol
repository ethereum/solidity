struct S {
    uint x;
}

function structSuffix(uint x) pure suffix returns (S memory s) {}
function arraySuffix(uint x) pure suffix returns (uint[5] memory a) {}

contract C {
    function f() public pure {
        1 structSuffix;
        1 arraySuffix;
    }
}
