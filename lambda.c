#include <stdio.h>
#include <stdlib.h>

enum nodetype {
  VAR,
  LAM,
  APP,
};

struct lambda_s {
  enum nodetype type;
  struct lambda_s **refs[64];
  int ref_num;
  char var;
  struct lambda_s *left;
  struct lambda_s *right;
};

typedef struct lambda_s *lambda;

lambda new_lambda() {
  lambda new;
  new = malloc(sizeof(struct lambda_s));
  if (new == NULL) {
    fprintf(stderr, "Error: new_lambda: malloc failed\n");
    exit(-1);
  }
  new->ref_num = 0;
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
  case VAR:
    putchar(m->var);
    break;
  case LAM:
    putchar('(');
    putchar('\\');
    putchar(m->var);
    putchar('-');
    putchar('>');
    print_lambda_r(m->left);
    putchar(')');
    break;
  case APP:
    putchar('(');
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

void derefer_r(lambda m, char c) {
  int i;
  switch (m->type) {
  case VAR:
    break;
  case LAM:
    m->var = c;
    for (i = 0; i < m->ref_num; i++) {
      *m->refs[i] = var(c);
    }
    derefer_r(m->left, c + 1);
    break;
  case APP:
    derefer_r(m->left, c + 1);
    derefer_r(m->right, c + 1);
    break;
  }
}

void derefer(lambda m) {
  derefer_r(m, 'a');
}

int main(void) {
  lambda K = lam('x', lam('y', var('x')));
  lambda S = lam('x', lam('y', lam('z', app(app(var('x'), var('z')), app(var('y'), var('z'))))));

  print_lambda(K);
  print_lambda(S);

  lambda lx = S;
  lambda ly = lx->left;
  lambda lz = ly->left;
  lambda ra = lz->left;

  lx->ref_num = 1;
  lx->refs[0] = &ra->left->left;
  free(*lx->refs[0]);
  *lx->refs[0] = NULL;

  ly->ref_num = 1;
  ly->refs[0] = &ra->right->left;
  free(*ly->refs[0]);
  *ly->refs[0] = NULL;

  lz->ref_num = 2;
  lz->refs[0] = &ra->left->right;
  lz->refs[1] = &ra->right->right;
  free(*lz->refs[0]);
  *lz->refs[0] = NULL;
  free(*lz->refs[1]);
  *lz->refs[1] = NULL;

  derefer(S);

  print_lambda(S);

  return 0;
}
