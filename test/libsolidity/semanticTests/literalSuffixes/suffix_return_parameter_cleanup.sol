function suffix(uint8) pure suffix returns (uint8 z) {
    assembly {
        // Return a value with dirty bytes outside of uint8
        z := 0xffff
    }
}

contract C {
    function test() external pure returns (uint, uint) {
        uint8 a;

        // The exact literal used does not matter
        uint8 suffixResult = 1 suffix;
        uint8 functionResult = suffix(1);

        // Get the whole slot, including bytes outside of uint8
        uint suffixResultFull;
        uint functionResultFull;
        assembly {
            suffixResultFull := suffixResult
            functionResultFull := functionResult
        }

        // If the result is not 0xff, no cleanup was performed.
        return (suffixResultFull, functionResultFull);
    }
}
// ----
// test() -> 0xffff, 0xffff
