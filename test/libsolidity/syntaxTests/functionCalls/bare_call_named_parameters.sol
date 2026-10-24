contract C {
    function f(address target) public returns (bool success, bytes memory data) {
        (success, data) = target.call({data: ""});
    }
}
// ----
// TypeError 4974: (121-144): Named argument "data" does not match function declaration.
