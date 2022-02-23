contract c {
    bytes arr;
    function f() public { bytes1 a = arr[0];}
}
// ----
// Warning 2072: (54-62): Unused local variable.
// Warning 2018: (32-73): Function state mutability can be restricted to view
