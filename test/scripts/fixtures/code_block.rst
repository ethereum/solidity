Some text


.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.7.0 <0.9.0;

    contract C {
        function foo() public view {}
    }


.. warning::

    A Warning.

::

    contract C {}

More text.

.. code-block:: yul

    let x := add(1, 5)

.. code-block:: yul

    // Yul code wrapped in object
    {
        {
            let y := mul(3, 5)
        }
    }

.. code-block:: yul
    // Yul code wrapped in named object
    object "Test" {
        {
            let y := mul(6, 9)
        }
    }

