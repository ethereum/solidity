contract test {
    function fa(bytes memory) public { }
    function(bytes memory) external internal a = fa;
}
// ----
// TypeError 7407: (106-108): Type function (bytes memory) is not implicitly convertible to expected type function (bytes memory) external. Special functions cannot be converted to function types.
