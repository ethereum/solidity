contract C {
    function f() view returns (bytes4) {
        function () g;
        return g.selector;
    }
}
// ----
// TypeError: (92-102): Member "selector" not found or not visible after argument-dependent lookup in function ()
