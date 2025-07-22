## Math Expression Evaluator
### Version: alpha (not functional yet)

### Goals 
- Unary negative and binary operations addition, subtraction, multiplication, division and exponentiation
- Constants—both standard and custom defined
- n-ary functions with a broken syntax initially, which should be improved to the standard notation later. 
- Standard functions like log(), sin(), and cos()

### On Identifiers
Constants and functions in this evaluator aren't seperate types but rather just semantic variants of the type identifier.
Identifiers represent any valid expression. They can be declared using the syntax:
```
(let iden = 2)
(let iden = 2^log(10))
(let iden = sinx/cosx)
```
Identifiers can be declared beforehand or at the time of first use.
The following expression evaluates to 8.
```
(x = 2)*x*x 
```
The let keyword is used to specify declaration without use.
The following expression evaluates to 4
```
(let x = 2) x*x
```

Identifiers aren't constant. Their values can be changed in the expression.
The following expression evaluates to 64
```
(x = 2)*x*(x=4)*x
```

Using let keyword for declaration of some identifier x in conjuction with another identifier which uses that identifier x (or multiple for n-ary functions) in its definition can be used to create a function.
Example syntax:
```
(let tan = sinx/cosx)
(let x = 45)tan 
```

The syntax is a little janky, so in future versions the tokenisation process can be used to interchange order of tokens to atleast have the argument declaration after the function name.
Allowing the standard function call format tan(45) can also be done by enforcing an order of arguments scheme based on order of first appearance in expression string, although that is slightly more work than just interchanging the order of tokens.
