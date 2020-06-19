contract c {
    bytes arr;
    function f() public { byte a = arr[0];}
}
// ----
// Warning 2072: (54-60): Unused local variable.
// Warning 2018: (32-71): Function state mutability can be restricted to view
