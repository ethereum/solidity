function suffix(uint x) pure returns (uint) { return x; }

contract C {
    function (uint) pure returns (uint) storagePtr = suffix;

    function f() public pure {
        function (uint) pure returns (uint) localPtr = suffix;
        1000 localPtr;
        1000 storagePtr;
    }
}
// ----
// TypeError 4438: (236-249): The literal suffix needs to be a pre-defined suffix or a file-level function.
// TypeError 4438: (259-274): The literal suffix needs to be a pre-defined suffix or a file-level function.
