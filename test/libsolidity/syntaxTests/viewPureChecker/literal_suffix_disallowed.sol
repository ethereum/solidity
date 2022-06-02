function pureSuffix(uint) view returns (uint) {}
function viewSuffix(uint) view returns (uint) {}
function mutableSuffix(uint) returns (uint) {}

contract C {
    function pureFunction() pure public {
        1 pureSuffix;
    }

    // TODO: There should be no mutability restriction suggestion
    function viewFunction() view public {
        1 pureSuffix;
        1 viewSuffix;
    }

    // TODO: There should be no mutability restriction suggestion
    function mutableFunction() public {
        1 pureSuffix;
        1 viewSuffix;
        1 mutableSuffix;
    }

    // TODO: There should be no mutability restriction suggestion
    function payableFunction() public {
        1 pureSuffix;
        1 viewSuffix;
        1 mutableSuffix;
    }
}
// ----
// Warning 2018: (300-387): Function state mutability can be restricted to pure
// Warning 2018: (459-569): Function state mutability can be restricted to pure
// Warning 2018: (641-751): Function state mutability can be restricted to pure
