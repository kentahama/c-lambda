#include <stdio.h>
#include <stdlib.h>

enum nodetype {
  /* TODO: dummy */
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

struct refer_ctx_s {
  int n;
  lambda lams[64];
  char vars[64];
};

struct refer_ctx_s add_lam(struct refer_ctx_s ctx, lambda m, char c) {
  int n = ctx.n++;
  ctx.lams[n] = m;
  ctx.vars[n] = c;
  return ctx;
}

void add_reference(lambda m, lambda *ref) {
  int num = m->ref_num++;
  m->refs[num] = ref;
}

int find_ctx_index(struct refer_ctx_s ctx, char var) {
  int i;
  for (i = ctx.n - 1; i >= 0; i--) {
    if (ctx.vars[i] == var) {
      return i;
    }
  }
  return -1;
}

void refer_r(lambda m, struct refer_ctx_s ctx, lambda *referer) {
  int i;

  switch (m->type) {
  case VAR:
    i = find_ctx_index(ctx, m->var);
    add_reference(ctx.lams[i], referer);
    free(m);
    m = *referer = NULL;
    break;
  case LAM:
    ctx = add_lam(ctx, m, m->var);
    m->var = 0;
    refer_r(m->left, ctx, &m->left);
    break;
  case APP:
    refer_r(m->left, ctx, &m->left);
    refer_r(m->right, ctx, &m->right);
    break;
  }
}

void refer(lambda m) {
  /* TODO: Dummy */
  /* TODO: Free var into initial_ctx*/
  struct refer_ctx_s initial_ctx;
  initial_ctx.n = 0;
  refer_r(m, initial_ctx, NULL);
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

  refer(K);
  derefer(K);
  print_lambda(K);

  refer(S);
  derefer(S);
  print_lambda(S);

  return 0;
}
