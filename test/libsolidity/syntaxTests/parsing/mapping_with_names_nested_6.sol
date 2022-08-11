contract test {
    mapping(uint nameSame => mapping(uint name1 => mapping(uint nameSame => uint name2) name3) name4) name5;
}
// ----
// DeclarationError 1809: (20-117): Conflicting parameter name "nameSame" in mapping.
