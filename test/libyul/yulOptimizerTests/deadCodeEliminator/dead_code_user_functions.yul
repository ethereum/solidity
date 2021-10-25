{
    switch calldataload(0)
    case 0 {
        recursive()
        sstore(0, 1)
    }
    case 1 {
        terminating()
        sstore(0, 7)
    }
    case 2 {
        reverting()
        sstore(0, 7)
    }


    function recursive()
    {
        recursive()
    }
    function terminating()
    {
        return(0, 0)
    }
    function reverting()
    {
        revert(0, 0)
    }
}
// ----
// step: deadCodeEliminator
//
// {
//     switch calldataload(0)
//     case 0 { recursive() }
//     case 1 { terminating() }
//     case 2 { reverting() }
//     function recursive()
//     { recursive() }
//     function terminating()
//     { return(0, 0) }
//     function reverting()
//     { revert(0, 0) }
// }
