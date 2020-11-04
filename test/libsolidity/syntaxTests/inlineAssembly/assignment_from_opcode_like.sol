contract C {
    function f() public pure {
        uint mload;
        assembly {
            let x := mload
        }
    }
}
// ----
// Warning 8261: (52-62): Variable is shadowed in inline assembly by an instruction of the same name
