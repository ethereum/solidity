contract test {
    mapping(uint nameSame => mapping(uint name1 => mapping(uint nameSame => uint name3) name6) name4) map;

    function main() external {
        mapping(uint nameSame => mapping(uint name1 => mapping(uint nameSame => uint name3) name6) name4) storage _map = map;
        _map[1][2][3] = 4;
    }
}
// ----
// DeclarationError 1809: (20-117): Conflicting parameter name "nameSame" in mapping.
// DeclarationError 1809: (163-260): Conflicting parameter name "nameSame" in mapping.
