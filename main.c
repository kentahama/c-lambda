#include <stdio.h>
#include "lambda.h"

int main(void) {
  lambda K = lam('x', lam('y', var('x')));
  lambda S = lam('x', lam('y', lam('z', app(app(var('x'), var('z')), app(var('y'), var('z'))))));
  lambda SKK = app(app(copy(S), copy(K)), copy(K));
  lambda X = lam('x', app(var('x'), var('x')));
  lambda XX = app(copy(X), copy(X));

  beta_reduction(&SKK);

  beta_reduction(&XX);

  return 0;
}
