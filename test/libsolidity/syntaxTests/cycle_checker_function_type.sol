// Used to cause ICE.
contract C {
        function ( ) internal returns ( bytes [ ] storage , mapping ( bytes => mapping ( bytes => mapping ( uint => mapping ( bytes => mapping ( string => mapping ( uint => mapping ( uint => mapping ( uint => mapping ( uint => mapping ( string => mapping ( string => mapping ( uint => mapping ( bytes => mapping ( uint => mapping ( uint => mapping ( uint => mapping ( uint => mapping ( uint => mapping ( bytes => mapping ( uint => mapping ( uint => mapping ( uint => mapping ( uint => mapping ( string => mapping ( uint => string ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) [ ] storage ) constant c = c ;
}
// ----
// TypeError: (43-643): The value of the constant c has a cyclic dependency via c.
