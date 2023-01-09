contract test {
    mapping(uint name1 => mapping(uint name2 => uint name3) name4) map;

    function main() external {
        mapping(uint nameSame => mapping(uint name2 => uint nameSame) name4) storage _map = map;
        _map[1][2] = 3;
    }
}
// ----
// DeclarationError 1809: (128-196): Conflicting parameter name "nameSame" in mapping.
