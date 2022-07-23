.. index:: ! modifier;placeholder

.. _placeholders:

************************
The Placeholder Operator
************************

In function modifiers, it is necessary to tell Solidity when you want the function affected by the modifier to be run. The placeholder (denoted by an underscore character followed by a semicolon `_;` is used to denote where the function being modified should be run within the modifier.

Let's take a look at how the placeholder is used within a mutex modifier:

.. code-block:: solidity

	modifier mutex {  
		require(!lock);  
		lock = true;  
		_; // This is where the function being modified will be run
		lock = false;  
	}


A simple way of thinking of this modifier is as a function which wraps the original function, in which case the placeholder operator would represent where we call our original function. 

Note that the placeholder operator is different from using underscores as leading or trailing characters in variable names, which is a stylistic choice.
