contract test {
    function fa(uint) public {}
    function fb(uint) internal {}
    function fc(uint) internal {}
    function fd(uint) external {}
    function fe(uint) external {}
    function ff(uint) internal {}
    function fg(uint) internal pure {}
    function fh(uint) pure internal {}

    function(uint) a = fa;
    function(uint) internal b = fb; // (explicit internal applies to the function type)
    function(uint) internal internal c = fc;
    function(uint) external d = this.fd;
    function(uint) external internal e = this.fe;
    function(uint) internal f = ff;
    function(uint) internal pure g = fg;
    function(uint) pure internal h = fh;
}
// ----
// Warning: (20-47): Function state mutability can be restricted to pure
// Warning: (52-81): Function state mutability can be restricted to pure
// Warning: (86-115): Function state mutability can be restricted to pure
// Warning: (120-149): Function state mutability can be restricted to pure
// Warning: (154-183): Function state mutability can be restricted to pure
// Warning: (188-217): Function state mutability can be restricted to pure
