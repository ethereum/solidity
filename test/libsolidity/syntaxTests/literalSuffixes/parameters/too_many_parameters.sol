function uintUintUintSuffix(uint, uint, uint) pure returns (uint) { return 1; }
function stringStringStringSuffix(string memory, string memory, string memory) pure returns (uint) { return 1; }
function uintStringSuffix(uint, string memory) pure returns (uint) { return 1; }
function stringUintSuffix(string memory, uint) pure returns (uint) { return 1; }

contract C {
    function f() public pure {
        1 uintUintUintSuffix;
        1 stringStringStringSuffix;
        1 uintStringSuffix;
        1 stringUintSuffix;

        1.1 uintUintUintSuffix;
        1.1 stringStringStringSuffix;
        1.1 uintStringSuffix;
        1.1 stringUintSuffix;

        "a" uintUintUintSuffix;
        "a" stringStringStringSuffix;
        "a" uintStringSuffix;
        "a" stringUintSuffix;
    }
}
// ----
// TypeError 4778: (408-428): Functions that take 3 or more arguments cannot be used as literal suffixes.
// TypeError 4778: (438-464): Functions that take 3 or more arguments cannot be used as literal suffixes.
// TypeError 8838: (474-492): The type of the literal cannot be converted to the parameters of the suffix function.
// TypeError 8838: (502-520): The type of the literal cannot be converted to the parameters of the suffix function.
// TypeError 4778: (531-553): Functions that take 3 or more arguments cannot be used as literal suffixes.
// TypeError 4778: (563-591): Functions that take 3 or more arguments cannot be used as literal suffixes.
// TypeError 8838: (601-621): The type of the literal cannot be converted to the parameters of the suffix function.
// TypeError 8838: (631-651): The type of the literal cannot be converted to the parameters of the suffix function.
// TypeError 4778: (662-684): Functions that take 3 or more arguments cannot be used as literal suffixes.
// TypeError 4778: (694-722): Functions that take 3 or more arguments cannot be used as literal suffixes.
// TypeError 4778: (732-752): Functions that take 2 arguments can only be used as literal suffixes for rational numbers.
// TypeError 4778: (762-782): Functions that take 2 arguments can only be used as literal suffixes for rational numbers.
