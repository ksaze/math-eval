## Math Expression Evaluator
### Version: beta (Missing features)

### Requirements
- C11+ complaint C compiler
- Make for building
- Criterion library (Optional: Only if you want testing.) 

### Goals 
- Basic operators: Add, subtract, unary negative, exponentiation, etc.
- Standard functions like log(), sin(), and cos().
- Constants—both standard and custom defined.
- n-ary functions.
- Achieve a recurisve function definition like factorial with the bare minimum feature set (Lambda calculus will save me).

### On Identifiers
Constants and functions in this evaluator aren't seperate types but rather just semantic variants of the type identifier.
Identifiers represent any valid expression. They can be declared using the syntax:
```
(iden = 2)
(iden = 2^log10)
(iden = sinx/cosx)
```

Identifiers aren't constant. Their values can be changed in the expression.
The following expression evaluates to 8
```
(x = 2) x*x
(x = 4) + x
```
As can be observed, assignment can happen in happen in-line in any part of the expression--the only exception being the inside another assignment, which might be supported later, as it requires architectural changes.

By allowing lazy evaluation of identifiers, a pseudo-function can be achieved.
Declaration of some identifier x in conjuction with another identifier which uses that identifier x (or multiple for n-ary functions) in its definition can be used to create a pseudo-function.
Example syntax:
```
(tan = sinx/cosx)
(x = 45)tan 
```
The syntax is a little janky, so in future versions the tokenisation process can be used to interchange order of tokens to atleast have the argument declaration after the function name.
Allowing the standard function call syntax can also be replicated by modifying the hash map of identifiers to store an array of identifiers which can be used to match declarations. 
