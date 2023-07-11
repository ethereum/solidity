contract test {
    struct Person {
        mapping(uint nameSame => mapping(uint name1 => mapping(uint nameSame => uint name2) name3) name4) name5;
    }
}
// ----
// DeclarationError 1809: (44-141): Conflicting parameter name "nameSame" in mapping.
