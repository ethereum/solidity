 contract C {
        function h ( bool flag ) public returns ( bool c ) {
                mapping ( string => uint24 ) [ 1 ] memory val ;
        }
}
// ----
// TypeError 4061: (91-136): Type mapping(string => uint24)[1] is only valid in storage because it contains a (nested) mapping.
