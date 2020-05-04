 contract C {
        function h ( bool flag ) public returns ( bool c ) {
                mapping ( string => uint24 ) [ 1 ] memory val ;
        }
}
// ----
// TypeError: (91-136): Data location must be "storage" for variable, but "memory" was given.
