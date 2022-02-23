{
    fun()

    revert(0, 0)

    function fun()
    {
        sstore(0, 1)
    }

    pop(add(1, 1))
}
// ----
// step: deadCodeEliminator
//
// {
//     fun()
//     revert(0, 0)
//     function fun()
//     { sstore(0, 1) }
// }
