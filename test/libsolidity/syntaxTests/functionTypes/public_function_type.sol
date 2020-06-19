contract C {
    function f() public {
        function(uint) public returns (uint) x;
    }
}
// ----
// TypeError 6012: (47-85): Invalid visibility, can only be "external" or "internal".
