contract C {
    function(bytes memory) internal public a;
}
// ----
// TypeError 6744: (17-57): Internal or recursive type is not allowed for public state variables.
