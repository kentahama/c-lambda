#include <stdio.h>
#include <stdlib.h>

enum nodetype {
  VAR,
  LAM,
  APP,
};

struct lambda_s {
  enum nodetype type;
  char var;
  struct lambda_s *left;
  struct lambda_s *right;
};

typedef struct lambda_s *lambda;

lambda new_lambda() {
  lambda new;
  new = malloc(sizeof(struct lambda_s));
  if (new == NULL) {
    fprintf(stderr, "Error: cannot alloc memory for lambda_s\n");
    exit(-1);
  }
  new->var = 0;
  new->left = new->right = NULL;
  return new;
}

lambda var(char v) {
  lambda ret = new_lambda();
  ret->type = VAR;
  ret->var = v;
  return ret;
}

lambda lam(char v, lambda m) {
  lambda ret = new_lambda();
  ret->type = LAM;
  ret->var = v;
  ret->left = m;
  return ret;
}

lambda app(lambda m, lambda n) {
  lambda ret = new_lambda();
  ret->type = APP;
  ret->left = m;
  ret->right = n;
  return ret;
}

void print_lambda_r(lambda m) {
  switch (m->type) {
    case VAR: putchar(m->var);
              break;
    case LAM: putchar('(');
              putchar('\\');
              putchar(m->var);
              putchar('-');
              putchar('>');
              print_lambda_r(m->left);
              putchar(')');
              break;
    case APP: putchar('(');
              print_lambda_r(m->left);
              putchar(' ');
              print_lambda_r(m->right);
              putchar(')');
              break;
  }
}

void print_lambda(lambda m) {
  print_lambda_r(m);
  putchar('\n');
}

int main(void) {
  lambda K = lam('x', lam('y', var('x')));
  lambda S = lam('x', lam('y', lam('z', app(app(var('x'), var('z')), app(var('y'), var('z'))))));

  print_lambda(K);
  print_lambda(S);

  return 0;
}
