contract test {
    function fa(bytes memory) { }
    function(bytes memory) external internal a = fa;
}
// ----
// TypeError: (99-101): Type function (bytes memory) is not implicitly convertible to expected type function (bytes memory) external.
