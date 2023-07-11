contract test {
    mapping(uint nameSame => mapping(uint name1 => mapping(uint nameSame => uint name3) name6) name4) public name5;
}
// ----
// DeclarationError 1809: (20-117): Conflicting parameter name "nameSame" in mapping.
