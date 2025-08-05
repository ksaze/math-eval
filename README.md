## Math Expression Evaluator
### Version: 1.0

### Requirements
- C11+ complaint C compiler
- Make for building
- Criterion library (Optional: Only if you want testing.) 

### Building
```
git clone https://github.com/ksaze/math-eval --depth=1
make
```

### Features
- Basic operators: Add, subtract, unary negative, exponentiation, etc.
- Standard functions like log(), sin(), and cos().
- Lazily evaluated identifiers that can function as constants and n-ary functions
- Identifiers declarations inside identifiers
- Dereference operator '*' to force eager evaluation of identifiers when required
- Conditional defined lamda calculus style using identifiers

### Problems
- Naive conditional defined using identifiers fail on recursive cases

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
As can be observed, assignment can happen in happen in-line in any part of the expression.
For declarations inside definition of another identifier, they must precede the actual computational expression.
```
(a = 
    (x = 3)(y = 2)(z = 1)
    <expr>
)
```

By allowing lazy evaluation of identifiers, a pseudo-function can be achieved.
Declaration of some identifier x in conjuction with another identifier which uses that identifier x (or multiple for n-ary functions) in its definition can be used to create a pseudo-function.
Example syntax:
```
(tan = sinx/cosx)
(x = 45)tan 
```

Sometimes, lazy evaluation can invalidate data. For example trying to decrement n using (n = n-1) would change the definition of n to that expression involving a reference to identifier n instead of actually decrementing n.
To force eager evaluation, the dereference operator '*' can be added before an identifier to force eager evaluation in the definition.
```
(n = *n - 1)
```

The conditional operator is defined config.txt. It is a function which takes three arguments: true, false, and predicate.
```
(is_neg = 1-(n*n)^(1/2)/n)
(pred = is_neg)
(true = 1)
(false = -1)
(n = 3)
cond +
(n = -19)
cond
```
