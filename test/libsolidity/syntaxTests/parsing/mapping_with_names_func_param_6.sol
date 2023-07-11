contract test {
    function _main(mapping(uint nameSame => mapping(uint name2 => mapping(uint nameSame => uint name3) name4) name5) storage map) internal {
        map[1][2][3] = 4;
    }
}
// ----
// DeclarationError 1809: (35-132): Conflicting parameter name "nameSame" in mapping.
