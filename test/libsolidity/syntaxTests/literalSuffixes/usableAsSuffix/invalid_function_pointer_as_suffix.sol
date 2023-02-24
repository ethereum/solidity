function suffix(uint x) pure suffix returns (uint) { return x; }

contract C {
    function (uint) pure returns (uint) storagePtr = suffix;

    function f() public pure {
        function (uint) pure returns (uint) localPtr = suffix;
        1000 localPtr;
        1000 storagePtr;
    }
}
// ----
// TypeError 4438: (248-256): The literal suffix must be either a subdenomination or a file-level suffix function.
// TypeError 4438: (271-281): The literal suffix must be either a subdenomination or a file-level suffix function.
