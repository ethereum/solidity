function viewSuffix(uint) view returns (uint) {}
function mutableSuffix(uint) returns (uint) {}

contract C {
    function pureFunction() pure public {
        1 viewSuffix;     // TODO: Should be disallowed
        1 mutableSuffix;  // TODO: Should be disallowed
    }

    // TODO: There should be no mutability restriction suggestion
    function viewFunction() view public {
        1 mutableSuffix;  // TODO: Should be disallowed
    }
}
// ----
// Warning 2018: (341-440): Function state mutability can be restricted to pure
