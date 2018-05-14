# c-lambda
(untyped) lambda calculus implementation written in C.

## Limitation:
 * The only first character of the variable name matters; others are ignored.
 * Free variables must be capital (small letters may be captured).

## Example

```
$ git clone https://github.com/kentahama/c-lambda.git
$ cd c-lambda
$ make
$ ./main
(\x->\y->x) M N
(((\a->(\b->a)) M) N)
((\a->M) N)
M
```
