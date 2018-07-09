contract C {
    function f() public view returns (bytes4) {
        function () g;
        return g.selector;
    }
}
// ----
// TypeError: (99-109): Member "selector" not found or not visible after argument-dependent lookup in function ()
