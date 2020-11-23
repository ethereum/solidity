contract A {
    fallback (bytes calldata _input) external returns (bytes memory) {
        return _input;
    }
}
contract B {
    fallback (bytes calldata _input) external returns (bytes memory) {
        return "xyz";
    }
}
contract C is B, A {
     function f() public returns (bool, bytes memory) {
        (bool success, bytes memory retval) = address(this).call("abc");
        return (success, retval);
    }
}
// ====
// EVMVersion: >=byzantium
// ----
// TypeError 6480: (229-420): Derived contract must override function "". Two or more base classes define function with same name and parameter types.
