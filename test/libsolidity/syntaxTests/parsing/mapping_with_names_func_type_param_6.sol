contract test {
    function(mapping(uint nameSame => mapping(uint name2 => mapping(uint nameSame => uint name3) name4) name5) storage) internal stateVariableName;
}
// ----
// DeclarationError 1809: (29-126): Conflicting parameter name "nameSame" in mapping.
